#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <pqxx/pqxx>
#include "event.h"
#include "posix_time/posix_time.hpp"

using namespace std;
using namespace pqxx;

#define CONNSTRING "host=queequeg dbname=concerto user=appUser password=Hell0Sql"

DataLayer::DataLayer(){

}

DataLayer::~DataLayer(){

}

vector<Event>*
DataLayer::get_events(time_t start, time_t end){
    vector<Event>* retval = new vector<Event>();
    pqxx::connection c(CONNSTRING);
    pqxx::work txn(c);


    return retval;
}

Event
DataLayer::build_evt_from_transaction_row()

