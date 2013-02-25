#ifndef GUARD__WEEKWRITER_H
#define GUARD__WEEKWRITER_H

#include "calWriter.h"
#include "dataLayer.h"

namespace hcal {

    class WeekWriter : public CalWriter {

    public:
        WeekWriter(v8::Array*);
        WeekWriter(time_t start, time_t end);
        WeekWriter(time_t start, time_t end, int, int);
        WeekWriter();
        ~WeekWriter();
        const char* write_calendar();

    private:
        int start_hour_;
        int end_hour_;
        double slot_height_;
        std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr);
        void write_calendar_page(HPDF_Doc, HPDF_Font, int);
        void write_events(HPDF_Page, float, float, int, int, int, int, int);
        void write_hour_rows(HPDF_Page, int, int);
    };
}

#endif