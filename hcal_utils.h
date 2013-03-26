#ifndef GUARD__HCAL_UTILS_H
#define GUARD__HCAL_UTILS_H

#include <node.h>
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
#include "algorithm/string.hpp"
#include <hpdf.h>
#include <hyphen.h>
#include <vector>
#include <map>
#include <list>
#include "event.h"
#include "eventRect.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <utility>

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

    inline HyphenDict* hyphen_dict() { //TODO: is this the most efficient way to keep in memory?
        static HyphenDict* dict = hnj_hyphen_load("en_US.dic");
        if (dict == NULL){
            throw std::runtime_error("hcal_utils: unable to load hyphenation dictionary.");
        }
        return dict;
    }

    enum View {day = 1, week, month, custom};
    void error_handler (HPDF_STATUS, HPDF_STATUS, void*);
    HPDF_Doc get_pdf();
    void write_text(HPDF_Page, float, float, const char*);
    void write_if_more_evts(int, HPDF_Page, float, float);
    void write_wrapped_text(HPDF_Page, float, float&, const std::string, float, float);
    std::pair<std::string, std::string> hyphenate_word(HyphenDict*, const char*, unsigned);
    //void hyphenate_word(HyphenDict*, const char*, unsigned);
    std::vector<std::string> get_words(const std::string, const char*);
    void write_page_title(HPDF_Page, HPDF_Font, const char*);
    void draw_line(HPDF_Page, float, float, float, float);
    void write_weekday_cols(HPDF_Page, HPDF_Font, int, int, int, float);
    View get_view(v8::String::AsciiValue&);
    void draw_event_rect(HPDF_Page, Event_Rect&);
    void write_hour_to_buf(char*, int, bool);
    boost::posix_time::time_duration get_time_offset_from_pg_timestamp(const std::string&);
}
#endif