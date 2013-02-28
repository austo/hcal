#include <cstdio>
#include <cstdlib>
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

        int rgb_red() const { return rgb_red_; }
        int rgb_green() const { return rgb_green_; }
        int rgb_blue() const { return rgb_blue_; }

    private:
        int rgb_red_;
        int rgb_green_;
        int rgb_blue_;

    };
}