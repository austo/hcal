#define BUILDING_NODE_EXTENSION
#ifndef OFFSET
    #define OFFSET -5 //TODO: improve using real boost timezones
#endif

#ifndef BOOST_NO_EXCEPTIONS
#define BOOST_NO_EXCEPTIONS
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
    end_ = boost::posix_time::from_time_t(end) + boost::posix_time::hours(OFFSET);;
    room_ = room;
    leader_ = leader;
    title_ = title;
}

Event::Event(EventWrapper* evt){
    //Event(evt->Id(), evt->Start(), evt->End(), evt->Room(), evt->Leader(), evt->Title());
    id_ = evt->Id();
    start_ = boost::posix_time::from_time_t(evt->Start()) + boost::posix_time::hours(OFFSET);
    end_ = boost::posix_time::from_time_t(evt->End()) + boost::posix_time::hours(OFFSET);
    room_ = evt->Room();
    leader_ = evt->Leader();
    title_ = evt->Title();
}
