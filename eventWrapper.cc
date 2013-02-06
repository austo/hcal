#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <iostream>
#include <string>
#include <ctime>
#include "eventWrapper.h"

using namespace v8;

EventWrapper::EventWrapper() {};
EventWrapper::~EventWrapper() {};

Persistent<Function> EventWrapper::constructor;

void EventWrapper::Init() {
    //Constructor function template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Event"));
    tpl->InstanceTemplate()->SetInternalFieldCount(6);
    
    //Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("duration"),
        FunctionTemplate::New(Duration)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("description"),
        FunctionTemplate::New(Description)->GetFunction());
    constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> EventWrapper::New(const Arguments& args) {
    HandleScope scope;    
    EventWrapper* evt = new EventWrapper();
    //JS dates divided by 1000 to account for default posix time resolution
    evt->id_ = args[0]->IsUndefined() ? -1 : args[0]->NumberValue();
    evt->start_ = args[1]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[1]);
    evt->end_ = args[2]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[2]);
    evt->room_ = args[3]->IsUndefined() ? 0 : args[3]->NumberValue();

    //Leader
    if (!args[4]->IsUndefined()){
        String::Utf8Value temp_leader(args[4]->ToString());
        #ifdef __DEBUG__
        printf("v8 - leader name: %s\n", *temp_leader);
        #endif
        evt->leader_ = std::string(*temp_leader);
        #ifdef __DEBUG__
        std::cout << "v8 - " << evt->leader_ << std::endl;
        #endif
    }
    else{
        evt->leader_ = std::string("No leader");
        #ifdef __DEBUG__
        std::cout << "v8 - " << evt->leader_ << std::endl;
        #endif
    }

    //Title
    if (!args[5]->IsUndefined()){
        String::Utf8Value temp_title(args[5]->ToString());
        #ifdef __DEBUG__
        printf("v8 - title string: %s\n", *temp_title);
        #endif
        evt->title_ = std::string(*temp_title);
        #ifdef __DEBUG__
        std::cout << "v8 - " << evt->title_ << std::endl;
        #endif
    }
    else{
        evt->title_ = std::string("No title");
    }
    evt->Wrap(args.This());
    return args.This();
}

Handle<Value> EventWrapper::get_wrapped_object(int id, time_t start, time_t end,
    std::string room_name, std::string leader, std::string title) {
    HandleScope scope;

    const unsigned argc = 6;
    Handle<Value> argv[argc] =  {   Number::New(id),
                                    NODE_UNIXTIME_V8(start), NODE_UNIXTIME_V8(end),
                                    Number::New(0), String::New(leader.c_str()),
                                    String::New(title.c_str())
                                };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);    
}

Handle<Value> EventWrapper::NewInstance(const Arguments& args) {
    HandleScope scope;
    if (args.Length() != 6) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }
    const unsigned argc = 6;
    Handle<Value> argv[argc] = { args[0], args[1], args[2], args[3], args[4], args[5] };
    Local<Object> instance = constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

Handle<Value> EventWrapper::Duration(const Arguments& args) {
  HandleScope scope;
  EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
  return scope.Close(Number::New(evt->Minutes()));
}

Handle<Value> EventWrapper::Description(const Arguments& args) {
  HandleScope scope;
  EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
  return scope.Close(String::New(evt->Title().c_str()));
}
