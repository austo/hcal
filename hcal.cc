#define BUILDING_NODE_EXTENSION
#define __DEBUG__

#include "hcal.h"

//TODO: accept as param from JS
#define OFFSET -5 //Cheap way of getting UTC offset to boost::date_time without defining timezone
#define THROW(msg) return ThrowException(Exception::Error(String::New(msg)));

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

Handle<Value> InsertEvent(const Arguments& args) {
    HandleScope scope;
    if (args.Length() != 7) {
        THROW("Wrong number of arguments - hcal.insertEvent() must be called with 7 args.");
        return scope.Close(Undefined());
    }    
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || 
        args[3]->IsUndefined() || args[4]->IsUndefined() || args[5]->IsUndefined()) {
        THROW("All arguments must be defined.");
        return scope.Close(Undefined());
    }
    if (args[6]->IsUndefined()){
        THROW("Inserted event callback must be defined.");
    }

    Handle<Value> retval;
    Local<Function> cb = Local<Function>::Cast(args[6]);
    const unsigned argc = 2;

    try{
        time_t start = args[0]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[0]);
        time_t end = args[1]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[1]);
        int room_id = args[2]->IsUndefined() ? 0 : args[2]->NumberValue();
        int leader_id = args[3]->IsUndefined() ? 0 : args[3]->NumberValue();
        std::string desc;
        if (!args[4]->IsUndefined()){
            String::Utf8Value temp_desc(args[4]->ToString());
            #ifdef __DEBUG__
            printf("v8 - desc string: %s\n", *temp_desc);
            #endif
            desc = std::string(*temp_desc);        
        }
        else{
            desc = std::string("No description");
        }
        bool recurring = args[5]->IsUndefined() ? false : args[5]->BooleanValue();

        hcal::DataLayer dl = hcal::DataLayer();
        retval = dl.insert_event(start, end, room_id, leader_id, desc, recurring);

        Local<Value> argv[argc] = { Local<Value>::New(Undefined()), Local<Value>::New(retval) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    catch (std::exception& e){
        std::stringstream ss;
        ss << "hcal.insertEvent threw exception: " << e.what();
        std::string ex_str = ss.str();
        Local<Value> argv[argc] = { Local<Value>::New(String::New(ex_str.c_str())), Local<Value>::New(Undefined()) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(Undefined());
}

Handle<Value> CreateConfig(const Arguments& args) {
    HandleScope scope;
    return scope.Close(ConfigWrapper::NewInstance(args));
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
    std::vector<EventWrapper>* evtWprs = 0;
    Handle<Array> retval;
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
            printf("v8 - First event duration: %ld\n", evt->Duration());
            std::cout << "v8 - Title: " << evt->Description() << std::endl;

            time_t rawtime;
            struct tm * timeinfo;

            hcal::DataLayer dl = hcal::DataLayer();
            retval = dl.get_wrapped_events(st, et);            

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
    return scope.Close(retval);   
}

Handle<Value> BuildCalendar(const Arguments& args) {
    HandleScope scope;
    if (args[0]->IsArray()) {
        Array* arr = Array::Cast(*args[0]);
        Local<Function> cb = Local<Function>::Cast(args[2]);
        String::AsciiValue viewStr(args[1]->ToString());
        const unsigned argc = 1;
        try{
            EventWriter::View view = EventWriter::get_view(viewStr);
            EventWriter evtWtr = EventWriter(arr, view);
            const char* fname = evtWtr.write_calendar();
            Local<Value> argv[argc] = { Local<Value>::New(String::New(fname)) };
            cb->Call(Context::GetCurrent()->Global(), argc, argv);
        }
        catch(std::exception& e){
            THROW(e.what());
        }    
    }
    return scope.Close(Undefined());
}


Handle<Value> PrintCalendar(const Arguments& args) {
    HandleScope scope;

    if (args.Length() != 4) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }    
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || args[3]->IsUndefined()){
        THROW("All arguments must be defined.");
        return scope.Close(Undefined());
    }
    time_t start = NODE_V8_UNIXTIME(args[0]);
    time_t end =  NODE_V8_UNIXTIME(args[1]);
    String::AsciiValue viewStr(args[2]->ToString());
    Local<Function> cb = Local<Function>::Cast(args[3]);
    const unsigned argc = 1;

    try{
        if (strcmp(*viewStr, "month") == 0){
            hcal::MonthWriter month_wtr = hcal::MonthWriter(start, end);
            const char* fname = month_wtr.write_calendar();
            Local<Value> argv[argc] = { Local<Value>::New(String::New(fname)) };
            cb->Call(Context::GetCurrent()->Global(), argc, argv);
        }

        // EventWriter::View view = EventWriter::get_view(viewStr);
        // hcal::DataLayer dl = hcal::DataLayer();
        // std::map<int, std::list<Event> >* emap = dl.get_event_map(start, end);
        // EventWriter evtWtr = EventWriter(emap, view);
        // const char* fname = evtWtr.write_calendar();
        // Local<Value> argv[argc] = { Local<Value>::New(String::New(fname)) };
        // cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    catch(std::exception& e){
        THROW(e.what());
    }    
    return scope.Close(Undefined());
}


void InitAll(Handle<Object> target) {
    EventWrapper::Init();
    ConfigWrapper::Init();
    target->Set(String::NewSymbol("createEvent"),
        FunctionTemplate::New(CreateEvent)->GetFunction());

    target->Set(String::NewSymbol("insertEvent"),
        FunctionTemplate::New(InsertEvent)->GetFunction());

    target->Set(String::NewSymbol("createConfig"),
        FunctionTemplate::New(CreateConfig)->GetFunction());

    target->Set(String::NewSymbol("add"),
        FunctionTemplate::New(Add)->GetFunction());

    target->Set(String::NewSymbol("testEventArray"),
        FunctionTemplate::New(TestEventArray)->GetFunction());

    target->Set(String::NewSymbol("buildCalendar"),
      FunctionTemplate::New(BuildCalendar)->GetFunction());

    target->Set(String::NewSymbol("printCalendar"),
      FunctionTemplate::New(PrintCalendar)->GetFunction());
}

NODE_MODULE(hcal, InitAll)