#ifndef GUARD__WEEKWRITER_H
#define GUARD__WEEKWRITER_H

#include "calWriter.h"
#include "dataLayer.h"
#include "eventRect.h"
#include "room.h"

namespace hcal {

    class WeekWriter : public CalWriter {

    public:
        WeekWriter(v8::Array*);
        WeekWriter(time_t start, time_t end, int);
        WeekWriter(time_t start, time_t end, int, int, int);
        WeekWriter();
        ~WeekWriter();
        const char* write_calendar();        

    private:
        int start_hour_;
        int end_hour_;
        double slot_height_;
        double slot_width_;
        int pg_used_height_;
        int pg_used_width_;
        std::map<int, Room> rooms_;
        std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr);
        void write_calendar_page(HPDF_Doc, HPDF_Font, int);
        void write_events(HPDF_Page, int);
        void write_hour_rows(HPDF_Page, HPDF_Font);
        Event_Rect get_event_rect(std::list<Event>::const_iterator&, const int&, int&);
        static int get_overlapping_events(std::list<Event>::const_iterator);
    };
}

#endif