#define __DEBUG__

#include "hcal.h"

//TODO: accept as param from JS
#define OFFSET -5 //Cheap way of getting UTC offset to boost::date_time without defining timezone
#define THROW(msg) return ThrowException(Exception::Error(String::New(msg)));

using namespace v8;
//Persistent<Object> module_handle;

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
    catch (hcal::dl_exception& dl_e){        
        Local<Value> argv[argc] = { Local<Value>::New(String::New(dl_e.what())), Local<Value>::New(Undefined()) };
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

Handle<Value> UpdateEvent(const Arguments& args) {
    HandleScope scope;
    if (args.Length() != 8) {
        THROW("Wrong number of arguments - hcal.updateEvent() must be called with 8 args.");
        return scope.Close(Undefined());
    }    
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || 
        args[3]->IsUndefined() || args[4]->IsUndefined() || args[5]->IsUndefined() || args[6]->IsUndefined()) {
        THROW("All arguments must be defined.");
        return scope.Close(Undefined());
    }
    if (args[7]->IsUndefined()){
        THROW("Updated event callback must be defined.");
    }

    Local<Function> cb = Local<Function>::Cast(args[7]);
    const unsigned argc = 1;
    Local<Value> argv[argc];

    try{
        int evt_id = args[0]->NumberValue();
        time_t start = NODE_V8_UNIXTIME(args[1]);
        time_t end = NODE_V8_UNIXTIME(args[2]);
        int room_id = args[3]->NumberValue();
        int leader_id = args[4]->NumberValue();
        std::string desc;
        if (!args[5]->IsUndefined()){
            String::Utf8Value temp_desc(args[5]->ToString());
            #ifdef __DEBUG__
            printf("v8 - desc string: %s\n", *temp_desc);
            #endif
            desc = std::string(*temp_desc);        
        }
        else{
            desc = std::string("No description");
        }
        bool recurring = args[6]->IsUndefined() ? false : args[6]->BooleanValue();

        hcal::DataLayer dl = hcal::DataLayer();
        hcal::DataLayer::UpdateStatus status = dl.update_event(evt_id, start, end, room_id, leader_id, desc, recurring);
        switch (status){
            case hcal::DataLayer::success:
                argv[0] = Local<Value>::New(Undefined());
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
            case hcal::DataLayer::failure:
                argv[0] = Local<Value>::New(String::New("hcal: Failed to update event."));
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
            case hcal::DataLayer::no_event:
                argv[0] = Local<Value>::New(String::New("hcal: no event with requested ID."));
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
        }
         
    }
    catch (std::exception& e){
        std::stringstream ss;
        ss << "hcal.updateEvent threw exception: " << e.what();
        std::string ex_str = ss.str();
        argv[0] = Local<Value>::New(String::New(ex_str.c_str()));
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(Undefined());
}

Handle<Value> DeleteEvent(const Arguments& args){
    HandleScope scope;
    if (args.Length() != 2){
        THROW("Wrong number of arguments = hcal.deleteEvent() must be called with 2 args.");
        return scope.Close(Undefined());
    }
    if (args[1]->IsUndefined()){
        THROW("Deleted event callback must be defined.");
    }

    Local<Function> cb = Local<Function>::Cast(args[1]);
    const unsigned argc = 1;
    Local<Value> argv[argc];
    
    try{
        int evt_id = args[0]->NumberValue();
        hcal::DataLayer dl = hcal::DataLayer();
        hcal::DataLayer::UpdateStatus status = dl.delete_event(evt_id);
        switch (status){
            case hcal::DataLayer::success:
                argv[0] = Local<Value>::New(Undefined());
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
            case hcal::DataLayer::failure:
                argv[0] = Local<Value>::New(String::New("hcal: Failed to update event."));
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
            case hcal::DataLayer::no_event:
                argv[0] = Local<Value>::New(String::New("hcal: no event with requested ID."));
                cb->Call(Context::GetCurrent()->Global(), argc, argv);
                break;
        }         
    }
    catch (std::exception& e){
        std::stringstream ss;
        ss << "hcal.deleteEvent threw exception: " << e.what();
        std::string ex_str = ss.str();
        argv[0] = Local<Value>::New(String::New(ex_str.c_str()));
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    return scope.Close(Undefined());
}

Handle<Value> GetEvents(const Arguments& args) {
    HandleScope scope;

    if (args.Length() != 4) {
        THROW("Wrong number of arguments - hcal.getEvents() must be called with 3 args.");
        return scope.Close(Undefined());
    }    
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || args[3]->IsUndefined()) {
        THROW("All arguments must be defined.");
        return scope.Close(Undefined());
    } 


    // Handle<Function> fn_fun = Handle<Function>::Cast(args[3]);
    // Persistent<Function> cb = Persistent<Function>::New(fn_fun);
    Local<Function> cb = Local<Function>::Cast(args[3]);
    const unsigned argc = 2;
    Local<Value> argv[argc];

    try{
        time_t start = args[0]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[0]);
        time_t end = args[1]->IsUndefined() ? 0 : NODE_V8_UNIXTIME(args[1]);
        int venue = args[2]->IsUndefined() ? 0 : args[2]->NumberValue();
        
        hcal::DataLayer dl = hcal::DataLayer();
        Handle<Array> evts = dl.get_wrapped_events(start, end, venue);

        argv[0] = Local<Value>::New(Undefined());
        argv[1] = Local<Value>::New(evts);
        std::cout << "v8 - calling getEvents callback." << std::endl;
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }
    catch (std::exception& e){
        std::stringstream ss;
        ss << "hcal.getEvents threw exception: " << e.what();
        std::string ex_str = ss.str();
        argv[0] = Local<Value>::New(String::New(ex_str.c_str()));
        argv[1] = Local<Value>::New(Undefined());
        cb->Call(Context::GetCurrent()->Global(), argc, argv);

        //cb->Call(context->Global(), argc, argv);
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
            retval = dl.get_wrapped_events(st, et, 0);            

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

Handle<Value> PrintCalendar(const Arguments& args) {
    HandleScope scope;

    if (args.Length() != 5) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }    
    if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || 
        args[3]->IsUndefined() || args[4]->IsUndefined()){
        THROW("All arguments must be defined.");
        return scope.Close(Undefined());
    }
    time_t start = NODE_V8_UNIXTIME(args[0]);
    time_t end =  NODE_V8_UNIXTIME(args[1]);
    String::AsciiValue viewStr(args[2]->ToString());
    int venue = args[3]->NumberValue();
    Local<Function> cb = Local<Function>::Cast(args[4]);
    const unsigned argc = 1;

    //TODO: need a fascade class here to hide implementation details (CalWriter)
    try{
        hcal::View v = hcal::get_view(viewStr);
        Local<Value> argv[argc];
        const char* fname;

        if (v == hcal::month){
            hcal::MonthWriter month_wtr(start, end, venue);
            fname = month_wtr.write_calendar();
            argv[0] = Local<Value>::New(String::New(fname));
            cb->Call(Context::GetCurrent()->Global(), argc, argv);
        }
        else if (v == hcal::week){
            hcal::WeekWriter week_wtr(start, end, venue);
            fname = week_wtr.write_calendar();
            argv[0] = Local<Value>::New(String::New(fname));
            cb->Call(Context::GetCurrent()->Global(), argc, argv);
        }      
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

    target->Set(String::NewSymbol("updateEvent"),
        FunctionTemplate::New(UpdateEvent)->GetFunction());

    target->Set(String::NewSymbol("deleteEvent"),
        FunctionTemplate::New(DeleteEvent)->GetFunction());

    target->Set(String::NewSymbol("getEvents"),
        FunctionTemplate::New(GetEvents)->GetFunction());

    target->Set(String::NewSymbol("createConfig"),
        FunctionTemplate::New(CreateConfig)->GetFunction());

    target->Set(String::NewSymbol("add"),
        FunctionTemplate::New(Add)->GetFunction());

    target->Set(String::NewSymbol("testEventArray"),
        FunctionTemplate::New(TestEventArray)->GetFunction());    

    target->Set(String::NewSymbol("printCalendar"),
      FunctionTemplate::New(PrintCalendar)->GetFunction());
}

NODE_MODULE(hcal, InitAll)