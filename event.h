#ifndef INNEREVENT_H
#define INNEREVENT_H

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
        Event(int, boost::posix_time::ptime, boost::posix_time::ptime, int, int, std::string);
        Event(EventWrapper*);
        //TODO: may want to add get_week_number and get_day_number
        int Id() const { return id_; }
        boost::posix_time::ptime Start() const { return start_; }
        boost::posix_time::ptime End() const { return end_; }
        int RoomId() const { return room_id_; }
        std::string LeaderName() const { return leader_name_; }
        std::string Description() const { return description_; }
        bool operator< (const Event& other) const {
            if (Start() == other.Start()){
                if (End() == other.End()){
                    return RoomId() < other.RoomId();
                }
                return End() < other.End();
            }
            return Start() < other.Start();
        }
        bool intersects(const Event& other);

    private:
        int id_;
        boost::posix_time::ptime start_;
        boost::posix_time::ptime end_;
        int room_id_;
        std::string room_name_;
        int leader_id_;
        std::string leader_name_;
        std::string description_;
};

#endif