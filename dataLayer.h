#ifndef GUARD__DATALAYER_H
#define GUARD__DATALAYER_H

#include <sstream>
#include <string>
#include <ctime>
#include <map>
#include <list>
#include <cstdlib>
#include <node.h>
#include <pqxx/pqxx>
#include <exception>
#include "throw_exception.hpp"
#include "eventWrapper.h"
#include "event.h"
#include "posix_time/posix_time.hpp"

namespace hcal{

    class DataLayer {

        public:
            DataLayer();
            ~DataLayer();
            v8::Handle<v8::Array> get_wrapped_events(time_t, time_t);
            //TODO: handle different views
            std::map<int, std::list<Event> >* get_event_map(time_t, time_t);
            v8::Handle<v8::Value> insert_event(time_t, time_t, int, int, std::string, bool);
            bool update_event(int, time_t, time_t, int, int, std::string, bool);
            static time_t get_time_t_from_ptime(boost::posix_time::ptime); 

        private:
            pqxx::result get_events_for_timespan(time_t start, time_t end);
            pqxx::result execute_query(pqxx::transaction_base&, std::string);
            void populate_emap(pqxx::result&, std::map<int, std::list<Event> >*);
            v8::Handle<v8::Array> build_wrapped_events(pqxx::result&);
            std::string get_env(const std::string&);
            boost::posix_time::time_duration utc_offset_td_;    
    };
}

#endif