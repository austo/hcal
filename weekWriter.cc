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
    }

    WeekWriter::WeekWriter()
    {
        eventMap_ = new map< int, list<Event> >();
        start_hour_ = 8;
        end_hour_ = 20;
        slot_height_ = 0;
        slot_width_ = 0;      
    }

    WeekWriter::WeekWriter(time_t start, time_t end)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end, week);
        room_colors_ = dl.get_room_colors();
        start_hour_ = 8;
        end_hour_ = 20;
        slot_height_ = 0;
        slot_width_ = 0;
    }

    WeekWriter::WeekWriter(time_t start, time_t end, int start_hour, int end_hour)
    {
        DataLayer dl = DataLayer();   
        eventMap_ = dl.get_event_map(start, end, week);
        room_colors_ = dl.get_room_colors();
        start_hour_ = start_hour;
        end_hour_ = end_hour;
        slot_height_ = 0;
        slot_width_ = 0;
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
        int pg_used_width = HPDF_Page_GetWidth(page) - 100;
        int pg_used_height = HPDF_Page_GetHeight(page) - 125;

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
        write_weekday_cols(page, font, pg_used_width, pg_used_height, 7, MARGIN);

        /*
            TODO: this method should take a configWrapper obj
            If it's a regular week, we make a configWrapper in calling
            method and use it here
            (Only bother unpacking configWrapper obj if we've got a custom view)

            NOTE: should get start_hour and num_hours earlier in lifecycle & make slot_height a member variable
            NOTE: CustomWriter will inherit from WeekWriter (or vice versa?)
        */
        write_hour_rows(page, pg_used_width, pg_used_height);

        slot_width_ = slot_width_ ? slot_width_ : ((double)pg_used_width / 7);
        int evtMargin = MARGIN + 5;

        write_events(page, week_ordinal);
    }

    void
    WeekWriter::write_hour_rows(HPDF_Page page, int pg_used_width, int pg_used_height)
    {
        int num_lines = (end_hour_ - start_hour_) * 2;

        //initialize slot_height_ if we haven't already
        slot_height_ = slot_height_ ? slot_height_ : (double)pg_used_height / (double)num_lines;        
        float line_y = MARGIN + slot_height_;

        HPDF_Page_SetLineWidth(page, .10);

        for (int i = 0, hrln = 1; i < num_lines; ++i){
            if (i == hrln){
                HPDF_Page_SetLineWidth(page, .6);
                draw_line(page, MARGIN, line_y, MARGIN + pg_used_width, line_y);
                HPDF_Page_SetLineWidth(page, .15);
                hrln += 2;
            }
            else{
               draw_line(page, MARGIN, line_y, MARGIN + pg_used_width, line_y); 
            }

            line_y += slot_height_;
        }
    }

    void
    WeekWriter::write_events(HPDF_Page page, int week_ordinal)
    {
        int num_conc_evts = 0, conc_evt_instance;
        HPDF_Page_SetLineWidth(page, 2);
        list<Event>::const_iterator evt_itr = (*eventMap_)[week_ordinal].begin();
        for (; evt_itr != (*eventMap_)[week_ordinal].end(); ++evt_itr){
            cout << "v8 - evt title: " << evt_itr->Description() << "; start time: " << evt_itr->Start() << endl;
            //decrement events if not zero
            if (!num_conc_evts){
                //cout << "v8 - getting conc events... " << endl;
                num_conc_evts = get_overlapping_events(evt_itr);
                cout << "v8 - num conc events: " << num_conc_evts << endl;
                conc_evt_instance = 0;
            }            
            Event_Rect evt_rect = get_event_rect(evt_itr, num_conc_evts, conc_evt_instance);

            //TODO: this should either be a class member or be called from within a utility function
            draw_event_rect(page, evt_rect);

            if (conc_evt_instance == num_conc_evts){
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

        double conc_evt_x_offset = (num_conc_evts > 0) ?
            ((slot_width_ / (num_conc_evts + 1)) * (conc_evt_instance + 1)): 0.0;
        //cout << "v8 - evt title: " << evt_itr->Description() << "; conc_evt_x_offset: " << conc_evt_x_offset << endl;
        
        //get width of event_rect based on slot_width_ / number of concurrent events
        //get start x coordinates of event_rect based on start_x + (evt_width * remaining_evts)
        int wk_day_num = (int)evt_itr->Start().date().day_of_week(), rm_id = evt_itr->RoomId();
        double start_x = MARGIN + (wk_day_num * slot_width_) + conc_evt_x_offset;        

        time_duration s_dur(evt_itr->Start().time_of_day()), e_dur(evt_itr->End().time_of_day());

        /*
            Get number of slots between start time and day's end
            and end time and day's end.
            Y offset will be (end slots - start slots)  
        */
        int start_hours_from_end = end_hour_ - s_dur.hours(),
            end_hours_from_end = end_hour_ - e_dur.hours();
        int start_half_slots = (start_hours_from_end * 4) - (s_dur.minutes() / 15),
            end_half_slots = (end_hours_from_end * 4) + (e_dur.minutes() / 15);
        
        double start_y = MARGIN + (double)start_half_slots * (slot_height_ / 2.0);
        double y_offset = (double)(end_half_slots - start_half_slots) * (slot_height_ / 2.0);

        Event_Rect retval(start_x, start_y, (slot_width_ / (num_conc_evts + 1)), y_offset, room_colors_[rm_id]);        
        string chv = retval.color.hex_val();

        //only increment if there are concurrent events (calling code checks for conc_evt_instance == num_conc_evts)
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