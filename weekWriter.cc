#include "weekWriter.h"

using namespace std;

namespace hcal {

    #define MARGIN 50
    #define ERR_CTOR_EMAP "WeekWriter: Constructor unable to create event map."
    #define ERR_WRITE_MONTHLY_EVENTS "WeekWriter: Error writing monthly events."
    #define ERR_UREC_VIEW "WeekWriter: Unrecognized calendar view."


    WeekWriter::WeekWriter(v8::Array* arr)
    {
        eventMap_ = get_evt_map(arr);
        if (eventMap_ == 0){
            throw runtime_error(ERR_CTOR_EMAP);
        }    
    }

    WeekWriter::WeekWriter()
    {
        eventMap_ = new map< int, list<Event> >();
    }

    WeekWriter::WeekWriter(time_t start, time_t end)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end);
    }

    WeekWriter::~WeekWriter()
    {
        if (eventMap_) delete eventMap_;
    }

    map< int, list<Event> >*
    WeekWriter::get_evt_map(v8::Array* arr)
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
                    index = evt.Start().date().week_number();
                    (*retval)[index].push_back(evt);
                }
            }           
        }
        return retval;
    }

    const char*
    WeekWriter::write_calendar()
    {
        const char* fname = "weekly_calendar.pdf";

        HPDF_Doc pdf = get_pdf();
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
        //ConfigWrapper cfg(0, 6, 8, 20);
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
                write_calendar_page(pdf, font, mitr->first);
            }          
        }    

        HPDF_SaveToFile(pdf, fname);
        HPDF_Free(pdf);
        return fname;    
    }


    void
    WeekWriter::write_calendar_page(HPDF_Doc pdf, HPDF_Font font, int weekOrdinal)
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
        ss << months()[(int)wkstart.month() - 1] << " " << wkstart.day() << " - " 
            << months()[(int)wkend.month() - 1] << " " << wkend.day();
        string tstring = ss.str();
        const char* page_title = tstring.c_str();

        write_page_title(page, font, page_title);

        //TODO: add configWrapper param
        write_weekday_cols(page, font, pageUsedWidth, pageUsedHeight, 7, MARGIN);

        /*
            TODO: this method should take a configWrapper obj
            If it's a regular week, we make a configWrapper in calling
            method and use it here
            (Only bother unpacking configWrapper obj if we've got a custom view)
        */
        double slotHeight;
        write_hour_rows(page, 8, 12, pageUsedWidth, pageUsedHeight, slotHeight);

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
    WeekWriter::write_hour_rows(HPDF_Page page, int startHour,
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

    void
    WeekWriter::write_events(HPDF_Page page, float cellHeight, float cellWidth,
        int monthOrdinal, int year, int firstDayNum, int rows, int daysInMonth )
    {
       //TODO: implement
    }    
}