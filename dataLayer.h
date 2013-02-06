#ifndef GUARD__DATALAYER_H
#define GUARD__DATALAYER_H

#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <node.h>
#include <pqxx/pqxx>
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
        std::string get_env(const std::string&);
        int utc_offset_;     
};

#endif