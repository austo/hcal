#ifndef GUARD__EVENTWRITER_H
#define GUARD__EVENTWRITER_H

#ifndef BOOST_NO_EXCEPTIONS
#define BOOST_NO_EXCEPTIONS
#endif

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
    EventWriter(std::vector<Event>*, View);
    EventWriter(v8::Array *, View);
    EventWriter();
    ~EventWriter();
    void set_events(std::vector<Event>*);
    const char* write_calendar();

private:
    std::vector<Event>* events_;
    std::map< int, std::list<Event> >* eventMap_;
    std::map< int, std::list<Event> >* GetEventMap(v8::Array* arr);
    View view_;
    static const char* weekdays[7];
    static const char* months[12];
    const char* write_monthly_calendar(int, int);
    const char* write_monthly_calendar();
    void write_events(HPDF_Page, float, float, int, int, int, int, int);
    static std::vector<int> build_row_array(int, int, int);
    static int get_day_row(std::vector<int>*, int);
    static void write_text(HPDF_Page, float, float, const char*);

};

#endif