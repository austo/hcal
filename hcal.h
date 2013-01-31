#ifndef GUARD__HCAL_H
#define GUARD__HCAL_H
#define BUILDING_NODE_EXTENSION

#include <node.h>

v8::Handle<v8::Value> CreateEvent(const v8::Arguments&);

v8::Handle<v8::Value> CreateConfig(const v8::Arguments&);

v8::Handle<v8::Value> Add(const v8::Arguments&);

v8::Handle<v8::Value> TestEventArray(const v8::Arguments&);

v8::Handle<v8::Value> BuildCalendar(const v8::Arguments&);

void InitAll(v8::Handle<v8::Object>);

void WriteException(v8::TryCatch&);

#endif