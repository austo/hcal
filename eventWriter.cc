#include <node.h>
#include <hpdf.h>
#include <setjmp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <sstream>
#include <iostream>
#include "eventWriter.h"
#include "eventWrapper.h"
#include "event.h"
#include "posix_time/posix_time.hpp"
#include "gregorian/gregorian.hpp"
using namespace std;

#define MARGIN 50

jmp_buf env;

void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

const char* EventWriter::weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* EventWriter::months[] = {"January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"};

EventWriter::EventWriter(v8::Array* arr, View v){
    view_ = v;
    eventMap_ = GetEventMap(arr);
}

EventWriter::EventWriter()
{
    eventMap_ = new map< int, list<Event> >();
}

//TODO: come up with a better way of checking for a non null events_ vector
EventWriter::~EventWriter()
{
    delete eventMap_;
}

map< int, list<Event> >* EventWriter::GetEventMap(v8::Array* arr){
    try{
        uint32_t len = arr->Length();
        map< int, list<Event> >* retVal = new map< int, list<Event> >();

        //TODO: handle other views (function pointers)
        for (uint32_t i = 0; i < len; ++i){
            v8::Local<v8::Object> obj = arr->CloneElementAt(i);
            if (!obj.IsEmpty()) {            
                EventWrapper* evtWrp = node::ObjectWrap::Unwrap<EventWrapper>(obj);
                Event evt = Event(evtWrp);
                int monthNo = (int)evt.Start().date().month().as_number();
                (*retVal)[monthNo].push_back(evt);
            }
        }
        return retVal;
    }
    catch(bad_alloc&){
        cout << "error allocating memory..." << endl;
    }
    catch (...){
        cout << "other exception...";
    }    
}


const char* EventWriter::write_calendar(){
    const char* not_implemented = "not done yet...";
    switch (view_){
        case month:
            return write_monthly_calendar();
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
const char* EventWriter::write_monthly_calendar()
{
    const char* fname = "test_calendar.pdf";
    const char* problem = "there was a problem...";
    stringstream ss;//create a stringstream     
    if (eventMap_ != NULL && eventMap_->size() > 0){
        map< int, list<Event> >::iterator m_itr;
        for (m_itr = eventMap_->begin(); m_itr != eventMap_->end(); ++m_itr){
            m_itr->second.sort();
            boost::gregorian::date startDate(m_itr->second.begin()->Start().date());
            int stYear = (int)startDate.year();
            const char* it_val = write_monthly_calendar(m_itr->first, stYear);
            ss << it_val << "-";
        }
        string retString = ss.str();
        return retString.c_str();        
    }      
    
    return problem;
}

const char* EventWriter::write_monthly_calendar(int month, int year)
{
    using namespace boost::gregorian;

    stringstream titleStream;//create a stringstream
    titleStream << months[month - 1] << " " << year;//add number to the stream
    string tstring = titleStream.str();
    const char* page_title = tstring.c_str();
    //const char* page_title = "Calendar Test";
    const char* fname = "test_calendar.pdf"; 

    HPDF_Doc pdf;
    HPDF_Font font;
    HPDF_Page page;
    float tw;
    int rowNum;

    pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return "failed";
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return "failed";
    }

    /* create default font */
    font = HPDF_GetFont (pdf, "Helvetica", NULL);

    /* add new page object w/ A4 size and landscape orientation */
    page = HPDF_AddPage (pdf);
    HPDF_Page_SetSize (page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_LANDSCAPE);

    //page usable area
    int pageUsedWidth = HPDF_Page_GetWidth(page) - 100;
    int pageUsedHeight = HPDF_Page_GetHeight (page) - 125;

    HPDF_Page_SetLineWidth (page, 1);
    HPDF_Page_Rectangle (page, MARGIN, MARGIN, pageUsedWidth, pageUsedHeight);
    HPDF_Page_Stroke (page);

    /* print the title of the page (with positioning center). */
    HPDF_Page_SetFontAndSize (page, font, 20);
    tw = HPDF_Page_TextWidth (page, page_title);
    write_text(page, (HPDF_Page_GetWidth(page) - tw) / 2, HPDF_Page_GetHeight (page) - 40, page_title);
    HPDF_Page_SetFontAndSize (page, font, 10);
    HPDF_Page_SetLineWidth (page, 1);

    //TODO: refactor into class and methods 
    float dayWidth = ((float)pageUsedWidth / 7);

    //vertical lines
    int i;
    for (i = 1; i < 8; i++)
    {
        float line_x = MARGIN + (dayWidth * i);
        HPDF_Page_MoveTo (page, line_x, MARGIN);
        HPDF_Page_LineTo (page, line_x, MARGIN + pageUsedHeight);
        HPDF_Page_Stroke (page);
    }

    //weekdays
    float dayMargin = MARGIN + (dayWidth / 2);
    for (i = 0; i < 7; i++){
        tw = HPDF_Page_TextWidth (page, weekdays[i]);
        float textpos = (dayMargin - (tw /2)) + (dayWidth * i);
        write_text(page, textpos, HPDF_Page_GetHeight (page) - (MARGIN + 15), weekdays[i]);        
    }

    //Use the calendar to get the last day of the month
    int eomDay = gregorian_calendar::end_of_month_day(year, month);
    date endOfMonth(year, month, eomDay);

    //construct an iterator starting with first day of the month
    day_iterator ditr(date(year, month, 1));
    int firstDayNum = (int)ditr->day_of_week();
    int firstWeekDays = 8 - firstDayNum;
    int remainingDays = eomDay - firstWeekDays + 1; 
    rowNum = remainingDays % 7 == 0 ? (remainingDays / 7) + 1 : (remainingDays / 7) + 2;
    int currentRowNum = rowNum;
    float dayHeight = ((float)pageUsedHeight / rowNum);
    
    //loop days & print each one in calendar box
    for (; ditr <= endOfMonth; ++ditr) {
        int dayNum = (int)ditr->day_of_week();
        unsigned short dateNum = ditr->day().as_number();
        if (dateNum > 1){
            if ((dateNum - firstWeekDays) % 7 == 0){
                currentRowNum -= 1;
            }
        }
        stringstream ss;//create a stringstream
        ss << dateNum;//add number to the stream
        string dstring = ss.str();
        const char* dateNumChar = dstring.c_str();

        tw = HPDF_Page_TextWidth(page, dateNumChar);
        float x_dayOffset = (dayWidth * dayNum) + ((dayWidth + MARGIN) - (tw + 2));
        float y_dayOffset = 40 + (dayHeight * currentRowNum);
        write_text(page, x_dayOffset, y_dayOffset, dateNumChar);        
    }

    //horizontal lines
    for (i = 1; i < rowNum; i++)
    {
        float line_y = MARGIN + (dayHeight * i);
        HPDF_Page_MoveTo (page, MARGIN, line_y);
        HPDF_Page_LineTo (page, MARGIN + pageUsedWidth, line_y);
        HPDF_Page_Stroke (page);
    }
    
    HPDF_Page_SetFontAndSize (page, font, 8);
    write_events(page, dayHeight, dayWidth, month, year, firstDayNum, rowNum, (int)endOfMonth.day().as_number());
    cout << "after writing events" << endl;
    /* save the document to a file */
    HPDF_SaveToFile (pdf, fname);

    /* clean up */
    HPDF_Free (pdf);

    return fname;
}


void EventWriter::write_events( HPDF_Page page,
                                float cellHeight,
                                float cellWidth,
                                int monthOrdinal,
                                int year,
                                int firstDayNum,
                                int rows,
                                int daysInMonth )
{

    vector<int> rowArray = build_row_array(rows, firstDayNum, daysInMonth);
    list<Event>::const_iterator i;
    int evtMargin = MARGIN + 5;
    float evtHeight = (cellHeight - 15) / 5;
    int c = 1;
    try{
        int dailyEvtCount = 0, currentDateNum = 0;
        for (i = (*eventMap_)[monthOrdinal].begin(); i != (*eventMap_)[monthOrdinal].end(); ++i){
            cout << "write event loop iteration " << c << endl;
            c += 1;
            boost::gregorian::date evtDate(i->Start().date());

            //Only add current month's events to calendar
            if ((int)evtDate.month().as_number() != monthOrdinal || (int)evtDate.year() != year){
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

            stringstream ss;
            if (stdur.minutes() == 0){
                ss << hours <<  ":00 " << meridian << " - " << i->Title();
            }
            else{
                ss << hours << ":" << stdur.minutes() << " " << meridian << " - " << i->Title();
            }
            string tstring = ss.str();
            const char* evtTitle = tstring.c_str();
            cout << "v8 - event title string: " << evtTitle << endl;
            write_text(page, x_offset, y_offset, evtTitle);        
        }
    }
    catch(exception& e){
        cout << "  Exception: " << e.what() << endl;
    }
    cout << "after write events loop\n";
    // HPDF_Page_SetLineWidth(page, 12);
    // HPDF_Page_SetRGBStroke (page, 0.0, 0.5, 0.0);

    /* Line Cap Style */
    // HPDF_Page_SetLineCap (page, HPDF_BUTT_END);
    //draw_line2 (page, 60, 570, "PDF_BUTT_END");

    // HPDF_Page_SetLineCap (page, HPDF_ROUND_END);
    //draw_line2 (page, 60, 505, "PDF_ROUND_END");
    //delete rowArray;
}

vector<int> EventWriter::build_row_array(int rows, int firstDayNum, int daysInMonth)
{
    int ldfw = 7 - firstDayNum;
    vector<int> retVal = vector<int>();

    for (int i = 0; i < rows; ++i) {
        retVal.push_back(ldfw + (i * 7));
    }
    return retVal;
}

int EventWriter::get_day_row(vector<int>* rowArray, int dayNum)
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

void EventWriter::write_text(HPDF_Page page, float x_offset, float y_offset, const char* text)
{
    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, x_offset, y_offset);
    HPDF_Page_ShowText (page, text);
    HPDF_Page_EndText (page);
}