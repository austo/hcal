#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <exception>

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

    private:
        int rgb_red_;
        int rgb_green_;
        int rgb_blue_;
        //static char hex_chars_[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        char* hex_chars() const {
            char retval[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
            return retval;
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
    };
}