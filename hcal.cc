#define BUILDING_NODE_EXTENSION
#define __DEBUG__

#ifdef __DEBUG__
#include <iostream>
#endif

#include <node.h>
#include <new>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <map>
#include <hpdf.h>
#include "throw_exception.hpp"
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
#include "eventWrapper.h"
#include "hcal.h"
#include "eventWriter.h"
#include "event.h"

//TODO: accept as param from JS
#define OFFSET -5 //Cheap way of getting UTC offset to boost::date_time without defining timezone
using namespace v8;

//extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

//TODO: move to better spot
//helper function to malloc new string from v8string
char* MallocCString(v8::Handle<Value> v8String){
    String::Utf8Value utf8String(v8String->ToString());
    char *cString = (char *) malloc(strlen(*utf8String) + 1);
    if(!cString) {
        return cString;
    }
    strcpy(cString, *utf8String);
    return cString;
}

void WriteException(TryCatch& trycatch){
    Handle<Value> exception = trycatch.Exception();
    String::AsciiValue exception_str(exception);
    printf("Exception: %s\n", *exception_str);
}

Handle<Value> CreateEvent(const Arguments& args) {
    HandleScope scope;
    return scope.Close(EventWrapper::NewInstance(args));
}

Handle<Value> Add(const Arguments& args) {
    HandleScope scope;
    EventWrapper* obj1 = node::ObjectWrap::Unwrap<EventWrapper>(args[0]->ToObject());
    EventWrapper* obj2 = node::ObjectWrap::Unwrap<EventWrapper>(args[1]->ToObject());
    double sum = obj1->Start() + obj2->Start();
    return scope.Close(Number::New(sum));
}

Handle<Value> TestEventArray(const Arguments& args){
    HandleScope scope;
    TryCatch trycatch;
    uint32_t len;
    time_t st, et;
    if (args[0]->IsArray()) {
        Array* arr = Array::Cast(*args[0]);        
        Local<Object> obj = arr->CloneElementAt(0);
        if (obj.IsEmpty()) {  
            WriteException(trycatch);
        }
        else {
            EventWrapper* evt = node::ObjectWrap::Unwrap<EventWrapper>(obj);
            len = arr->Length();
            st = evt->Start();
            et = evt->End();
            #ifdef __DEBUG__
            printf("v8 - TestJsArray array length: %d\n", len);
            printf("v8 - First JS start time is: %s\n", ctime(&st));
            printf("v8 - First JS end time is: %s\n", ctime(&et));
            printf("v8 - First event duration: %ld\n", evt->Minutes());
            std::cout << "v8 - Leader: " << evt->Leader() << std::endl;
            std::cout << "v8 - Title: " << evt->Title() << std::endl;

            time_t rawtime;
            struct tm * timeinfo;

            boost::posix_time::ptime p1(boost::posix_time::not_a_date_time);
            boost::posix_time::ptime p2;
            p1 = boost::posix_time::from_time_t(st) + boost::posix_time::hours(OFFSET);
            boost::gregorian::date d1(p1.date());

            std::cout << "v8 - ptime from start: " << p1 << std::endl;
            std::cout << "v8 - ptime time_of_day: " << p1.time_of_day() << std::endl;
            std::cout << "v8 - gregorian date day of week: " << d1.day_of_week() << std::endl;

            time (&rawtime);
            printf("v8 - Rawtime: %ld\n", rawtime);

            timeinfo = localtime (&rawtime);
            printf("v8 - Current local time and date: %s", asctime (timeinfo));
            #endif
        }
    }
    else {
       WriteException(trycatch);
    }
    return scope.Close(Number::New(st));
}

/*  TODO: changes this to provide for a function with two args:
    err, pdf
    ideally, the pdf arg should be a filestream handle, which we would
    then hand off to express...

    1. check args (need to check events array...)
    2. switch (else if) view arg
    3. call correct EventWriter method
*/    
Handle<Value> BuildCalendar(const Arguments& args) {
    HandleScope scope;
    if (args[0]->IsArray()) {
        Array* arr = Array::Cast(*args[0]);
        Local<Function> cb = Local<Function>::Cast(args[2]);
        String::AsciiValue viewStr(args[1]->ToString());
        if (strcmp(*viewStr, "month") == 0){
            const unsigned argc = 1;
            EventWriter evtWtr = EventWriter(arr, EventWriter::month);
            const char* fname = evtWtr.write_calendar();
            Local<Value> argv[argc] = { Local<Value>::New(String::New(fname)) };
            cb->Call(Context::GetCurrent()->Global(), argc, argv);
        }
    }
    return scope.Close(Undefined());
}

void InitAll(Handle<Object> target) {
    EventWrapper::Init();
    target->Set(String::NewSymbol("createEvent"),
        FunctionTemplate::New(CreateEvent)->GetFunction());

    target->Set(String::NewSymbol("add"),
        FunctionTemplate::New(Add)->GetFunction());

    target->Set(String::NewSymbol("testEventArray"),
        FunctionTemplate::New(TestEventArray)->GetFunction());

    target->Set(String::NewSymbol("buildCalendar"),
      FunctionTemplate::New(BuildCalendar)->GetFunction());
}

NODE_MODULE(hcal, InitAll)