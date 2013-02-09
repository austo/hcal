#define BUILDING_NODE_EXTENSION
#include "eventWrapper.h"

using namespace v8;

#define THROW(msg) return ThrowException(Exception::Error(String::New(msg)));

EventWrapper::EventWrapper() {};
EventWrapper::~EventWrapper() {};

Persistent<Function> EventWrapper::constructor;

void EventWrapper::Init() {
    //Constructor function template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Event"));
    tpl->InstanceTemplate()->SetInternalFieldCount(6);
    
    //Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("id"),
        FunctionTemplate::New(GetId)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("start"),
        FunctionTemplate::New(GetStart)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("end"),
        FunctionTemplate::New(GetEnd)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("roomId"),
        FunctionTemplate::New(GetRoomId)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("duration"),
        FunctionTemplate::New(GetDuration)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("description"),
        FunctionTemplate::New(GetDescription)->GetFunction());
    constructor = Persistent<Function>::New(tpl->GetFunction());
}

/*
    Incoming order:
    int id_;
    time_t start_;
    time_t end_;
    int room_id_;
    std::string room_name_;
    int leader_id_;
    std::string leader_name_;
    std::string description_;
*/

Handle<Value> EventWrapper::New(const Arguments& args) {
    HandleScope scope;    
    EventWrapper* evt = new EventWrapper();
    //JS dates divided by 1000 to account for default posix time resolution
    evt->id_ = args[0]->IsUndefined() ? -1 : args[0]->NumberValue();
    evt->start_ = args[1]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[1]);
    evt->end_ = args[2]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[2]);
    evt->room_id_ = args[3]->IsUndefined() ? 0 : args[3]->NumberValue();
    //leader_id_
    evt->leader_id_ = args[4]->IsUndefined() ? 0 : args[4]->NumberValue();

    //Description
    if (!args[5]->IsUndefined()){
        String::Utf8Value temp_title(args[5]->ToString());
        #ifdef __DEBUG__
        printf("v8 - title string: %s\n", *temp_title);
        #endif
        evt->description_ = std::string(*temp_title);
        #ifdef __DEBUG__
        std::cout << "v8 - " << evt->description_ << std::endl;
        #endif
    }
    else{
        evt->description_ = std::string("No description");
    }
    evt->Wrap(args.This());
    return args.This();
}


//For building an event from c++
Handle<Value> EventWrapper::get_wrapped_object(int id, time_t start, time_t end,
    int room_id, int leader_id, std::string description) {
    HandleScope scope;

    const unsigned argc = 6;
    Handle<Value> argv[argc] =  {   Number::New(id),
                                    NODE_UNIXTIME_V8(start), NODE_UNIXTIME_V8(end),
                                    Number::New(room_id), Number::New(leader_id),
                                    String::New(description.c_str())
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

Handle<Value> EventWrapper::GetId(const v8::Arguments& args){
    HandleScope scope;
    EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
    return scope.Close(Number::New(evt->Id()));
}

Handle<Value> EventWrapper::GetStart(const v8::Arguments& args){
    HandleScope scope;
    EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
    return scope.Close(NODE_UNIXTIME_V8(evt->Start()));
}

Handle<Value> EventWrapper::GetEnd(const v8::Arguments& args){
    HandleScope scope;
    EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
    return scope.Close(NODE_UNIXTIME_V8(evt->End()));
}

Handle<Value> EventWrapper::GetRoomId(const v8::Arguments& args){
    HandleScope scope;
    EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
    return scope.Close(Number::New(evt->RoomId()));
}

Handle<Value> EventWrapper::GetDuration(const Arguments& args) {
  HandleScope scope;
  EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
  return scope.Close(Number::New(evt->Duration()));
}

Handle<Value> EventWrapper::GetDescription(const Arguments& args) {
  HandleScope scope;
  EventWrapper* evt = ObjectWrap::Unwrap<EventWrapper>(args.This());
  return scope.Close(String::New(evt->Description().c_str()));
}
