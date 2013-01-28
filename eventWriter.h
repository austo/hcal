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

private:
    std::map< int, std::list<Event> >* eventMap_;
    std::map< int, std::list<Event> >* GetEventMap(v8::Array* arr);
    View view_;
    static const char* weekdays[7];
    static const char* months[12];
    const char* write_monthly_calendar();
    void write_monthly_calendar_page(HPDF_Doc, HPDF_Font, int, int);
    void write_events(HPDF_Page, float, float, int, int, int, int, int);
    static std::vector<int> build_row_array(int, int, int);
    static int get_day_row(std::vector<int>*, int);
    static void write_text(HPDF_Page, float, float, const char*);
};

#endif