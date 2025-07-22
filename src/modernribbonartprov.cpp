/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * modernribbonartprov.cpp
 * Copyright (C) 2025 Emilien Kia <Emilien.Kia+dev@gmail.com>
 *
 * logviewer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * logviewer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// MyModernRibbonArtProvider.cpp

#include "modernribbonartprov.hpp"

#include <wx/dc.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/gdicmn.h>
#include <wx/ribbon/art_internal.h>
#include <wx/ribbon/toolbar.h>

MyModernRibbonArtProvider::MyModernRibbonArtProvider():
    wxRibbonMSWArtProvider()
{
    _tab_background_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION);
    _tab_active_background_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    _tab_ctrl_background_brush = wxBrush(_colour_page_background);
    _tab_ctrl_active_background_brush = wxBrush(_tab_active_background_colour);
    _tab_ctrl_border_pen = *wxTRANSPARENT_PEN;
    _tab_active_label_colour = _tab_hover_label_colour = _tab_label_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);


    _colour_page_background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    _brush_page_background = wxBrush(_colour_page_background);
    _pen_page_border = *wxTRANSPARENT_PEN;

    _panel_border_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
    _pen_panel_border = wxPen(_panel_border_colour, 1, wxPENSTYLE_SOLID);
    _panel_hovered_background_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION);
    _panel_hovered_background_brush = wxBrush(_panel_hovered_background_colour);
}

//
// Ribbon bar
//

void MyModernRibbonArtProvider::DrawTabCtrlBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    dc.SetPen(_tab_ctrl_border_pen);
    dc.SetBrush(_tab_ctrl_background_brush);
    dc.DrawRectangle(rect);
#if 0
    wxRibbonMSWArtProvider::DrawTabCtrlBackground(dc, wnd, rect);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxGREEN_BRUSH);
#endif
}

void MyModernRibbonArtProvider::DrawTab(wxDC& dc, wxWindow* wnd, const wxRibbonPageTabInfo& tab)
{
#if 0
    wxRibbonMSWArtProvider::DrawTab(dc, wnd, tab);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(tab.rect);
#endif

    if(tab.rect.height <= 2)
        return;

    if(tab.active || tab.hovered || tab.highlight)
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(_tab_ctrl_active_background_brush);
        dc.DrawRectangle(tab.rect);
    }

    // Inherited from wxRibbonMSWArtProvider:
    if(m_flags & wxRIBBON_BAR_SHOW_PAGE_ICONS)
    {
        wxBitmap icon = tab.page->GetIcon();
        if(icon.IsOk())
        {
            int x = tab.rect.x + 4;
            if((m_flags & wxRIBBON_BAR_SHOW_PAGE_LABELS) == 0)
                x = tab.rect.x + (tab.rect.width - icon.GetLogicalWidth()) / 2;
            dc.DrawBitmap(icon, x, tab.rect.y + 1 + (tab.rect.height - 1 -
                                                     icon.GetLogicalHeight()) / 2, true);
        }
    }
    // Inherited from wxRibbonMSWArtProvider:
    if(m_flags & wxRIBBON_BAR_SHOW_PAGE_LABELS)
    {
        wxString label = tab.page->GetLabel();
        if(!label.empty())
        {
            dc.SetFont(m_tab_label_font);

            if (tab.active)
            {
                dc.SetTextForeground(_tab_active_label_colour);
            }
            else if (tab.hovered)
            {
                dc.SetTextForeground(_tab_hover_label_colour);
            }
            else
            {
                dc.SetTextForeground(_tab_label_colour);
            }

            dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);

            int text_height;
            int text_width;
            dc.GetTextExtent(label, &text_width, &text_height);
            int width = tab.rect.width - 5;
            int x = tab.rect.x + 3;
            if(m_flags & wxRIBBON_BAR_SHOW_PAGE_ICONS)
            {
                x += 3 + tab.page->GetIcon().GetLogicalWidth();
                width -= 3 + tab.page->GetIcon().GetLogicalWidth();
            }
            int y = tab.rect.y + (tab.rect.height - text_height) / 2;

            if(width <= text_width)
            {
                dc.SetClippingRegion(x, tab.rect.y, width, tab.rect.height);
                dc.DrawText(label, x, y);
            }
            else
            {
                dc.DrawText(label, x + (width - text_width) / 2 + 1, y);
            }
        }
    }
}

void MyModernRibbonArtProvider::DrawTabSeparator(wxDC& dc, wxWindow* wnd, const wxRect& rect, double visibility)
{
#if 1
    wxRibbonMSWArtProvider::DrawTabSeparator(dc, wnd, rect, visibility);
    dc.SetPen(*wxYELLOW_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(rect);
#endif
}

void MyModernRibbonArtProvider::DrawPageBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) {
    dc.SetPen(_pen_page_border);
    dc.SetBrush(_brush_page_background);
#if 0
    wxRibbonMSWArtProvider::DrawPageBackground(dc, wnd, rect);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxGREEN_BRUSH);
#endif
    dc.DrawRectangle(rect);
}

//
// Ribbon panel
//

void MyModernRibbonArtProvider::DrawPanelBackground(wxDC &dc, wxRibbonPanel *wnd, const wxRect &rect) {

    // Inspirited from wxRibbonMSWArtProvider::DrawPanelBackground
    wxRect true_rect(rect);
    RemovePanelPadding(&true_rect);
    bool has_ext_button = wnd->HasExtButton();

    int label_height;
    {
        dc.SetFont(m_panel_label_font);
        dc.SetPen(*wxTRANSPARENT_PEN);
        if(wnd->IsHovered())
        {
            dc.SetBrush(m_panel_hover_label_background_brush);
            dc.SetTextForeground(m_panel_hover_label_colour);
        }
        else
        {
            dc.SetBrush(m_panel_label_background_brush);
            dc.SetTextForeground(m_panel_label_colour);
        }

        wxRect label_rect(true_rect);
        wxString label = wnd->GetLabel();
        bool clip_label = false;
        wxSize label_size(dc.GetTextExtent(label));

        label_rect.SetX(label_rect.GetX() + 1);
        label_rect.SetWidth(label_rect.GetWidth() - 2);
        label_rect.SetHeight(label_size.GetHeight() + 2);
        label_rect.SetY(true_rect.GetBottom() - label_rect.GetHeight());
        label_height = label_rect.GetHeight();

        wxRect label_bg_rect = label_rect;

        if(has_ext_button)
            label_rect.SetWidth(label_rect.GetWidth() - 13);

        if(label_size.GetWidth() > label_rect.GetWidth())
        {
            // Test if there is enough length for 3 letters and ...
            wxString new_label = label.Mid(0, 3) + "...";
            label_size = dc.GetTextExtent(new_label);
            if(label_size.GetWidth() > label_rect.GetWidth())
            {
                // Not enough room for three characters and ...
                // Display the entire label and just crop it
                clip_label = true;
            }
            else
            {
                // Room for some characters and ...
                // Display as many characters as possible and append ...
                for(size_t len = label.Len() - 1; len >= 3; --len)
                {
                    new_label = label.Mid(0, len) + "...";
                    label_size = dc.GetTextExtent(new_label);
                    if(label_size.GetWidth() <= label_rect.GetWidth())
                    {
                        label = new_label;
                        break;
                    }
                }
            }
        }

        if(clip_label)
        {
            wxDCClipper clip(dc, label_rect);
            dc.DrawText(label, label_rect.x, label_rect.y +
                                             (label_rect.GetHeight() - label_size.GetHeight()) / 2);
        }
        else
        {
            dc.DrawText(label, label_rect.x +
                               (label_rect.GetWidth() - label_size.GetWidth()) / 2,
                        label_rect.y +
                        (label_rect.GetHeight() - label_size.GetHeight()) / 2);
        }

        if(has_ext_button)
        {
            if(wnd->IsExtButtonHovered())
            {
                dc.SetPen(m_panel_hover_button_border_pen);
                dc.SetBrush(m_panel_hover_button_background_brush);
                dc.DrawRoundedRectangle(label_rect.GetRight(), label_rect.GetBottom() - 13, 13, 13, 1.0);
                dc.DrawBitmap(m_panel_extension_bitmap[1], label_rect.GetRight() + 3, label_rect.GetBottom() - 10, true);
            }
            else
                dc.DrawBitmap(m_panel_extension_bitmap[0], label_rect.GetRight() + 3, label_rect.GetBottom() - 10, true);
        }
    }

#if 0
    wxRibbonMSWArtProvider::DrawPanelBackground(dc, wnd, rect);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxGREEN_BRUSH);
    dc.DrawRectangle(rect);
#endif

    // Draw panel separator
    wxRect separator_rect = true_rect;
    separator_rect.Deflate(0, 8);
    dc.SetPen(_pen_panel_border);
    dc.DrawLine(separator_rect.GetTopRight(), separator_rect.GetBottomRight());
}

void MyModernRibbonArtProvider::DrawMinimisedPanel(wxDC& dc, wxRibbonPanel* wnd, const wxRect& rect, wxBitmap& bitmap) {

    dc.SetPen(*wxTRANSPARENT_PEN);
    if(wnd->IsHovered()) {
        dc.SetBrush(_panel_hovered_background_brush);
        dc.DrawRectangle(rect);
    }

    dc.SetPen(_pen_panel_border);
    dc.DrawLine(rect.GetTopRight(), rect.GetBottomRight());

    // Preview Icon, Label and Arrow:
    // Inspirated to wxRibbonMSWArtProvider::DrawMinimisedPanel[Common]
    wxRect preview(0, 0, 32, 32);
    if(m_flags & wxRIBBON_BAR_FLOW_VERTICAL)
    {
        preview.x = rect.x + 4;
        preview.y = rect.y + (rect.height - preview.height) / 2;
    }
    else
    {
        preview.x = rect.x + (rect.width - preview.width) / 2;
        preview.y = rect.y + 4;
    }

    // Preview label:
    wxCoord label_width, label_height;
    dc.SetFont(m_panel_label_font);
    dc.GetTextExtent(wnd->GetLabel(), &label_width, &label_height);

    int xpos = rect.x + (rect.width - label_width + 1) / 2;
    int ypos = preview.y + preview.height + 5;

    if(m_flags & wxRIBBON_BAR_FLOW_VERTICAL)
    {
        xpos = preview.x + preview.width + 5;
        ypos = rect.y + (rect.height - label_height) / 2;
    }

    dc.SetTextForeground(m_panel_minimised_label_colour);
    dc.DrawText(wnd->GetLabel(), xpos, ypos);

    // Preview arrow:
    wxPoint arrow_points[3];
    if(m_flags & wxRIBBON_BAR_FLOW_VERTICAL)
    {
        xpos += label_width;
        arrow_points[0] = wxPoint(xpos + 5, ypos + label_height / 2);
        arrow_points[1] = arrow_points[0] + wxPoint(-3,  3);
        arrow_points[2] = arrow_points[0] + wxPoint(-3, -3);
    }
    else
    {
        ypos += label_height;
        arrow_points[0] = wxPoint(rect.width / 2, ypos + 5);
        arrow_points[1] = arrow_points[0] + wxPoint(-3, -3);
        arrow_points[2] = arrow_points[0] + wxPoint( 3, -3);
    }

    dc.SetPen(*wxTRANSPARENT_PEN);
    wxBrush B(m_panel_minimised_label_colour);
    dc.SetBrush(B);
    dc.DrawPolygon(sizeof(arrow_points)/sizeof(wxPoint), arrow_points, rect.x, rect.y);

    // Preview icon
    if(bitmap.IsOk())
    {
        dc.DrawBitmap(bitmap, preview.x + (preview.width - bitmap.GetLogicalWidth()) / 2,
                      preview.y + (preview.height - 7 - bitmap.GetLogicalHeight()) / 2, true);
    }

#if 0
    wxRibbonMSWArtProvider::DrawMinimisedPanel(dc, wnd, rect, bitmap);
    dc.SetPen(*wxRED_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);;
    dc.DrawRectangle(rect);
#endif
}

//
// Ribbon button bar
//
void MyModernRibbonArtProvider::DrawButtonBarBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    // Do nothing, let it transparent
}

void MyModernRibbonArtProvider::DrawButtonBarButton(wxDC& dc, wxWindow* wnd, const wxRect& rect, wxRibbonButtonKind kind, long state, const wxString& label, const wxBitmap& bitmap_large, const wxBitmap& bitmap_small)
{
    // Inspirited to wxRibbonMSWArtProvider::DrawButtonBarButton

    if(kind == wxRIBBON_BUTTON_TOGGLE)
    {
        kind = wxRIBBON_BUTTON_NORMAL;
        if(state & wxRIBBON_BUTTONBAR_BUTTON_TOGGLED)
            state ^= wxRIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK;
    }

    if(state & (wxRIBBON_BUTTONBAR_BUTTON_HOVER_MASK |
                wxRIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK))
    {
        if(state & wxRIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK)
            dc.SetPen(m_button_bar_active_border_pen);
        else
            dc.SetPen(m_button_bar_hover_border_pen);

        wxRect bg_rect(rect);
        bg_rect.x++;
        bg_rect.y++;
        bg_rect.width -= 2;
        bg_rect.height -= 2;

        wxRect bg_rect_top(bg_rect);
        bg_rect_top.height /= 3;
        bg_rect.y += bg_rect_top.height;
        bg_rect.height -= bg_rect_top.height;

        if(state & wxRIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK)
        {
            dc.SetBrush(m_button_bar_active_background_top_colour);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(rect);
        }
        else
        {
            dc.SetBrush(m_button_bar_hover_background_top_colour);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(rect);
        }

        if(kind == wxRIBBON_BUTTON_HYBRID)
        {
            switch(state & wxRIBBON_BUTTONBAR_BUTTON_SIZE_MASK)
            {
                case wxRIBBON_BUTTONBAR_BUTTON_LARGE:
                {
                    int iYBorder = rect.y + bitmap_large.GetLogicalHeight() + 4;
                    wxRect partial_bg(rect);
                    if(state & wxRIBBON_BUTTONBAR_BUTTON_NORMAL_HOVERED)
                    {
                        partial_bg.SetBottom(iYBorder - 1);
                    }
                    else
                    {
                        partial_bg.height -= (iYBorder - partial_bg.y + 1);
                        partial_bg.y = iYBorder + 1;
                    }
                    dc.DrawLine(rect.x, iYBorder, rect.x + rect.width, iYBorder);
                    bg_rect.Intersect(partial_bg);
                    bg_rect_top.Intersect(partial_bg);
                }
                    break;
                case wxRIBBON_BUTTONBAR_BUTTON_MEDIUM:
                {
                    int iArrowWidth = 9;
                    if(state & wxRIBBON_BUTTONBAR_BUTTON_NORMAL_HOVERED)
                    {
                        bg_rect.width -= iArrowWidth;
                        bg_rect_top.width -= iArrowWidth;
                        dc.DrawLine(bg_rect_top.x + bg_rect_top.width,
                                    rect.y, bg_rect_top.x + bg_rect_top.width,
                                    rect.y + rect.height);
                    }
                    else
                    {
                        --iArrowWidth;
                        bg_rect.x += bg_rect.width - iArrowWidth;
                        bg_rect_top.x += bg_rect_top.width - iArrowWidth;
                        bg_rect.width = iArrowWidth;
                        bg_rect_top.width = iArrowWidth;
                        dc.DrawLine(bg_rect_top.x - 1, rect.y,
                                    bg_rect_top.x - 1, rect.y + rect.height);
                    }
                }
                    break;
                case wxRIBBON_BUTTONBAR_BUTTON_SMALL:
                    break;
            }
        }

        wxPoint border_points[9];
        border_points[0] = wxPoint(2, 0);
        border_points[1] = wxPoint(rect.width - 3, 0);
        border_points[2] = wxPoint(rect.width - 1, 2);
        border_points[3] = wxPoint(rect.width - 1, rect.height - 3);
        border_points[4] = wxPoint(rect.width - 3, rect.height - 1);
        border_points[5] = wxPoint(2, rect.height - 1);
        border_points[6] = wxPoint(0, rect.height - 3);
        border_points[7] = wxPoint(0, 2);
        border_points[8] = border_points[0];

        dc.DrawLines(sizeof(border_points)/sizeof(wxPoint), border_points,
                     rect.x, rect.y);
    }

    dc.SetFont(m_button_bar_label_font);
    dc.SetTextForeground(state & wxRIBBON_BUTTONBAR_BUTTON_DISABLED
                         ? m_button_bar_label_disabled_colour
                         : m_button_bar_label_colour);
    DrawButtonBarButtonForeground(dc, rect, kind, state, label, bitmap_large, bitmap_small);
}

//
// Ribbon tool bar
//
void MyModernRibbonArtProvider::DrawToolBarBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    // Do nothing, let it transparent
}


void MyModernRibbonArtProvider::DrawToolGroupBackground(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxRect& rect)
{
    // Do nothing, let it transparent
/*
    dc.SetPen(m_toolbar_border_pen);
    wxPoint outline[9];
    outline[0] = wxPoint(2, 0);
    outline[1] = wxPoint(rect.width - 3, 0);
    outline[2] = wxPoint(rect.width - 1, 2);
    outline[3] = wxPoint(rect.width - 1, rect.height - 3);
    outline[4] = wxPoint(rect.width - 3, rect.height - 1);
    outline[5] = wxPoint(2, rect.height - 1);
    outline[6] = wxPoint(0, rect.height - 3);
    outline[7] = wxPoint(0, 2);
    outline[8] = outline[0];

    dc.DrawLines(sizeof(outline)/sizeof(wxPoint), outline, rect.x, rect.y);
*/
}

void MyModernRibbonArtProvider::DrawTool(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxRect& rect, const wxBitmap& bitmap, wxRibbonButtonKind kind, long state)
{
    if(kind == wxRIBBON_BUTTON_TOGGLE)
    {
        if(state & wxRIBBON_TOOLBAR_TOOL_TOGGLED)
            state ^= wxRIBBON_TOOLBAR_TOOL_ACTIVE_MASK;
    }

    if(state & (wxRIBBON_TOOLBAR_TOOL_ACTIVE_MASK|wxRIBBON_TOOLBAR_TOOL_HOVER_MASK) )
    {
        if(state & wxRIBBON_TOOLBAR_TOOL_ACTIVE_MASK)
            dc.SetBrush(wxBrush(m_tool_active_background_top_colour));
        else if(state & wxRIBBON_TOOLBAR_TOOL_HOVER_MASK)
            dc.SetBrush(wxBrush(m_tool_active_background_top_colour));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rect);
    }
    bool is_split_hybrid = (kind == wxRIBBON_BUTTON_HYBRID &&
            (state & (wxRIBBON_TOOLBAR_TOOL_HOVER_MASK | wxRIBBON_TOOLBAR_TOOL_ACTIVE_MASK)));
    if(is_split_hybrid)
    {
        wxRect nonrect(rect);
        if(state & (wxRIBBON_TOOLBAR_TOOL_DROPDOWN_HOVERED |
                    wxRIBBON_TOOLBAR_TOOL_DROPDOWN_ACTIVE))
        {
            nonrect.width -= 8;
        }
        else
        {
            nonrect.x += nonrect.width - 8;
            nonrect.width = 8;
        }
        wxBrush B(m_tool_hover_background_top_colour);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(B);
        dc.DrawRectangle(nonrect.x, nonrect.y, nonrect.width, nonrect.height);
    }

    // No border

    // Foreground
    int avail_width = rect.GetWidth();
    if(kind & wxRIBBON_BUTTON_DROPDOWN)
    {
        avail_width -= 8;
        if(is_split_hybrid)
        {
            dc.DrawLine(rect.x + avail_width + 1, rect.y,
                        rect.x + avail_width + 1, rect.y + rect.height);
        }
        dc.DrawBitmap(m_toolbar_drop_bitmap, rect.x + avail_width + 2,
                      rect.y + (rect.height / 2) - 2, true);
    }
    dc.DrawBitmap(bitmap, rect.x + (avail_width - bitmap.GetLogicalWidth()) / 2,
                  rect.y + (rect.height - bitmap.GetLogicalHeight()) / 2, true);
}