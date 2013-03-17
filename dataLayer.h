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
#include "room.h"
#include "hcal_utils.h"
#include "posix_time/posix_time.hpp"
#include "date_time/local_time_adjustor.hpp"
#include "date_time/c_local_time_adjustor.hpp"


namespace hcal{

    class dl_exception : public std::exception {
    public:
        dl_exception(){
            message_ = std::string("Data integrity exception.");
        }
        dl_exception(std::string err_msg){
            message_ = err_msg;
        }
        ~dl_exception() throw(){};
        virtual const char* what() const throw(){
            return this->message_.c_str();
        }
    private:
        std::string message_;        
    };

    class DataLayer {

        public:
            DataLayer();
            ~DataLayer();
            enum UpdateStatus {success = 0, failure, no_event};
            v8::Handle<v8::Array> get_wrapped_events(time_t, time_t);
            //TODO: handle different views
            std::map<int, std::list<Event> >* get_event_map(time_t, time_t, View);
            v8::Handle<v8::Value> insert_event(time_t, time_t, int, int, std::string, bool);
            UpdateStatus update_event(int, time_t, time_t, int, int, std::string, bool);
            UpdateStatus delete_event(int);
            std::map<int, std::string> get_room_colors();
            std::map<int, Room> get_rooms();
            static time_t get_time_t_from_ptime(boost::posix_time::ptime); 


        private:
            pqxx::result get_events_for_timespan(time_t start, time_t end);
            pqxx::result execute_query(pqxx::transaction_base&, std::string);
            void populate_emap(pqxx::result&, std::map<int, std::list<Event> >*, View);
            static void get_week_offset_for_current_year(boost::posix_time::ptime&, boost::gregorian::date&, int&, int&, int&);
            v8::Handle<v8::Array> build_wrapped_events(pqxx::result&);
            std::string get_env(const std::string&);
            //boost::posix_time::time_duration utc_offset_td_;    
            std::string time_region_;
    };
}

#endif