#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <pqxx/pqxx>
#include <node.h>
#include "dataLayer.h"
#include "eventWrapper.h"
#include "posix_time/posix_time.hpp"

using namespace std;
using namespace pqxx;
using namespace boost::posix_time;

#define CONNSTRING "host=queequeg dbname=concerto user=appUser password=cAligul@"

DataLayer::DataLayer(){

}

DataLayer::~DataLayer(){

}

v8::Handle<v8::Array>
DataLayer::get_wrapped_events(time_t start, time_t end){
    //vector<EventWrapper>* retval = 0;
    v8::HandleScope scope;
    v8::Handle<v8::Array> retval;
    ptime p_start = from_time_t(start);
    ptime p_end = from_time_t(end);
    connection c(CONNSTRING);
    work txn(c);
    std::stringstream ss;
    ss << "select id, description, time_start, time_end, room, leader from calendar_events ";
    ss << "where time_start >= " << txn.quote(to_simple_string(p_start));
    ss << " and time_end <= " << txn.quote(to_simple_string(p_end));
    result evts = execute_query(txn, ss.str());
    if (evts.size() > 0){
        retval = build_wrapped_events(evts);
    }
    //cout << "v8 - DL retval: " << (*retval)[0].Title() << endl;
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

        //get time_t for start and end times
        ptime p_evt_start(time_from_string(row["time_start"].as<string>()));
        ptime p_evt_end(time_from_string(row["time_end"].as<string>()));
        td = p_evt_start - epoch_start;
        time_t t_evt_start = td.total_seconds();
        td = p_evt_end - epoch_start;
        time_t t_evt_end = td.total_seconds();

        // v8::Handle<v8::Value> argv[6] = {
        //     v8::Number::New(row["id"].as<int>()),
        //     v8::Number::New(t_evt_start * 1000),
        //     v8::Number::New(t_evt_end * 1000),
        //     v8::Number::New(0),
        //     v8::String::New(row["leader"].as<string>().c_str()),
        //     v8::String::New(row["description"].as<string>().c_str())
        // };
        // retval->Set(i, EventWrapper::NewInstance(6, argv));
        // ++i;
        // //Local<Object> instance = EventWrapper::NewInstance(6, argv);

        EventWrapper* evtwrp = new EventWrapper(
            row["id"].as<int>(),
            t_evt_start,
            t_evt_end,
            row["room"].as<string>(),
            row["leader"].as<string>(),
            row["description"].as<string>()
        );
        retval->Set(i, evtwrp);
        //retval->push_back(evtwrp);
    }
    return scope.Close(retval);
}

result DataLayer::execute_query(transaction_base& txn, std::string query){
  return txn.exec(query);
}

//  vector<EventWrapper>* retval = new vector<EventWrapper>();
//     ptime epoch_start = from_time_t(0);
//     time_duration td;



//     result::const_iterator row;
//     for (row = evts.begin(); row != evts.end(); ++row){

//         //get time_t for start and end times
//         ptime p_evt_start(time_from_string(row["time_start"].as<string>()));
//         ptime p_evt_end(time_from_string(row["time_end"].as<string>()));
//         td = p_evt_start - epoch_start;
//         time_t t_evt_start = td.total_seconds();
//         td = p_evt_end - epoch_start;
//         time_t t_evt_end = td.total_seconds();

//         EventWrapper evtwrp(
//             row["id"].as<int>(),
//             t_evt_start,
//             t_evt_end,
//             row["room"].as<string>(),
//             row["leader"].as<string>(),
//             row["description"].as<string>()
//         );

//         retval->push_back(evtwrp);
//     }
//     return retval;
// }
