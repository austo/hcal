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

    protected:
        class Point{
        public:
            double x;
            double y;
            Point(double x_init, double y_init){
                x = x_init;
                y = y_init;
            }
            Point(){
                x = 0.0;
                y = 0.0;
            }
        };
        class Event_Rect{
        public:
            Point l_left;
            Point l_right;
            Point u_left;
            Point u_right;
            Event_Rect(Point ll, Point lr, Point ul, Point ur){
                l_left = ll;
                l_right = lr;
                u_left = ul;
                u_right = ur;
            }
            Event_Rect(double start_x, double start_y, double offset_x, double offset_y){
                double end_x = start_x + offset_x;
                double end_y = start_y + offset_y;
                l_left = Point(start_x, start_y);
                l_right = Point(end_x, start_y);
                u_left = Point(start_x, end_y);
                u_right = Point(end_x, end_y);
            }
        };

    private:
        int start_hour_;
        int end_hour_;
        double slot_height_;
        double slot_width_;
        std::map< int, std::list<Event> >* get_evt_map(v8::Array* arr);
        void write_calendar_page(HPDF_Doc, HPDF_Font, int);
        void write_events(HPDF_Page, int);
        void write_hour_rows(HPDF_Page, int, int);
        Event_Rect get_slot_position(std::list<Event>::const_iterator&);    
    };
}

#endif