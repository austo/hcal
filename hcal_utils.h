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

    inline const char** weekdays() {
        static const char* retval[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        return retval;
    }

    inline const char** months() {
        static const char* retval[] = {"January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"};
        return retval;
    }

    enum View {day = 1, week, month, custom};
    void error_handler (HPDF_STATUS, HPDF_STATUS, void*);
    HPDF_Doc get_pdf();
    void write_text(HPDF_Page, float, float, const char*);
    void write_wrapped_text(HPDF_Page, float, float&, const std::string, float, float);
    std::vector<std::string> get_words(const std::string, const char*);
    void write_page_title(HPDF_Page, HPDF_Font, const char*);
    void draw_line(HPDF_Page, float, float, float, float);
    void write_weekday_cols(HPDF_Page, HPDF_Font, int, int, int, float);
    View get_view(v8::String::AsciiValue&);  
}
#endif