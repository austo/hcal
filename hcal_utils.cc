#include "hcal_utils.h"
using namespace std;

#define ERR_CREATE_PDF "MonthWriter: failed to create HPDF object."
#define ERR_UREC_VIEW "EventWriter: Unrecognized calendar view."

namespace hcal {

    void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
    {
        stringstream ss;
        ss << "ERROR: error_no = " << (HPDF_UINT)error_no << "\ndetail_no = " << (HPDF_UINT)detail_no << endl;    
        throw runtime_error(ss.str());
    }
    
    void draw_line(HPDF_Page page, float x_start, float y_start, float x_end, float y_end)
    {
        HPDF_Page_MoveTo (page, x_start, y_start);
        HPDF_Page_LineTo (page, x_end, y_end);
        HPDF_Page_Stroke (page);
    }

    void draw_event_rect(HPDF_Page page, Event_Rect& evt_rect)
    {
        if (evt_rect.is_out_of_bounds()){
            return;
        }

        HPDF_Page_SetRGBStroke (page, evt_rect.color.dec_red(), evt_rect.color.dec_green(), evt_rect.color.dec_blue());
        HPDF_Page_SetLineJoin (page, HPDF_ROUND_JOIN);
        HPDF_Page_MoveTo (page, evt_rect.l_left.x, evt_rect.l_left.y);
        HPDF_Page_LineTo (page, evt_rect.l_right.x, evt_rect.l_right.y);
        HPDF_Page_LineTo (page, evt_rect.u_right.x, evt_rect.u_right.y);
        HPDF_Page_LineTo (page, evt_rect.u_left.x, evt_rect.u_left.y);
        HPDF_Page_LineTo (page, evt_rect.l_left.x, evt_rect.l_left.y);        
        HPDF_Page_Stroke (page);
        //HPDF_Page_FillStroke (page);
    }

    HPDF_Doc get_pdf()
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

    void write_text(HPDF_Page page, float x_offset, float y_offset, const char* text)
    {
        HPDF_Page_BeginText (page);
        HPDF_Page_MoveTextPos (page, x_offset, y_offset);
        HPDF_Page_ShowText (page, text);
        HPDF_Page_EndText (page);
    }

    void write_page_title(HPDF_Page page, HPDF_Font font, const char* page_title)
    {
        /* print the title of the page (with positioning center). */
        HPDF_Page_SetFontAndSize(page, font, 20);
        float tw = HPDF_Page_TextWidth(page, page_title);
        write_text(page, (HPDF_Page_GetWidth(page) - tw) / 2, HPDF_Page_GetHeight(page) - 40, page_title);
        HPDF_Page_SetFontAndSize(page, font, 10);
        HPDF_Page_SetLineWidth(page, 1);
    }

    void write_weekday_cols(HPDF_Page page, HPDF_Font font, int pageUsedWidth, int pageUsedHeight, int numCols, float margin)
    {
        HPDF_Page_SetLineWidth(page, 1);
        HPDF_Page_Rectangle(page, margin, margin, pageUsedWidth, pageUsedHeight);
        HPDF_Page_Stroke(page);

        float dayWidth = ((float)pageUsedWidth / numCols);

        //vertical lines
        int i, j;
        for (i = 1, j = numCols + 1; i < j; ++i)
        {
            float line_x = margin + (dayWidth * i);
            draw_line(page, line_x, margin, line_x, margin + pageUsedHeight);        
        }

        //weekdays
        float daymargin = margin + (dayWidth / 2);
        for (i = 0; i < 7; ++i){
            float tw = HPDF_Page_TextWidth(page, weekdays()[i]);
            float textpos = (daymargin - (tw /2)) + (dayWidth * i);
            write_text(page, textpos, HPDF_Page_GetHeight(page) - (margin + 15), weekdays()[i]);        
        }
    }

    void write_wrapped_text(HPDF_Page page,
        float x_offset, float& y_offset, const string str_text, float line_height, float avail_width)
    {
        string temp;
        float tw = HPDF_Page_TextWidth(page, str_text.c_str());

        if (tw > avail_width){
            const char* delim = " ";
            vector<string> tokens = get_words(str_text, delim);            

            vector<string>::iterator vit = tokens.begin();
            for (; vit != tokens.end(); ++vit){
                temp += *vit;
                temp += " ";
                if (HPDF_Page_TextWidth(page, temp.c_str()) > avail_width){
                    temp = temp.substr(0, temp.size() - 1);
                    temp = temp.substr(0, temp.find_last_of(" "));
                    write_text(page, x_offset, y_offset, temp.c_str());
                    y_offset -= line_height;
                    temp = string(*vit);
                    temp += " ";
                }
            }
            write_text(page, x_offset, y_offset, temp.c_str());
            y_offset -= line_height;          
        }
        else{
            write_text(page, x_offset, y_offset, str_text.c_str());
            y_offset -= line_height;
        }
    }

    vector<string> get_words(const string str, const char* delim)
    {
        int len = str.size() + 1;
        char temp[len], *wp;
        strcpy(temp, str.c_str());
        vector<string> retval;
        char *p = strtok_r(temp, delim, &wp);
        while(p){
            string w_str(p);
            retval.push_back(w_str);
            p = strtok_r(NULL, delim, &wp);
        }
        return retval;
    }

    void write_if_more_evts(int more_evts, HPDF_Page page, float x_offset, float y_offset){
        if (more_evts){
            y_offset -= 5;
            string sentence_end = more_evts == 1 ? "." : "s.";
            stringstream mss;
            mss << ". . . and " << more_evts << " more event" << sentence_end;
            write_text(page, x_offset, y_offset, mss.str().c_str());
        }
    }

    View get_view(v8::String::AsciiValue& viewStr)
    {
        if (strcmp(*viewStr, "month") == 0){
            return month;
        }
        else if (strcmp(*viewStr, "week") == 0){
            return week;
        }
        else if (strcmp(*viewStr, "day") == 0){
            return day;
        }
        else{
            throw runtime_error(ERR_UREC_VIEW);
        }
    }

    boost::posix_time::time_duration
    get_time_offset_from_pg_timestamp(const string& timestamp_str){
        unsigned last_dash = timestamp_str.find_last_of("-+");
        if (timestamp_str.at(last_dash + 1) == '0'){
            last_dash += 2;
        }
        else{
            last_dash += 1;
        }
        string offset_str = timestamp_str.substr(last_dash);
        int offset = atoi(offset_str.c_str());
        return boost::posix_time::hours(offset);
    }

    //TODO: consolidate
    void write_hour_to_buf(char* buf, int hr, bool write_meridian){
        //assume military time
        if (hr > 12){
            hr -= 12;
            if (write_meridian){
                if (hr < 10){
                    sprintf(buf, " %d pm", hr);
                }
                else{
                    sprintf(buf, "%d pm", hr);
                }
            }
            else{
                if (hr < 10){
                    sprintf(buf, " %d", hr);
                }
                else{
                    sprintf(buf, " %d", hr);
                }
            }
        }
        else if (hr == 12){
            if (write_meridian){
                sprintf(buf, "%d pm", hr);
            }
            else{
                sprintf(buf, "%d", hr);
            }
        }
        else if (hr == 0){
            hr = 12;
            if (write_meridian){
                sprintf(buf, "%d am", hr);                
            }
            else{
                sprintf(buf, "%d", hr);
            }
        }
        else{
            if (write_meridian){
                if (hr < 10){
                    sprintf(buf, " %d am", hr);
                }
                else{
                    sprintf(buf, "%d am", hr);
                }
            }
            else{
                if (hr < 10){
                    sprintf(buf, " %d", hr);
                }
                else{
                    sprintf(buf, "%d", hr);
                }
            }
        }
    }
}