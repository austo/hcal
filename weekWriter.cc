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
        start_hour_ = 8;
        end_hour_ = 20;
        slot_height_ = 0;
        slot_width_ = 0;
        pg_used_width_ = 0;
        pg_used_height_ = 0; 
        venue_ = 0;
    }

    WeekWriter::WeekWriter()
    {
        eventMap_ = new map< int, list<Event> >();
        start_hour_ = 8;
        end_hour_ = 20;
        slot_height_ = 0;
        slot_width_ = 0;
        pg_used_width_ = 0;
        pg_used_height_ = 0;  
        venue_ = 0;     
    }

    WeekWriter::WeekWriter(time_t start, time_t end, int venue)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end, week, venue);
        rooms_ = dl.get_rooms();
        start_hour_ = 8;
        end_hour_ = 20;
        slot_height_ = 0;
        slot_width_ = 0;
        pg_used_width_ = 0;
        pg_used_height_ = 0;
        venue_ = venue; 
    }

    WeekWriter::WeekWriter(time_t start, time_t end, int start_hour, int end_hour, int venue)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end, week, venue);
        rooms_ = dl.get_rooms();
        start_hour_ = start_hour;
        end_hour_ = end_hour;
        slot_height_ = 0;
        slot_width_ = 0;
        pg_used_width_ = 0;
        pg_used_height_ = 0;
        venue_ = venue;
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
        const char* fname = "weekly_calendar_poly.pdf";

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
    WeekWriter::write_calendar_page(HPDF_Doc pdf, HPDF_Font font, int week_ordinal)
    {
        using namespace boost::gregorian;
        
        //add new letter-size page w/landscape orientation
        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_LANDSCAPE);

        //page usable area
        pg_used_width_ = pg_used_width_ ? pg_used_width_ : HPDF_Page_GetWidth(page) - (2 * MARGIN);
        pg_used_height_ = pg_used_height_ ? pg_used_height_ : HPDF_Page_GetHeight(page) - (2 * MARGIN + (MARGIN / 2));

        date start_date((*eventMap_)[week_ordinal].begin()->Start().date());
        date_duration dd((int)start_date.day_of_week());
        date_duration wkdur(6);
        date wk_start = start_date - dd;
        date wk_end = wk_start + wkdur;
        stringstream ss;
        ss << months()[(int)wk_start.month() - 1] << " " << wk_start.day() << " - " 
            << months()[(int)wk_end.month() - 1] << " " << wk_end.day();
        string tstring = ss.str();
        const char* page_title = tstring.c_str();

        write_page_title(page, font, page_title);

        //TODO: add configWrapper param
        write_weekday_cols(page, font, pg_used_width_, pg_used_height_, 7, MARGIN);

        /*
            TODO: this method should take a configWrapper obj
            If it's a regular week, we make a configWrapper in calling
            method and use it here
            (Only bother unpacking configWrapper obj if we've got a custom view)

            NOTE: should get start_hour and num_hours earlier in lifecycle & make slot_height a member variable
            NOTE: CustomWriter will inherit from WeekWriter (or vice versa?)
        */
        write_hour_rows(page, font);

        slot_width_ = slot_width_ ? slot_width_ : ((double)pg_used_width_ / 7);
        int evtMargin = MARGIN + 5;

        write_events(page, font, week_ordinal);
    }

    void
    WeekWriter::write_hour_rows(HPDF_Page page, HPDF_Font font)
    {
        int num_lines = (end_hour_ - start_hour_) * 2;

        //initialize slot_height_ if we haven't already
        slot_height_ = slot_height_ ? slot_height_ : (double)pg_used_height_ / (double)num_lines;        
        float line_y = MARGIN + slot_height_;

        HPDF_Page_SetLineWidth(page, .10);        
        HPDF_Page_SetFontAndSize(page, font, 8);
        double hr_num_x = MARGIN - 27;
        char buf[6];
        //write last number at lower margin
        write_hour_to_buf(buf, end_hour_, true);
        write_text(page, hr_num_x, MARGIN, buf);

        for (int i = 0, hrln = 1, disp_hour = end_hour_ - 1; i < num_lines; ++i){
            if (i == hrln){
                HPDF_Page_SetLineWidth(page, .6);

                bool write_meridian = (disp_hour == start_hour_ || disp_hour == 12);
                write_hour_to_buf(buf, disp_hour, write_meridian);
                write_text(page, hr_num_x, line_y - 2, buf);
                --disp_hour;

                draw_line(page, MARGIN, line_y, MARGIN + pg_used_width_, line_y);
                HPDF_Page_SetLineWidth(page, .15);
                hrln += 2;
            }
            else{
               draw_line(page, MARGIN, line_y, MARGIN + pg_used_width_, line_y); 
            }

            line_y += slot_height_;
        }
    }

    void WeekWriter::write_room_color_key(HPDF_Page page, HPDF_Font font){
        /*
            only use colors on page?
            what happens if there are 20 rooms - multiple rows?
            determine how wide key will be (text width plus box offset width)
            draw box and fill with room color
        */
    }

    void
    WeekWriter::write_events(HPDF_Page page, HPDF_Font font, int week_ordinal)
    {
        int num_conc_evts = 0, conc_evt_instance;
        HPDF_Page_SetLineWidth(page, 1);
        HPDF_Page_SetFontAndSize(page, font, 5); //TODO: make dependent on number of events
        list<Event>::const_iterator evt_itr = (*eventMap_)[week_ordinal].begin();
        for (; evt_itr != (*eventMap_)[week_ordinal].end(); ++evt_itr){
            cout << "v8 - evt title: " << evt_itr->Description() << "; start time: " << evt_itr->Start() << endl;
            //decrement events if not zero
            if (!num_conc_evts){
                num_conc_evts = get_overlapping_events(evt_itr);
                cout << "v8 - num conc events: " << num_conc_evts << endl;
                conc_evt_instance = 0;
            }            
            Event_Rect evt_rect = get_event_rect(evt_itr, num_conc_evts, conc_evt_instance);

            //TODO: this should either be a class member or be called from within a utility function
            draw_event_rect(page, evt_rect);

            // HPDF_INT asc = HPDF_Font_GetAscent(font);
            // HPDF_INT dsc = HPDF_Font_GetDescent(font);
            float ln_height = 5, avail_width = evt_rect.l_right.x - evt_rect.l_left.x,
                x_offset = (float)(evt_rect.l_left.x + 1), y_offset = (float)(evt_rect.l_left.y - 5);

            HPDF_Page_SetRGBFill(page, 0, 0, 0);
            write_wrapped_text(page, x_offset, y_offset, evt_itr->Description(), ln_height, avail_width);

            if (conc_evt_instance == num_conc_evts + 1){
                num_conc_evts = 0;
            }
        }
        /*
            TODO: implement
            1. what day and slot is event in?
            2. determine if event should be full width (or how wide if not)
            3. color event slot
            4. write event title (w/ wrapped text) according to alloted width
        */
    }

    /*
        TODO:
            Handle cases where event overflows available hours and/or event spans multiple days;
            If event is not within the bounds of the day, don't print it;

            Change member function name to print_event.
            Need a function int get_overlapping_events(list<Event>::const_iterator& evt_itr)
    */
    Event_Rect
    WeekWriter::get_event_rect(list<Event>::const_iterator& evt_itr, const int& num_conc_evts, int& conc_evt_instance){
        using namespace boost::posix_time;

        //TODO: test out of bounds here or in another method
        //TODO: handle multi-day events so as not to force narrow all other events - add multi-day flag to Event?

        time_duration s_dur(evt_itr->Start().time_of_day()), e_dur(evt_itr->End().time_of_day());

        double start_y = 0;
        if (s_dur.hours() < start_hour_){
            if (e_dur.hours() < start_hour_){ 
                return Event_Rect(); 
            }
            //start hour is invisible, change coordinates top of grid
            start_y = MARGIN + (double)pg_used_height_;
        }
        bool end_at_margin = false;
        if (e_dur.hours() >= end_hour_ || e_dur.hours() <= start_hour_){
            if(s_dur.hours() > end_hour_ ){
                return Event_Rect();
            }
            //end hour is invisible, change coordinates to bottom of grid
            end_at_margin = true;
        }

        double conc_evt_x_offset = (num_conc_evts > 0) ?
            ((slot_width_ / (num_conc_evts + 1)) * (conc_evt_instance)) : 0.0;
        
        int wk_day_num = (int)evt_itr->Start().date().day_of_week(), rm_id = evt_itr->RoomId();
        double start_x = MARGIN + (wk_day_num * slot_width_) + conc_evt_x_offset;

        /*
            Get number of slots between start time and day's end
            and end time and day's end.
            Y offset will be (end slots - start slots)  
        */
        int start_hours_from_end = end_hour_ - s_dur.hours(),
            end_hours_from_end = end_hour_ - e_dur.hours();
        int start_half_slots = (start_hours_from_end * 4) - (s_dur.minutes() / 15),
            end_half_slots = (end_hours_from_end * 4) + (e_dur.minutes() / 15);
        
        start_y = start_y ? start_y : MARGIN + (double)start_half_slots * (slot_height_ / 2.0);
        double y_offset = end_at_margin ? start_y - MARGIN : (double)(end_half_slots - start_half_slots) * (slot_height_ / 2.0);

        Event_Rect retval(start_x, start_y, (slot_width_ / (num_conc_evts + 1)), y_offset, rooms_[rm_id].color);        
        string chv = retval.color.hex_val();

        //only increment if there are concurrent events (calling code checks for conc_evt_instance == num_conc_evts + 1)
        if (num_conc_evts){
            ++conc_evt_instance;
        }
        
        return retval;
    }

    int WeekWriter::get_overlapping_events(list<Event>::const_iterator evt_itr) {
        using namespace boost::posix_time;
        /*
            NOTE: Pass in iterator by value
            1. get start and end dates from current iterator position
            2. while successive event start time is less than current event end time
                increment retval;
        */
        int retval = 0;
        time_period target_span(evt_itr->Start(), evt_itr->End());
        ++evt_itr;
        time_period next_span(evt_itr->Start(), evt_itr->End());
        while (target_span.intersects(next_span)){
            ++retval;
            //lengthen target_span if new event goes later            
            target_span = target_span.span(next_span);            
            ++evt_itr;
            next_span = time_period(evt_itr->Start(), evt_itr->End());
        }
        return retval;
    }
}