#ifndef GUARD__HCAL_H
#define GUARD__HCAL_H
#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <iostream>
#include <new>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <map>
#include <hpdf.h>
#include <exception>
#include "throw_exception.hpp"
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
#include "dataLayer.h"
#include "eventWrapper.h"
#include "configWrapper.h"
#include "hcal.h"
#include "eventWriter.h"
#include "event.h"

v8::Handle<v8::Value> CreateEvent(const v8::Arguments&);

v8::Handle<v8::Value> CreateConfig(const v8::Arguments&);

v8::Handle<v8::Value> Add(const v8::Arguments&);

v8::Handle<v8::Value> TestEventArray(const v8::Arguments&);

v8::Handle<v8::Value> BuildCalendar(const v8::Arguments&);

v8::Handle<v8::Value> PrintCalendar(const v8::Arguments&);

void InitAll(v8::Handle<v8::Object>);

void WriteException(v8::TryCatch&);

#endif