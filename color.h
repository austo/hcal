#ifndef GUARD__COLOR_H
#define GUARD__COLOR_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <exception>
#include "hcal_utils.h"

namespace hcal {    

    class color_exception : public std::exception {        
    public:
        color_exception(){
            message_ = std::string("Color exception.");
        }
        color_exception(std::string err_msg){
            message_ = err_msg;
        }
        ~color_exception() throw(){};
        virtual const char* what() const throw(){
            return this->message_.c_str();
        }
    private:
        std::string message_;        
    };
    
    class Color {
    public:
        Color(int red, int green, int blue){
            if (params_invalid(red, green, blue)){
                throw color_exception("RGB color integers must be between 0 and 255, inclusive.");
            }
            rgb_red_ = red;
            rgb_green_ = green;
            rgb_blue_ = blue;
        }
        Color(std::string hex){
            if (hex.size() != 6){
                throw color_exception("Hexadecimal color strings must be 6 characters.");                
            }
            if (params_invalid(hex)){
                throw color_exception("Hexadecimal color string contains invalid characters.");
            }
            std::string red_str = hex.substr(0, 2), green_str = hex.substr(2, 2), blue_str = hex.substr(4, 2);            
            std::sscanf(red_str.c_str(), "%x", &rgb_red_);
            std::sscanf(green_str.c_str(), "%x", &rgb_green_);
            std::sscanf(blue_str.c_str(), "%x", &rgb_blue_);
        }
        Color(float red, float green, float blue){
            if (params_invalid(red, green, blue)){
                throw color_exception("RGB color floating point values must be between 0 and 1.0, inclusive.");
            }
            rgb_red_ = (int)(red * 255);
            rgb_green_ = (int)(green * 255);
            rgb_blue_ = (int)(blue * 255);
        }
        Color(){
            rgb_red_ = 0;
            rgb_green_ = 0;
            rgb_blue_ = 0;
        }

        int rgb_red() const { return rgb_red_; }
        int rgb_green() const { return rgb_green_; }
        int rgb_blue() const { return rgb_blue_; }

        float dec_red() const {
            return get_decimal(rgb_red_);            
        }

        float dec_green() const {
            return get_decimal(rgb_green_);            
        }

        float dec_blue() const {
            return get_decimal(rgb_blue_);            
        }

        std::string hex_val() const {
            return get_hex_value();
        }

    private:
        int rgb_red_;
        int rgb_green_;
        int rgb_blue_;
        std::string get_hex_value() const {
            int n = 0;
            std::stringstream ss;
            n += append_hex_val(ss, rgb_red_);
            n += append_hex_val(ss, rgb_green_);
            n += append_hex_val(ss, rgb_blue_);            

            if (n != 6){
                std::stringstream err_ss;
                err_ss << "Unable to write hexadecimal color string with size " << n << ".";
                throw color_exception(err_ss.str());
            }
            return ss.str();
        }

        static float get_decimal(int val){
            float temp = (float) val / 255.0;
            float rounded = floorf(temp * 10 + 0.5) / 10;
            return rounded;
        }                
        static bool params_invalid(int r, int g, int b){
            return ((r < 0 || r > 255) || (g < 0 || g > 255) || (b < 0 || b > 255));
        }
        static bool params_invalid(float r, float g, float b){
            return ((r < 0.0 || r > 1.0) || (g < 0.0 || g > 1.0) || (b < 0.0 || b > 1.0));
        }

        static bool params_invalid(std::string hex_str){
            //sorted array of all valid hex values
            char hex_chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' };
            char* ch;
            for (unsigned i = 0; i < hex_str.size(); ++i){
                ch = (char*) bsearch(&hex_str[i], hex_chars, 22, sizeof(char), compare_chars);
                if (ch == NULL){
                    return true;
                }
            }
            return false;
        }

        static int append_hex_val(std::stringstream& ss, int rgb_val){
            char buf[3];
            int n = sprintf(buf, "%x", rgb_val);
            if (n == 1){
                ss << 0;
                n += 1;
            }
            ss << buf;
            return n;
        }
    };
}
#endif