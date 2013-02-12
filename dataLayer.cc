#include "dataLayer.h"

using namespace std;
using namespace pqxx;
using namespace boost::posix_time;

namespace hcal{

    #define UTC_OFFSET 5 //TODO: improve using real boost timezones
    #define HCAL_UTC_OFFSET "HCAL_UTC_OFFSET"

    #define CONNSTRING "host=queequeg dbname=concerto user=appUser password=cAligul@"
    #define COL_ID "id"
    #define COL_DESCRIPTION "description"
    #define COL_TIME_START "time_start"
    #define COL_TIME_END "time_end"
    #define COL_ROOM_NAME "room_name"
    #define COL_ROOM_ID "room_id"
    #define COL_LEADER "leader"
    #define COL_LEADER_ID "leader_id"
    #define COMMA_SPACE ", "
    #define QUERY_GET_EVENTS "select id, description, time_start, time_end, \
        room_id, leader_id from events"
    #define QUERY_INSERT_EVENT "select insert_event("
    #define QUERY_UPDATE_EVENT "select update_event("
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
        const char* utc_offset_char = getenv(HCAL_UTC_OFFSET);
        int utc_offset = utc_offset_char ? atoi(utc_offset_char) : 0;
        utc_offset_td_ = boost::posix_time::hours(utc_offset);
    }

    DataLayer::~DataLayer(){}

    v8::Handle<v8::Array>
    DataLayer::get_wrapped_events(time_t start, time_t end){
        v8::HandleScope scope;
        v8::Handle<v8::Array> retval;    
        result evts = get_events_for_timespan(start, end);

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

            //get time_t for start and end times, adding back utc offset
            ptime p_evt_start(time_from_string(row[COL_TIME_START].as<string>()));
            ptime p_evt_end(time_from_string(row[COL_TIME_END].as<string>()));
            p_evt_start += utc_offset_td_;
            p_evt_end += utc_offset_td_;
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

    /*
        TODO: this needs to handle different views, likely with function pointers
        NOTE: it may be better to use a switch statement (unless calling code can 
        gracefully supply a sorting function)
    */

    emap_ptr DataLayer::get_event_map(time_t start, time_t end){
        emap_ptr retval = new map<int, list<Event> >();
        result evts = get_events_for_timespan(start, end);
        populate_emap(evts, retval);
        return retval;
    }

    void DataLayer::populate_emap(result& evts, emap_ptr emap)
    {
        result::const_iterator row;
        for (row = evts.begin(); row != evts.end(); ++row){

            //get time_t for start and end times, adding back utc offset
            ptime p_evt_start(time_from_string(row[COL_TIME_START].as<string>()));
            ptime p_evt_end(time_from_string(row[COL_TIME_END].as<string>()));
            p_evt_start += utc_offset_td_;
            p_evt_end += utc_offset_td_;

            int index = (int)p_evt_start.date().month().as_number();;

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
        ss << QUERY_GET_EVENTS << QUERY_WHERE << COL_TIME_START << QUERY_GT_EQ
           << txn.quote(to_simple_string(p_start)) << QUERY_AND << COL_TIME_END
           << QUERY_LT_EQ << txn.quote(to_simple_string(p_end))
           << QUERY_ORDER_BY << COL_TIME_START << QUERY_END;
        return execute_query(txn, ss.str());
    }

    v8::Handle<v8::Value>
    DataLayer::insert_event(time_t start, time_t end, int room_id, int leader_id, string desc, bool recurring){
        v8::HandleScope scope;
        ptime p_evt_start = from_time_t(start);
        ptime p_evt_end = from_time_t(end);
        p_evt_start -= utc_offset_td_;
        p_evt_end -= utc_offset_td_;
        connection c(CONNSTRING);
        work txn(c);
        stringstream ss;
        ss << QUERY_INSERT_EVENT << txn.quote(desc) << COMMA_SPACE << txn.quote(to_simple_string(p_evt_start))
           << COMMA_SPACE <<  txn.quote(to_simple_string(p_evt_end)) << COMMA_SPACE << txn.quote(room_id)
           << COMMA_SPACE << txn.quote(leader_id) << COMMA_SPACE << txn.quote(recurring) << QUERY_CLOSE_PARENS_END;
        cout << ss.str() << endl;
        result res = execute_query(txn, ss.str());
        txn.commit();

        int evt_id = res[0][0].as<int>();

        //revert time back to js utc offset
        p_evt_start += utc_offset_td_;
        p_evt_end += utc_offset_td_;
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

    // v8::Handle<v8::Value>
    // DataLayer::update_event(time_t start, time_t end, int room_id, int leader_id, string desc, bool recurring){
    //     v8::HandleScope scope;
    //     ptime p_evt_start = from_time_t(start);
    //     ptime p_evt_end = from_time_t(end);
    //     p_evt_start -= utc_offset_td_;
    //     p_evt_end -= utc_offset_td_;
    //     connection c(CONNSTRING);
    //     work txn(c);
    //     stringstream ss;
    //     ss << QUERY_INSERT_EVENT << txn.quote(desc) << COMMA_SPACE << txn.quote(to_simple_string(p_evt_start))
    //        << COMMA_SPACE <<  txn.quote(to_simple_string(p_evt_end)) << COMMA_SPACE << txn.quote(room_id)
    //        << COMMA_SPACE << txn.quote(leader_id) << COMMA_SPACE << txn.quote(recurring) << QUERY_CLOSE_PARENS_END;
    //     cout << ss.str() << endl;
    //     result res = execute_query(txn, ss.str());
    //     txn.commit();

    //     int evt_id = res[0][0].as<int>();

    //     //revert time back to js utc offset
    //     p_evt_start += utc_offset_td_;
    //     p_evt_end += utc_offset_td_;
    //     v8::Handle<v8::Value> retval = EventWrapper::get_wrapped_object(
    //             evt_id,
    //             get_time_t_from_ptime(p_evt_start),
    //             get_time_t_from_ptime(p_evt_end),
    //             room_id,
    //             leader_id,
    //             desc
    //     );
    //     return scope.Close(retval);
    // }


    result DataLayer::execute_query(transaction_base& txn, string query){
      return txn.exec(query);
    }

    string get_env(const string& e_var) {
        const char* val = getenv(e_var.c_str());
        return val ? val : "";    
    }

    time_t DataLayer::get_time_t_from_ptime(ptime pt){
        ptime epoch_start = from_time_t(0);
        time_duration td = pt - epoch_start;
        return td.total_seconds();
    }
}