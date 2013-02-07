#include <node.h>
#include <hpdf.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "eventWriter.h"
#include "eventWrapper.h"
#include "configWrapper.h"
#include "event.h"
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
using namespace std;

#define MARGIN 50
#define ERR_CTOR_EMAP "EventWriter: Constructor unable to create event map."
#define ERR_WRITE_MONTHLY_EVENTS "EventWriter: Error writing monthly events."
#define ERR_UREC_VIEW "EventWriter: Unrecognized calendar view."
#define ERR_CREATE_PDF "EventWriter: failed to create HPDF object."

void
error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    stringstream ss;
    ss << "ERROR: error_no = " << (HPDF_UINT)error_no << "\ndetail_no = " << (HPDF_UINT)detail_no << endl;    
    throw runtime_error(ss.str());
}

const char* EventWriter::weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* EventWriter::months[] = {"January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"};

EventWriter::EventWriter(v8::Array* arr, View v)
{
    view_ = v;
    eventMap_ = get_evt_map(arr);
    if (eventMap_ == 0){
        throw runtime_error(ERR_CTOR_EMAP);
    }    
}

EventWriter::EventWriter()
{
    eventMap_ = new map< int, list<Event> >();
}

EventWriter::EventWriter(map< int, list<Event> >* emap, View v)
{
    view_ = v;
    eventMap_ = emap;
}

EventWriter::~EventWriter()
{
    if (eventMap_) delete eventMap_;
}

/*  
    TODO: handle other views (function pointers - maybe not really necessary inside a class)
*/
map< int, list<Event> >*
EventWriter::get_evt_map(v8::Array* arr)
{
    map< int, list<Event> >* retVal = 0;
    uint32_t len = arr->Length();

    try{
        retVal = new map< int, list<Event> >();
    }
    catch(bad_alloc&){
        retVal = 0;
        cout << "error allocating memory..." << endl;
    }

    if (retVal){
        package_evt_map(retVal, arr, len);          
    }
    return retVal;
}

//TODO: handle case when events span multiple years
void
EventWriter::package_evt_map(map< int, list<Event> >* emap, v8::Array* arr, uint32_t len)
{
    int index = 0;
    for (uint32_t i = 0; i < len; ++i){
        v8::Local<v8::Object> obj = arr->CloneElementAt(i);
        if (!obj.IsEmpty()) {            
            EventWrapper* evtWrp = node::ObjectWrap::Unwrap<EventWrapper>(obj);
            Event evt = Event(evtWrp);
            switch(view_){
                case month:
                    index = (int)evt.Start().date().month().as_number();
                    break;
                case week:
                    index = evt.Start().date().week_number();
                    break;
                case day:
                    index = (int)evt.Start().date().day_of_year();
                    break;
                default:
                    break;
            }
            (*emap)[index].push_back(evt);
        }
    }
}

const char*
EventWriter::write_calendar()
{
    const char* not_implemented = "not done yet...";
    switch (view_){
        case month:
            return write_monthly_calendar();
        case week:
            return write_weekly_calendar();
        default:
            return not_implemented;
    }
}

/*  
    1. get start date of first event
    2. build calendar with correct view based on start date of first event
    3. (monthly calendar) using calculated dayCellHeight, place events in correct time-slot
        (five or six events per day - viewable)
*/
const char*
EventWriter::write_monthly_calendar()
{
    const char* fname = "monthly_calendar.pdf";

    HPDF_Doc pdf = get_pdf();
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);

    stringstream ss;    
    if (eventMap_ != NULL && eventMap_->size() > 0){
        map< int, list<Event> >::iterator mitr;
        for (mitr = eventMap_->begin(); mitr != eventMap_->end(); ++mitr){
            mitr->second.sort();
            boost::gregorian::date startDate(mitr->second.begin()->Start().date());
            int stYear = (int)startDate.year();
            write_monthly_calendar_page(pdf, font, mitr->first, stYear);
        }          
    }

    HPDF_SaveToFile(pdf, fname);
    HPDF_Free(pdf);
    return fname;
}

const char*
EventWriter::write_weekly_calendar()
{
    const char* fname = "weekly_calendar.pdf";

    HPDF_Doc pdf = get_pdf();
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    ConfigWrapper cfg(0, 6, 8, 20);
    /*
        1. get week start and end dates based on first event in mapped list
        2. build new week page
        loop
            a. is event in current week?
                i. yes - write event
                    (handle vertical offset of events on same day)
                    (handle horiziontal offset of event at same time - should there be a limit?)
                ii. no - build new week page
    */
    // stringstream ss;  

    if (eventMap_ != NULL && eventMap_->size() > 0){
        map< int, list<Event> >::iterator mitr;
        for (mitr = eventMap_->begin(); mitr != eventMap_->end(); ++mitr){
            mitr->second.sort();            
            write_weekly_calendar_page(pdf, font, mitr->first);
        }          
    }    

    HPDF_SaveToFile(pdf, fname);
    HPDF_Free(pdf);
    return fname;    
}

void
EventWriter::write_weekly_calendar_page(HPDF_Doc pdf, HPDF_Font font, int weekOrdinal)
{
    using namespace boost::gregorian;
    
    //add new letter-size page w/landscape orientation
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_LANDSCAPE);

    //page usable area
    int pageUsedWidth = HPDF_Page_GetWidth(page) - 100;
    int pageUsedHeight = HPDF_Page_GetHeight(page) - 125;

    list<Event>::const_iterator i = (*eventMap_)[weekOrdinal].begin();
    date startDate(i->Start().date());
    date_duration dd((int)startDate.day_of_week());
    date_duration wkdur(7);
    date wkstart = startDate - dd;
    date wkend = wkstart + wkdur;
    stringstream ss;
    ss << months[(int)wkstart.month() - 1] << " " << wkstart.day() << " - " 
        << months[(int)wkend.month() - 1] << " " << wkend.day();
    string tstring = ss.str();
    const char* page_title = tstring.c_str();

    write_page_title(page, font, page_title);

    //TODO: add configWrapper param
    write_weekday_cols(page, font, pageUsedWidth, pageUsedHeight);

    /*
        TODO: this method should take a configWrapper obj
        If it's a regular week, we make a configWrapper in calling
        method and use it here
        (Only bother unpacking configWrapper obj if we've got a custom view)
    */
    double slotHeight;
    write_weekly_hour_rows(page, 8, 12, pageUsedWidth, pageUsedHeight, slotHeight);

    float dayWidth = ((float)pageUsedWidth / 7);
    int evtMargin = MARGIN + 5;
    date currentDate(startDate);

    for (; i != (*eventMap_)[weekOrdinal].end(); ++i){
        int dayNum = (int)i->Start().date().day_of_week();
        float x_offset = (dayWidth * dayNum) + evtMargin;
        /*
            color in the slots based on room
            need a reliable way of determing slot based on event time
                using:
                    number of hours on calendar
                    first hour
                    may need a configuratuion object
        */
    }  

}

void
EventWriter::write_monthly_calendar_page(HPDF_Doc pdf, HPDF_Font font, int month, int year)
{
    using namespace boost::gregorian;

    stringstream titleStream;
    titleStream << months[month - 1] << " " << year;
    string tstring = titleStream.str();
    const char* page_title = tstring.c_str();

    //add new letter-size page w/landscape orientation
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_LANDSCAPE);

    //page usable area
    int pageUsedWidth = HPDF_Page_GetWidth(page) - 100;
    int pageUsedHeight = HPDF_Page_GetHeight(page) - 125;

    write_page_title(page, font, page_title);
    write_weekday_cols(page, font, pageUsedWidth, pageUsedHeight);    

    //Use the calendar to get the last day of the month
    int eomDay = gregorian_calendar::end_of_month_day(year, month);
    date endOfMonth(year, month, eomDay);

    //construct an iterator starting with first day of the month
    day_iterator ditr(date(year, month, 1));
    int firstDayNum = (int)ditr->day_of_week();
    int firstWeekDays = 8 - firstDayNum;
    int remainingDays = eomDay - firstWeekDays + 1; 
    int rowNum = remainingDays % 7 == 0 ? (remainingDays / 7) + 1 : (remainingDays / 7) + 2;
    int currentRowNum = rowNum;
    float dayHeight = ((float)pageUsedHeight / rowNum);
    float dayWidth = ((float)pageUsedWidth / 7);

    //loop days & print each one in column at correct height
    for (; ditr <= endOfMonth; ++ditr) {
        int dayNum = (int)ditr->day_of_week();
        unsigned short dateNum = ditr->day().as_number();
        if (dateNum > 1){
            if ((dateNum - firstWeekDays) % 7 == 0){
                currentRowNum -= 1;
            }
        }
        stringstream ss;
        ss << dateNum;
        string dstring = ss.str();
        const char* dateNumChar = dstring.c_str();

        float tw = HPDF_Page_TextWidth(page, dateNumChar);
        float x_dayOffset = (dayWidth * dayNum) + ((dayWidth + MARGIN) - (tw + 2));
        float y_dayOffset = 40 + (dayHeight * currentRowNum);
        write_text(page, x_dayOffset, y_dayOffset, dateNumChar);        
    }

    //horizontal lines
    for (int i = 1; i < rowNum; i++)
    {
        float line_y = MARGIN + (dayHeight * i);
        draw_line(page, MARGIN, line_y, MARGIN + pageUsedWidth, line_y);        
    }
    
    HPDF_Page_SetFontAndSize(page, font, 8);
    write_events(page, dayHeight, dayWidth, month, year, firstDayNum, rowNum, (int)endOfMonth.day().as_number());
    #ifdef __DEBUG__
    cout << "v8 - after writing events" << endl;
    #endif  
}


void
EventWriter::write_events(HPDF_Page page, float cellHeight, float cellWidth,
    int monthOrdinal, int year, int firstDayNum, int rows, int daysInMonth )
{
    vector<int> rowArray = build_row_array(rows, firstDayNum, daysInMonth);
    list<Event>::const_iterator i;
    int evtMargin = MARGIN + 5;
    float evtHeight = (cellHeight - 15) / 5;
    #ifdef __DEBUG__
    int c = 1;
    #endif
    try{
        int dailyEvtCount = 0, currentDateNum = 0;
        for (i = (*eventMap_)[monthOrdinal].begin(); i != (*eventMap_)[monthOrdinal].end(); ++i){
            #ifdef __DEBUG__
            cout << "write event loop iteration " << c << endl;
            c += 1;
            #endif
            boost::gregorian::date evtDate(i->Start().date());

            //Only add current year's events to calendar
            if ((int)evtDate.year() != year){
                continue;
            }
            int dayNum = (int)evtDate.day_of_week();
            int dateNum = (int)evtDate.day().as_number();
            int rowNum = get_day_row(&rowArray, dateNum);
            if (dateNum == currentDateNum){
                dailyEvtCount += 1;
            }
            else{
                dailyEvtCount = 0;
                currentDateNum = dateNum;
            }

            //Write start and event title in cellHeight/rows
            float x_offset = (cellWidth * dayNum) + evtMargin;
            float y_offset = ((cellHeight * rowNum) + MARGIN) - (15 + (dailyEvtCount * evtHeight));

            boost::posix_time::time_duration stdur = i->Start().time_of_day();
            long hours = stdur.hours();
            string meridian = "am";
            if (hours > 12){
                hours -= 12;
                meridian = "pm";
            }
            else if(hours == 12){
                meridian = "pm";
            }         

            stringstream ss;
            if (stdur.minutes() == 0){
                ss << hours <<  ":00 " << meridian << " - " << i->Title();
            }
            else{
                ss << hours << ":" << stdur.minutes() << " " << meridian << " - " << i->Title();
            }
            string tstring = ss.str();
            const char* evtTitle = tstring.c_str();
            #ifdef __DEBUG__
            cout << "v8 - event title string: " << evtTitle << endl;
            #endif
            write_text(page, x_offset, y_offset, evtTitle);        
        }
    }
    catch(exception& e){
        cout << "  Exception: " << e.what() << endl;
        throw runtime_error(ERR_WRITE_MONTHLY_EVENTS);
    }
    #ifdef __DEBUG__
    cout << "v8 - after write events loop\n";
    #endif
    // HPDF_Page_SetLineWidth(page, 12);
    // HPDF_Page_SetRGBStroke (page, 0.0, 0.5, 0.0);

    /* Line Cap Style */
    // HPDF_Page_SetLineCap (page, HPDF_BUTT_END);
    //draw_line2 (page, 60, 570, "PDF_BUTT_END");

    // HPDF_Page_SetLineCap (page, HPDF_ROUND_END);
    //draw_line2 (page, 60, 505, "PDF_ROUND_END");
    //delete rowArray;
}

vector<int>
EventWriter::build_row_array(int rows, int firstDayNum, int daysInMonth)
{
    int ldfw = 7 - firstDayNum;
    vector<int> retVal = vector<int>();

    for (int i = 0; i < rows; ++i) {
        retVal.push_back(ldfw + (i * 7));
    }
    return retVal;
}

int
EventWriter::get_day_row(vector<int>* rowArray, int dayNum)
{
    int rows = rowArray->size();

    //Event is on first row
    if (dayNum <= (*rowArray)[0]){
        return rows;
    }

    for (int i = 0; i < rows; i++) {
        if (dayNum > (*rowArray)[i] && dayNum <= (*rowArray)[i + 1]){
            return rows - (i + 1);
        }
    }
    return 0;
}

void
EventWriter::write_text(HPDF_Page page, float x_offset, float y_offset, const char* text)
{
    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, x_offset, y_offset);
    HPDF_Page_ShowText (page, text);
    HPDF_Page_EndText (page);
}

void
EventWriter::write_page_title(HPDF_Page page, HPDF_Font font, const char* page_title)
{
    /* print the title of the page (with positioning center). */
    HPDF_Page_SetFontAndSize(page, font, 20);
    float tw = HPDF_Page_TextWidth(page, page_title);
    write_text(page, (HPDF_Page_GetWidth(page) - tw) / 2, HPDF_Page_GetHeight(page) - 40, page_title);
    HPDF_Page_SetFontAndSize(page, font, 10);
    HPDF_Page_SetLineWidth(page, 1);
}

void
EventWriter::draw_line(HPDF_Page page, float x_start, float y_start, float x_end, float y_end)
{
    HPDF_Page_MoveTo (page, x_start, y_start);
    HPDF_Page_LineTo (page, x_end, y_end);
    HPDF_Page_Stroke (page);
}

EventWriter::View
EventWriter::get_view(v8::String::AsciiValue& viewStr)
{
    if (strcmp(*viewStr, "month") == 0){
        return EventWriter::month;
    }
    else if (strcmp(*viewStr, "week") == 0){
        return EventWriter::week;
    }
    else if (strcmp(*viewStr, "day") == 0){
        return EventWriter::day;
    }
    else{
        throw runtime_error(ERR_UREC_VIEW);
    }
}


HPDF_Doc
EventWriter::get_pdf()
{
    HPDF_Doc pdf = 0;
    try{
        pdf = HPDF_New(error_handler, NULL);
        return pdf;
    }
    catch(exception& e){
        if (pdf){
            HPDF_Free(pdf);
        }
        throw runtime_error(e.what());
    }
    if (!pdf) {
        throw runtime_error(ERR_CREATE_PDF);        
    }
}


void
EventWriter::write_weekday_cols(HPDF_Page page, HPDF_Font font, int pageUsedWidth, int pageUsedHeight)
{
    HPDF_Page_SetLineWidth(page, 1);
    HPDF_Page_Rectangle(page, MARGIN, MARGIN, pageUsedWidth, pageUsedHeight);
    HPDF_Page_Stroke(page);

    float dayWidth = ((float)pageUsedWidth / 7);

    //vertical lines
    int i;
    for (i = 1; i < 8; i++)
    {
        float line_x = MARGIN + (dayWidth * i);
        draw_line(page, line_x, MARGIN, line_x, MARGIN + pageUsedHeight);        
    }

    //weekdays
    float dayMargin = MARGIN + (dayWidth / 2);
    for (i = 0; i < 7; i++){
        float tw = HPDF_Page_TextWidth(page, weekdays[i]);
        float textpos = (dayMargin - (tw /2)) + (dayWidth * i);
        write_text(page, textpos, HPDF_Page_GetHeight(page) - (MARGIN + 15), weekdays[i]);        
    }
}

void
EventWriter::write_weekly_hour_rows(HPDF_Page page, int startHour,
    int numHours, int pageUsedWidth, int pageUsedHeight, double& slotHeight)
{
    int numLines = numHours * 2;
    slotHeight = (double)pageUsedHeight / (double)numLines;
    float line_y = MARGIN + slotHeight;

    HPDF_Page_SetLineWidth(page, .15);

    for (int i = 0, hrln = 1; i < numLines; ++i){
        if (i == hrln){
            HPDF_Page_SetLineWidth(page, .6);
            draw_line(page, MARGIN, line_y, MARGIN + pageUsedWidth, line_y);
            HPDF_Page_SetLineWidth(page, .15);
            hrln += 2;
        }
        else{
           draw_line(page, MARGIN, line_y, MARGIN + pageUsedWidth, line_y); 
        }

        line_y += slotHeight;
    }
}