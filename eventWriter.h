#ifndef GUARD__EVENTWRITER_H
#define GUARD__EVENTWRITER_H

#include <node.h>
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
#include <hpdf.h>
#include <vector>
#include <map>
#include <list>
#include "event.h"

class EventWriter {

public:
    enum View {day = 1, week, month, custom};
    EventWriter(v8::Array *, View);
    EventWriter();
    ~EventWriter();
    const char* write_calendar();
    static EventWriter::View get_view(v8::String::AsciiValue&);

private:
    View view_;
    std::map< int, std::list<Event> >* eventMap_;
    std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr);
    void package_evt_map(std::map< int, std::list<Event> >*, v8::Array*, uint32_t);
    static const char* weekdays[7];
    static const char* months[12];
    const char* write_monthly_calendar();
    const char* write_weekly_calendar();
    void write_weekday_cols(HPDF_Page, HPDF_Font, int, int);
    void write_monthly_calendar_page(HPDF_Doc, HPDF_Font, int, int);
    void write_weekly_calendar_page(HPDF_Doc, HPDF_Font, int);
    void write_events(HPDF_Page, float, float, int, int, int, int, int);
    static std::vector<int> build_row_array(int, int, int);
    static int get_day_row(std::vector<int>*, int);
    static HPDF_Doc get_pdf();
    static void write_text(HPDF_Page, float, float, const char*);
    static void write_page_title(HPDF_Page, HPDF_Font, const char*);
    static void write_weekly_hour_rows(HPDF_Page, int, int, int, int, double&);
    static void draw_line(HPDF_Page, float, float, float, float);
};

#endif