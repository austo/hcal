#ifndef GUARD__DATALAYER_H
#define GUARD__DATALAYER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <pqxx/pqxx>
#include "event.h"
#include "posix_time/posix_time.hpp"



class DataLayer {
    public:
        DataLayer();
        ~DataLayer();
        std::vector<Event>* get_events(time_t, time_t);
        

    private:
        
};

#endif