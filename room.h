#ifndef ROOM_H
#define ROOM_H

#include <string>
#include "color.h"


namespace hcal{

    class Room{
    public:
        int id;
        std::string name;
        Color color;
        
        Room(){
           //defalt member constructors
        }

        Room(int param_id, std::string param_name, std::string param_hex_color){
            id = param_id;
            name = param_name;
            color = Color(param_hex_color);
        }
    }; 
}
#endif