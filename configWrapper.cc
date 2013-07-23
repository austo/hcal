#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "configWrapper.h"

using namespace v8;

ConfigWrapper::ConfigWrapper() {};
ConfigWrapper::~ConfigWrapper() {};

Persistent<Function> ConfigWrapper::constructor;

void
ConfigWrapper::Init() {
  //Constructor function template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Config"));
  tpl->InstanceTemplate()->SetInternalFieldCount(4);
  
  //Prototype
  // tpl->PrototypeTemplate()->Set(String::NewSymbol("duration"),
  //     FunctionTemplate::New(Duration)->GetFunction());
  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value>
ConfigWrapper::New(const Arguments& args) {
  HandleScope scope;    
  ConfigWrapper* cfg = new ConfigWrapper();
  cfg->sday_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  cfg->eday_ = args[1]->IsUndefined() ? 0 : args[1]->NumberValue();
  cfg->shour_ = args[2]->IsUndefined() ? 0 : args[2]->NumberValue();
  cfg->ehour_ = args[3]->IsUndefined() ? 0 : args[3]->NumberValue();
   
  cfg->Wrap(args.This());
  return args.This();
}

Handle<Value>
ConfigWrapper::NewInstance(const Arguments& args) {
  HandleScope scope;
  if (args.Length() != 4) {
    ThrowException(
      Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  const unsigned argc = 4;
  Handle<Value> argv[argc] = { args[0], args[1], args[2], args[3] };
  Local<Object> instance = constructor->NewInstance(argc, argv);
  return scope.Close(instance);
}