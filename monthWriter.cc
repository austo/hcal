// #define __DEBUG__
#include "monthWriter.h"

using namespace std;

namespace hcal {

    #define MARGIN 50
    #define ERR_CTOR_EMAP "MonthWriter: Constructor unable to create event map."
    #define ERR_WRITE_MONTHLY_EVENTS "MonthWriter: Error writing monthly events."
    #define ERR_UREC_VIEW "MonthWriter: Unrecognized calendar view."


    MonthWriter::MonthWriter(v8::Array* arr)
    {
        eventMap_ = get_evt_map(arr);
        if (eventMap_ == 0){
            throw runtime_error(ERR_CTOR_EMAP);
        }    
    }

    MonthWriter::MonthWriter()
    {
        eventMap_ = new map< int, list<Event> >();
    }

    MonthWriter::MonthWriter(time_t start, time_t end)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end, month);
    }

    MonthWriter::~MonthWriter()
    {
        if (eventMap_) delete eventMap_;
    }

    map< int, list<Event> >*
    MonthWriter::get_evt_map(v8::Array* arr)
    {        
        map< int, list<Event> >* retval = 0;
        uint32_t len = arr->Length();

        try{
            retval = new map< int, list<Event> >();
        }
        catch(bad_alloc&){
            retval = 0;
            cout << "error allocating memory..." << endl;
        }

        if (retval){
           int index = 0;
            for (uint32_t i = 0; i < len; ++i){
                v8::Local<v8::Object> obj = arr->CloneElementAt(i);
                if (!obj.IsEmpty()) {            
                    EventWrapper* evtWrp = node::ObjectWrap::Unwrap<EventWrapper>(obj);
                    Event evt = Event(evtWrp);
                    index = (int)evt.Start().date().month().as_number();                  
                    (*retval)[index].push_back(evt);
                }
            }           
        }
        return retval;
    }

    const char*
    MonthWriter::write_calendar()
    {
        const char* fname = "monthly_calendar_poly.pdf";

        HPDF_Doc pdf = get_pdf();
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);

        stringstream ss;    
        if (eventMap_ != NULL && eventMap_->size() > 0){
            map< int, list<Event> >::iterator mitr;
            for (mitr = eventMap_->begin(); mitr != eventMap_->end(); ++mitr){
                mitr->second.sort();
                boost::gregorian::date startDate(mitr->second.begin()->Start().date());
                int stYear = (int)startDate.year();
                write_calendar_page(pdf, font, mitr->first, stYear);
            }          
        }

        HPDF_SaveToFile(pdf, fname);
        HPDF_Free(pdf);
        return fname;
    }


    void
    MonthWriter::write_calendar_page(HPDF_Doc pdf, HPDF_Font font, int month, int year)
    {
        using namespace boost::gregorian;

        stringstream titleStream;
        titleStream << months()[month - 1] << " " << year;
        string tstring = titleStream.str();
        const char* page_title = tstring.c_str();

        //add new letter-size page w/landscape orientation
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_LANDSCAPE);

        //page usable area
        int pageUsedWidth = HPDF_Page_GetWidth(page) - 100;
        int pageUsedHeight = HPDF_Page_GetHeight(page) - 125;

        write_page_title(page, font, page_title);
        write_weekday_cols(page, font, pageUsedWidth, pageUsedHeight, 7, MARGIN);    

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
    MonthWriter::write_events(HPDF_Page page, float cellHeight, float cellWidth,
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
            int currentDateNum, more_evts = 0;
            float x_offset, y_offset;
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
                    float lower_bound = rowNum > 0 ? (cellHeight * (rowNum - 1)) : 0 ;
                    if (y_offset <= (lower_bound + MARGIN + evtHeight)){
                        more_evts += 1;
                        continue;
                    }
                    y_offset -= 5;
                }
                else{
                    write_if_more_evts(more_evts, page, x_offset, y_offset);
                    currentDateNum = dateNum;
                    more_evts = 0;
                    y_offset = ((cellHeight * rowNum) + MARGIN) - 18;
                    x_offset = (cellWidth * dayNum) + evtMargin;
                }                  
                
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
                    ss << hours <<  ":00 " << meridian << " - " << i->Description();
                }
                else{
                    ss << hours << ":" << stdur.minutes() << " " << meridian << " - " << i->Description();
                }
                #ifdef __DEBUG__
                cout << "v8 - event title string: " << evtTitle << endl;
                #endif                
                write_wrapped_text(page, x_offset, y_offset, ss.str(), evtHeight- 5, cellWidth - 5);                
            }
            write_if_more_evts(more_evts, page, x_offset, y_offset);
        }
        catch(exception& e){
            cout << "  Exception: " << e.what() << endl;
            throw runtime_error(ERR_WRITE_MONTHLY_EVENTS);
        }
        #ifdef __DEBUG__
        cout << "v8 - after write events loop\n";
        #endif    
    }

    vector<int>
    MonthWriter::build_row_array(int rows, int firstDayNum, int daysInMonth)
    {
        int ldfw = 7 - firstDayNum;
        vector<int> retVal = vector<int>();

        for (int i = 0; i < rows; ++i) {
            retVal.push_back(ldfw + (i * 7));
        }
        return retVal;
    }

    int
    MonthWriter::get_day_row(vector<int>* rowArray, int dayNum)
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
}