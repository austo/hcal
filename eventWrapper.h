#ifndef GUARD__EVENTWRAPPER_H
#define GUARD__EVENTWRAPPER_H
#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <ctime>
#include <string>
#include <iostream>

class EventWrapper : public node::ObjectWrap {
    friend class DataLayer;
    public:
        EventWrapper();
        ~EventWrapper();

        static void Init();
        static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
        static v8::Handle<v8::Value> get_wrapped_object(int, time_t, time_t, int, int, std::string);
        int Id() const { return id_; }
        time_t Start() const { return start_; }
        time_t End() const { return end_; }
        int RoomId() const { return room_id_; }
        std::string LeaderName() const { return leader_name_; }
        std::string Description() const { return description_; }
        time_t Duration() const { return (end_ - start_) / 60; }        

    private:
        static v8::Persistent<v8::Function> constructor;
        static v8::Handle<v8::Value> New(const v8::Arguments& args);
        static v8::Handle<v8::Value> GetDuration(const v8::Arguments& args);
        static v8::Handle<v8::Value> GetDescription(const v8::Arguments& args);
        int id_;
        time_t start_;
        time_t end_;
        int room_id_;
        std::string room_name_;
        int leader_id_;
        std::string leader_name_;
        std::string description_;

};

#endif