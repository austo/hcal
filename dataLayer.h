#ifndef GUARD__DATALAYER_H
#define GUARD__DATALAYER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <node.h>
#include <pqxx/pqxx>
#include "event.h"
#include "eventWrapper.h"
#include "posix_time/posix_time.hpp"



class DataLayer {
    public:
        DataLayer();
        ~DataLayer();
        v8::Handle<v8::Array> get_wrapped_events(time_t, time_t);
        

    private:
        pqxx::result execute_query(pqxx::transaction_base&, std::string);
        v8::Handle<v8::Array> build_wrapped_events(pqxx::result&);
        
};

#endif