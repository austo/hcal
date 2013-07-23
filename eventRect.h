#ifndef EVENT_RECT_H
#define EVENT_RECT_H

#include "color.h"

namespace hcal{

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
        bool out_of_bounds(){
            return x == -1 && y == -1;
        }
    };

    class Event_Rect{
    public:
        Point l_left;
        Point l_right;
        Point u_left;
        Point u_right;
        Color color;
        Event_Rect(Point ll, Point lr, Point ul, Point ur){
            l_left = ll;
            l_right = lr;
            u_left = ul;
            u_right = ur;                
        }
        Event_Rect(double start_x, double start_y, double offset_x, double offset_y){
            double end_x = start_x + offset_x;
            double end_y = start_y + offset_y;
            l_left = Point(start_x, end_y);
            l_right = Point(end_x, end_y);
            u_left = Point(start_x, start_y);
            u_right = Point(end_x, start_y);
        }
        Event_Rect(double start_x, double start_y, double offset_x, double offset_y, std::string hex_color){
            double end_x = start_x + offset_x;
            double end_y = start_y + offset_y;
            l_left = Point(start_x, start_y);
            l_right = Point(end_x, start_y);
            u_left = Point(start_x, end_y);
            u_right = Point(end_x, end_y);
            color = Color(hex_color);
        }
        Event_Rect(double start_x, double start_y, double offset_x, double offset_y, Color rm_color){
            double end_x = start_x + offset_x;
            double end_y = start_y + offset_y;
            l_left = Point(start_x, start_y);
            l_right = Point(end_x, start_y);
            u_left = Point(start_x, end_y);
            u_right = Point(end_x, end_y);
            color = rm_color;
        }
        Event_Rect(){
            l_left = Point(-1, -1);
            l_right = Point(-1, -1);
            u_left = Point(-1, -1);
            u_right = Point(-1, -1);
        }
        bool is_out_of_bounds(){
            return l_left.out_of_bounds() && l_right.out_of_bounds() && u_right.out_of_bounds() && u_left.out_of_bounds();
        }
    };
}
#endif