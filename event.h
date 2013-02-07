#ifndef GUARD__INNEREVENT_H
#define GUARD__INNEREVENT_H
#define BUILDING_NODE_EXTENSION

#ifndef OFFSET
    #define OFFSET -5 //TODO: improve using real boost timezones
#endif

#include <ctime>
#include <string>
#include "eventWrapper.h"
#include "posix_time/posix_time.hpp"

class Event {
    public:
        Event(int, time_t, time_t, int, std::string, std::string);
        Event(int, boost::posix_time::ptime, boost::posix_time::ptime, int, std::string, std::string);
        Event(EventWrapper*);
        //TODO: may want to add get_week_number and get_day_number
        int Id() const { return id_; }
        boost::posix_time::ptime Start() const { return start_; }
        boost::posix_time::ptime End() const { return end_; }
        int Room() const { return room_; }
        std::string Leader() const { return leader_; }
        std::string Title() const { return title_; }
        bool operator< (const Event &other) const {
            if (Start() == other.Start()){
                return Id() < other.Id();
            }
            return Start() < other.Start();
        }

    private:
        int id_;
        boost::posix_time::ptime start_;
        boost::posix_time::ptime end_;
        int room_;
        std::string leader_;
        std::string title_;
};

#endif