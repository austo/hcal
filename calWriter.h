#ifndef GUARD__CALWRITER_H
#define GUARD__CALWRITER_H

#include <node.h>
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
#include <hpdf.h>
#include <vector>
#include <map>
#include <list>
#include "event.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "hcal_utils.h"

namespace hcal {

    class CalWriter {

    public:
        virtual ~CalWriter() { if (eventMap_) delete eventMap_; };
        virtual const char* write_calendar() = 0;

    protected:
        HPDF_Doc doc_;
        std::map< int, std::list<Event> >* eventMap_;
        virtual std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr) = 0;    
        virtual void write_events(HPDF_Page, float, float, int, int, int, int, int) = 0;          
    };
}

#endif