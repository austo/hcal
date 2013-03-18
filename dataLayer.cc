#include "dataLayer.h"

using namespace std;
using namespace pqxx;
using namespace boost::posix_time;

namespace hcal{

    #define UTC_OFFSET 5 //TODO: improve using real boost timezones
    #define HCAL_UTC_OFFSET "HCAL_UTC_OFFSET"
    #define HCAL_TIME_REGION "\'America/New_York\'"
    #define CONNSTRING "host=192.168.1.136 dbname=concerto user=appUser password=cAligul@"
    #define DL_EX_PREFIX "DataLayer: " 

    #define COL_ID "id"
    #define COL_DISP_COLOR "display_color"
    #define COL_DESCRIPTION "description"
    #define COL_TIME_START "time_start"
    #define COL_TIME_END "time_end"
    #define COL_ROOM_NAME "room_name"
    #define COL_ROOM_ID "room_id"
    #define COL_LEADER "leader"
    #define COL_LEADER_ID "leader_id"
    #define COMMA_SPACE ", "
    #define QUERY_GET_EVENTS "select id, description, time_start, \
        time_end, room_id, leader_id from events"
    #define QUERY_GET_EVENTS_FUNC "select * from get_events_for_timespan("
    #define QUERY_GET_ROOM_COLORS "select id, display_color from rooms;"
    #define QUERY_GET_ROOMS "select id, name as room_name, display_color from rooms order by id;"
    #define QUERY_INSERT_EVENT "select insert_event("
    #define QUERY_UPDATE_EVENT "select update_event("
    #define QUERY_DELETE_EVENT "select delete_event("
    #define QUERY_DELETED_FALSE "deleted = false"
    #define QUERY_OPEN_PARENS "("
    #define QUERY_CLOSE_PARENS ")"
    #define QUERY_CLOSE_PARENS_END ");"
    #define QUERY_WHERE " where "
    #define QUERY_AND " and "
    #define QUERY_LT_EQ  " <= "
    #define QUERY_GT_EQ " >= "
    #define QUERY_N_EQ " <> "
    #define QUERY_EQ " = "
    #define QUERY_ORDER_BY " order by "
    #define QUERY_END ";"

    typedef std::map<int, std::list<Event> >* emap_ptr;

    DataLayer::DataLayer(){
        //TODO: may want to update this to pull time string from db
        const char* utc_offset_char = getenv(HCAL_UTC_OFFSET);
        if (utc_offset_char == NULL){
            throw dl_exception("DataLayer: HCAL_UTC_OFFSET not set.");
        }
        time_region_ = string(utc_offset_char);
    }

    DataLayer::~DataLayer(){}

    v8::Handle<v8::Array>
    DataLayer::get_wrapped_events(time_t start, time_t end, int venue){
        v8::HandleScope scope;
        v8::Handle<v8::Array> retval;    
        result evts = get_events_for_timespan(start, end, venue);

        if (evts.size() > 0){
            retval = build_wrapped_events(evts);
        }
        return scope.Close(retval);
    }

    v8::Handle<v8::Array>
    DataLayer::build_wrapped_events(result& evts){
        v8::HandleScope scope;
        v8::Handle<v8::Array> retval = v8::Array::New(evts.size());    

        int i = 0;
        result::const_iterator row;
        for (row = evts.begin(); row != evts.end(); ++row){

            ptime p_evt_start(time_from_string(row[COL_TIME_START].as<string>()));
            ptime p_evt_end(time_from_string(row[COL_TIME_END].as<string>()));
            
            time_t t_evt_start = get_time_t_from_ptime(p_evt_start);
            time_t t_evt_end = get_time_t_from_ptime(p_evt_end);     

            v8::Handle<v8::Value> evtHdl = EventWrapper::get_wrapped_object(
                row[COL_ID].as<int>(),
                t_evt_start,
                t_evt_end,
                row[COL_ROOM_ID].as<int>(),
                row[COL_LEADER_ID].as<int>(),
                row[COL_DESCRIPTION].as<string>()
            );
            retval->Set(i, evtHdl);
            ++i;
        }
        return scope.Close(retval);
    }

    
    emap_ptr DataLayer::get_event_map(time_t start, time_t end, View v){
        emap_ptr retval = new map<int, list<Event> >();
        result evts = get_events_for_timespan(start, end);
        populate_emap(evts, retval, v);
        return retval;
    }

    emap_ptr DataLayer::get_event_map(time_t start, time_t end, View v, int venue){
        emap_ptr retval = new map<int, list<Event> >();
        result evts = get_events_for_timespan(start, end, venue);
        populate_emap(evts, retval, v);
        return retval;
    }

    void DataLayer::populate_emap(result& evts, emap_ptr emap, View v)
    {
        //local adjuster with zero-hour offset to account for daylight savings time;
        typedef boost::date_time::local_adjustor<ptime, 0, us_dst> local_adj;

        int current_year, wk_offset = 0, index = 0;
        result::const_iterator row;
        boost::gregorian::date first_sunday(boost::gregorian::not_a_date_time);
        for (row = evts.begin(); row != evts.end(); ++row){

            /*
                addition of utc offset is handled in database -
                this is equivalent to construction from time_t with zero offset,
                i.e. "make a local time on this computer from a given utc time"
            */
            ptime p_evt_start(time_from_string(row[COL_TIME_START].as<string>()));
            ptime p_evt_end(time_from_string(row[COL_TIME_END].as<string>()));

            //adjust for DST
            p_evt_start = local_adj::utc_to_local(p_evt_start);
            p_evt_end = local_adj::utc_to_local(p_evt_end);
                 

            switch(v){
                case month:
                    index = (int)p_evt_start.date().month().as_number();
                    break;
                case week: {
                    /*
                        ISO dates won't work for our needs
                        (hcal weeks start on Sunday and partial first week is counted),
                        so use a little boost date arithmetic to get our week index
                    */
                    if (first_sunday.is_not_a_date() || p_evt_start.date().year() != current_year || p_evt_start.date() < first_sunday){
                        get_week_offset_for_current_year(p_evt_start, first_sunday, current_year, wk_offset, index);                        
                    }
                    else{
                        index = 0;
                    }                                     

                    boost::gregorian::days days_since_new_year = p_evt_start.date() - first_sunday;
                    int weeks_since_first_sunday = days_since_new_year.days() / 7;
                    index = index ? index : weeks_since_first_sunday + wk_offset; 
                    cout << "v8 - wk index: " << index << " wk_offset: " << wk_offset << endl; 
                    break;
                }
                default:
                    throw runtime_error("DataLayer: view not implemented.");
            }

            Event evt(  row[COL_ID].as<int>(),
                        p_evt_start,
                        p_evt_end,
                        row[COL_ROOM_ID].as<int>(),
                        row[COL_LEADER_ID].as<int>(),
                        row[COL_DESCRIPTION].as<string>());

            (*emap)[index].push_back(evt);
        }   
    }

    result DataLayer::get_events_for_timespan(time_t start, time_t end){
        ptime p_start = from_time_t(start);
        ptime p_end = from_time_t(end);
        connection c(CONNSTRING);
        work txn(c);
        std::stringstream ss;
        ss << QUERY_GET_EVENTS_FUNC << txn.quote(to_simple_string(p_start))
           << COMMA_SPACE << txn.quote(to_simple_string(p_end)) << COMMA_SPACE
           << time_region_ << QUERY_CLOSE_PARENS_END;
        
        return execute_query(txn, ss.str());
    }

    result DataLayer::get_events_for_timespan(time_t start, time_t end, int venue){
        ptime p_start = from_time_t(start);
        ptime p_end = from_time_t(end);
        connection c(CONNSTRING);
        work txn(c);
        std::stringstream ss;
        ss << QUERY_GET_EVENTS_FUNC << txn.quote(to_simple_string(p_start))
           << COMMA_SPACE << txn.quote(to_simple_string(p_end)) << COMMA_SPACE
           << venue << QUERY_CLOSE_PARENS_END;
        
        return execute_query(txn, ss.str());
    }

    map<int, string> DataLayer::get_room_colors(){
        map<int, string> retval = map<int, string>();
        connection c(CONNSTRING);
        work txn(c);
        string query(QUERY_GET_ROOM_COLORS);
        result rm_colors = execute_query(txn, query);

        result::const_iterator row;
        for (row = rm_colors.begin(); row != rm_colors.end(); ++row){
            retval[row[COL_ID].as<int>()] = row[COL_DISP_COLOR].as<string>();
        }
        return retval;
    }

    map<int, Room> DataLayer::get_rooms(){
        map<int, Room> retval = map<int, Room>();
        connection c(CONNSTRING);
        work txn(c);
        string query(QUERY_GET_ROOMS);
        result rooms_result = execute_query(txn, query);

        result::const_iterator row;
        for (row = rooms_result.begin(); row != rooms_result.end(); ++row){
            retval[row[COL_ID].as<int>()] = 
                Room(row[COL_ID].as<int>(), row[COL_ROOM_NAME].as<string>(), row[COL_DISP_COLOR].as<string>());
        }
        return retval;
    }

    v8::Handle<v8::Value>
    DataLayer::insert_event(time_t start, time_t end, int room_id, int leader_id, string desc, bool recurring){
        v8::HandleScope scope;
        ptime p_evt_start = from_time_t(start);
        ptime p_evt_end = from_time_t(end);
        //p_evt_start -= utc_offset_td_;
        //p_evt_end -= utc_offset_td_;
        connection c(CONNSTRING);
        work txn(c);
        stringstream ss;
        ss << QUERY_INSERT_EVENT << txn.quote(desc) << COMMA_SPACE << txn.quote(to_simple_string(p_evt_start) + "+00")
           << COMMA_SPACE <<  txn.quote(to_simple_string(p_evt_end) + "+00" ) << COMMA_SPACE << txn.quote(room_id)
           << COMMA_SPACE << txn.quote(leader_id) << COMMA_SPACE << txn.quote(recurring) << QUERY_CLOSE_PARENS_END;
        cout << ss.str() << endl;
        result res = execute_query(txn, ss.str());
        txn.commit();

        int evt_id = res[0][0].as<int>();

        //event already present, throw dl_exception
        if (evt_id == -1){
            stringstream err_ss;
            err_ss << DL_EX_PREFIX
                << "There is already an event at " << to_simple_string(p_evt_start) << " in room " << room_id << ".";
            throw dl_exception(err_ss.str());
        }
        else if (evt_id == -2){
            stringstream err_ss;
            err_ss << DL_EX_PREFIX
                << "There is already an event at " << to_simple_string(p_evt_start) << " with leader " << leader_id << ".";
            throw dl_exception(err_ss.str());
        }

        //revert time back to js utc offset
        //p_evt_start += utc_offset_td_;
        //p_evt_end += utc_offset_td_;
        v8::Handle<v8::Value> retval = EventWrapper::get_wrapped_object(
                evt_id,
                get_time_t_from_ptime(p_evt_start),
                get_time_t_from_ptime(p_evt_end),
                room_id,
                leader_id,
                desc
        );
        return scope.Close(retval);
    }

    //TODO: may need to change update function return value to int with discrete values for event not found and failure 
    DataLayer::UpdateStatus
    DataLayer::update_event(int evt_id, time_t start, time_t end, int room_id, int leader_id, string desc, bool recurring){
        UpdateStatus retval = failure;
        try{
            ptime p_evt_start = from_time_t(start);
            ptime p_evt_end = from_time_t(end);
            //p_evt_start -= utc_offset_td_;
            //p_evt_end -= utc_offset_td_;
            connection c(CONNSTRING);
            work txn(c);
            stringstream ss;
            ss << QUERY_UPDATE_EVENT << txn.quote(evt_id) << COMMA_SPACE << txn.quote(room_id) << COMMA_SPACE << txn.quote(leader_id) << COMMA_SPACE
               << txn.quote(recurring) << COMMA_SPACE << txn.quote(desc) << COMMA_SPACE << txn.quote(to_simple_string(p_evt_start) + "+00") << COMMA_SPACE 
               << txn.quote(to_simple_string(p_evt_end) + "+00") << QUERY_CLOSE_PARENS_END;
            cout << ss.str() << endl;
            result res = execute_query(txn, ss.str());
            txn.commit();

            retval = (DataLayer::UpdateStatus)res[0][0].as<int>();
            #ifdef __DEBUG__           
            cout << "v8 - update retval: " << retval << endl;
            #endif
        }
        catch(exception& e){
            cout << e.what() << endl;
        }        
        return retval;
    }

    //TODO: simply return update status
    DataLayer::UpdateStatus
    DataLayer::delete_event(int evt_id){
        UpdateStatus retval = failure;
        try{
            connection c(CONNSTRING);
            work txn(c);
            stringstream ss;
            ss << QUERY_DELETE_EVENT << txn.quote(evt_id) << QUERY_CLOSE_PARENS;
            result res = execute_query(txn, ss.str());
            txn.commit();

            retval = (DataLayer::UpdateStatus)res[0][0].as<int>();
            #ifdef __DEBUG__
            cout << "v8 - delete event retval: " << retval << endl;
            #endif
        }
        catch(exception& e){ //may want to analyze result and throw dl_exception
            cout << e.what() << endl;
        }
        return retval;
    }


    result DataLayer::execute_query(transaction_base& txn, string query){
      return txn.exec(query);
    }


    time_t DataLayer::get_time_t_from_ptime(ptime pt){
        ptime epoch_start = from_time_t(0);
        time_duration td = pt - epoch_start;
        return td.total_seconds();
    }

    //TODO: cleanup this code, look for ineffencies
    void DataLayer::get_week_offset_for_current_year(ptime& p_evt_start, 
        boost::gregorian::date& first_sunday, int& current_year, int& wk_offset, int& index){
        using namespace boost::gregorian;

        //add a year's worth of weeks if we're changing years (i.e. not first time)
        int multi_yr_adj;
        if (first_sunday.is_not_a_date()){
            multi_yr_adj = 0;
        }
        else if (p_evt_start.date() < first_sunday){
            multi_yr_adj = 0;
        }
        else{
            multi_yr_adj = 53;
        }
        
        current_year = p_evt_start.date().year();
        first_day_of_the_week_in_month first_sun(boost::date_time::Sunday, boost::date_time::Jan);
        first_sunday = first_sun.get_date(current_year);

        /*leave index unchanged if current event is within
          first week of new year (calling code checks for index = 0) */
        index = p_evt_start.date() < first_sunday ? index : 0;
        int tmp_offset = (int)first_sunday.day() == 1 ? 1 : 2; //handle 2+ year spans
        wk_offset += (multi_yr_adj + tmp_offset);
    }    
}