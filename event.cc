#define BUILDING_NODE_EXTENSION
#ifndef OFFSET
    #define OFFSET -5 //TODO: improve using real boost timezones
#endif

#include <ctime>
#include <string>
#include "event.h"
#include "eventWrapper.h"
#include "posix_time/posix_time.hpp"


using namespace std;

Event::Event(int id, time_t start, time_t end, int room, std::string leader, std::string title){
    id_ = id;
    start_ = boost::posix_time::from_time_t(start) + boost::posix_time::hours(OFFSET);
    end_ = boost::posix_time::from_time_t(end) + boost::posix_time::hours(OFFSET);
    room_id_ = room;
    leader_name_ = leader;
    description_ = title;
}

Event::Event(int id, boost::posix_time::ptime start,
    boost::posix_time::ptime end, int room_id, int leader_id, std::string desc){
    id_ = id;
    start_ = start + boost::posix_time::hours(OFFSET);
    end_ = end + boost::posix_time::hours(OFFSET);
    room_id_ = room_id;
    leader_id_ = leader_id;
    description_ = desc;
}

Event::Event(int id, boost::posix_time::ptime start,
    boost::posix_time::ptime end, int room, std::string leader, std::string title){

    id_ = id;
    start_ = start + boost::posix_time::hours(OFFSET);
    end_ = end + boost::posix_time::hours(OFFSET);
    room_id_ = room;
    leader_name_ = leader;
    description_ = title;
}

Event::Event(EventWrapper* evt){
    id_ = evt->Id();
    start_ = boost::posix_time::from_time_t(evt->Start()) + boost::posix_time::hours(OFFSET);
    end_ = boost::posix_time::from_time_t(evt->End()) + boost::posix_time::hours(OFFSET);
    room_id_ = evt->RoomId();
    //leader_ = evt->Leader();
    description_ = evt->Description();
}
