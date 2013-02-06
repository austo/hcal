#include "dataLayer.h"

using namespace std;
using namespace pqxx;
using namespace boost::posix_time;


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
#define COMMA_SPACE ", "
#define QUERY_GET_EVENTS "SELECT id, description, time_start, time_end, room_name, room_id, leader FROM calendar_events"
#define QUERY_WHERE " where "
#define QUERY_AND " and "
#define QUERY_LT_EQ  " <= "
#define QUERY_GT_EQ " >= "
#define QUERY_N_EQ " <> "
#define QUERY_EQ " = "


DataLayer::DataLayer(){
    const char* utc_offset_char = getenv(HCAL_UTC_OFFSET);
    utc_offset_ = utc_offset_char ? atoi(utc_offset_char) : 0;
    //cout << "v8 - env utc offset: " << (utc_offset_ + 1) << endl;
}
DataLayer::~DataLayer(){}

v8::Handle<v8::Array>
DataLayer::get_wrapped_events(time_t start, time_t end){
    v8::HandleScope scope;
    v8::Handle<v8::Array> retval;
    ptime p_start = from_time_t(start);
    ptime p_end = from_time_t(end);
    connection c(CONNSTRING);
    work txn(c);
    std::stringstream ss;
    ss << QUERY_GET_EVENTS << QUERY_WHERE << COL_TIME_START << QUERY_GT_EQ
       << txn.quote(to_simple_string(p_start)) << QUERY_AND << COL_TIME_END
       << QUERY_LT_EQ << txn.quote(to_simple_string(p_end)) << ";";
    result evts = execute_query(txn, ss.str());

    if (evts.size() > 0){
        retval = build_wrapped_events(evts);
    }
    return scope.Close(retval);
}

v8::Handle<v8::Array>
DataLayer::build_wrapped_events(result& evts){
    v8::HandleScope scope;
    v8::Handle<v8::Array> retval = v8::Array::New(evts.size());    
    ptime epoch_start = from_time_t(0);
    time_duration td;

    int i = 0;
    result::const_iterator row;
    for (row = evts.begin(); row != evts.end(); ++row){

        //get time_t for start and end times, adding back utc offset
        ptime p_evt_start(time_from_string(row[COL_TIME_START].as<string>()));
        ptime p_evt_end(time_from_string(row[COL_TIME_END].as<string>()));
        p_evt_start += boost::posix_time::hours(utc_offset_);
        p_evt_end += boost::posix_time::hours(utc_offset_);
        td = p_evt_start - epoch_start;
        time_t t_evt_start = td.total_seconds();
        td = p_evt_end - epoch_start;
        time_t t_evt_end = td.total_seconds();       

        v8::Handle<v8::Value> evtHdl = EventWrapper::get_wrapped_object(
            row[COL_ID].as<int>(),
            t_evt_start,
            t_evt_end,
            row[COL_ROOM_ID].as<int>(),
            row[COL_ROOM_NAME].as<string>(),
            row[COL_LEADER].as<string>(),
            row[COL_DESCRIPTION].as<string>()
        );
        retval->Set(i, evtHdl);
        ++i;
    }
    return scope.Close(retval);
}

result DataLayer::execute_query(transaction_base& txn, std::string query){
  return txn.exec(query);
}

string get_env(const string& e_var) {
    const char* val = std::getenv(e_var.c_str());
    return val ? val : "";    
}