#ifndef GUARD__HCAL_UTILS_H
#define GUARD__HCAL_UTILS_H

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

namespace hcal {

    static const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* months[] = {"January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"}; 

    void error_handler (HPDF_STATUS, HPDF_STATUS, void*);
    HPDF_Doc get_pdf();
    void write_text(HPDF_Page, float, float, const char*);
    void write_page_title(HPDF_Page, HPDF_Font, const char*);
    void draw_line(HPDF_Page, float, float, float, float);
    void write_weekday_cols(HPDF_Page, HPDF_Font, int, int, int, float);      
}
#endif