#ifndef GUARD__MONTHWRITER_H
#define GUARD__MONTHWRITER_H

#include "calWriter.h"
#include "dataLayer.h"

namespace hcal {

    class MonthWriter : public CalWriter {

    public:
        MonthWriter(v8::Array*);
        MonthWriter(time_t start, time_t end);
        MonthWriter();
        ~MonthWriter();
        const char* write_calendar();

    private:
        std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr);
        void write_calendar_page(HPDF_Doc, HPDF_Font, int, int);
        void write_events(HPDF_Page, float, float, int, int, int, int, int);
        static std::vector<int> build_row_array(int, int, int);
        static int get_day_row(std::vector<int>*, int);
        int write_wrapped_event_title(HPDF_Page, float, float, const std::string, float);
    };
}

#endif