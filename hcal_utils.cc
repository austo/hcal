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
}