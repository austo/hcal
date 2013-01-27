#ifndef GUARD__HCAL_H
#define GUARD__HCAL_H
#define BUILDING_NODE_EXTENSION

#ifndef BOOST_NO_EXCEPTIONS
#define BOOST_NO_EXCEPTIONS
#endif

#include <node.h>
#include <vector>
#include <list>
#include <map>
#include "event.h"

v8::Handle<v8::Value> CreateEvent(const v8::Arguments&);

v8::Handle<v8::Value> Add(const v8::Arguments&);

v8::Handle<v8::Value> TestEventArray(const v8::Arguments&);

v8::Handle<v8::Value> BuildCalendar(const v8::Arguments&); 

std::map< int, std::list<Event> >* GetEventMap(v8::Array* arr);

std::list<Event>* GetEventList(v8::Array*);

std::vector<Event>* GetEvents(v8::Array*);

void InitAll(v8::Handle<v8::Object>);

void WriteException(v8::TryCatch&);


#endif