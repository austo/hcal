#ifndef GUARD__CONFIGWRAPPER_H
#define GUARD__CONFIGWRAPPER_H
#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <ctime>
#include <string>

class ConfigWrapper : public node::ObjectWrap {
    public:
        ConfigWrapper();
        ConfigWrapper(int sday, int eday, int shour, int ehour){
            sday_ = sday;
            eday_ = eday;
            shour_ = shour;
            ehour_ = ehour;
        }
        ~ConfigWrapper();

        static void Init();
        static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
        int StartDay() const { return sday_; }
        int EndDay() const { return eday_; };
        int StartHour() const { return shour_; }
        int EndHour() const { return ehour_; }

    private:
        static v8::Persistent<v8::Function> constructor;
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        int sday_;
        int eday_;
        int shour_;
        int ehour_;
};

#endif