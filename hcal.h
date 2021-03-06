#ifndef HCAL_H
#define HCAL_H

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
#include "event.h"
#include "monthWriter.h"
#include "weekWriter.h"

v8::Handle<v8::Value> CreateEvent(const v8::Arguments&);

v8::Handle<v8::Value> CreateConfig(const v8::Arguments&);

v8::Handle<v8::Value> GetEvents(const v8::Arguments&);

v8::Handle<v8::Value> UpdateEvent(const v8::Arguments&);

v8::Handle<v8::Value> DeleteEvent(const v8::Arguments&);

v8::Handle<v8::Value> InsertEvent(const v8::Arguments&);

v8::Handle<v8::Value> Add(const v8::Arguments&);

v8::Handle<v8::Value> TestEventArray(const v8::Arguments&);

v8::Handle<v8::Value> PrintCalendar(const v8::Arguments&);

void InitAll(v8::Handle<v8::Object>);

void WriteException(v8::TryCatch&);

#endif