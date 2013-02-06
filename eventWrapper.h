#ifndef GUARD__EVENTWRAPPER_H
#define GUARD__EVENTWRAPPER_H
#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <ctime>
#include <string>

class EventWrapper : public node::ObjectWrap {
    public:
        EventWrapper();
        EventWrapper(int, time_t, time_t, std::string, std::string, std::string);
        ~EventWrapper();

        static void Init();
        static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
        int Id() const { return id_; }
        time_t Start() const { return start_; }
        time_t End() const { return end_; }
        int Room() const { return room_; }
        std::string Leader() const { return leader_; }
        std::string Title() const { return title_; }
        time_t Minutes() const { return (end_ - start_) / 60; }        

    private:
        static v8::Persistent<v8::Function> constructor;
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> Duration(const v8::Arguments& args);
        int id_;
        time_t start_;
        time_t end_;
        int room_;
        std::string leader_;
        std::string title_;
        std::string room_name_;
};

#endif