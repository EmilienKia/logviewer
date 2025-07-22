/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * modernribbonartprov.hpp
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

#ifndef LOGVIEWER_MODERNRIBBONARTPROV_HPP
#define LOGVIEWER_MODERNRIBBONARTPROV_HPP

#include <wx/wx.h>

#include <wx/ribbon/art.h>
#include <wx/graphics.h>

class MyModernRibbonArtProvider : public wxRibbonMSWArtProvider
{
public:
    MyModernRibbonArtProvider();
    ~MyModernRibbonArtProvider() override = default;

    // Ribbon bar:
    void DrawTabCtrlBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawTab(wxDC& dc, wxWindow* wnd, const wxRibbonPageTabInfo& tab) override;
    void DrawTabSeparator(wxDC& dc, wxWindow* wnd, const wxRect& rect, double visibility) override;

    // Ribbon bar page:
    void DrawPageBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;

    // Ribbon panel
    void DrawPanelBackground(wxDC &dc, wxRibbonPanel *wnd, const wxRect &rect) override;
    void DrawMinimisedPanel(wxDC& dc, wxRibbonPanel* wnd, const wxRect& rect, wxBitmap& bitmap) override;

    // Ribbon button bar
    void DrawButtonBarBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawButtonBarButton(wxDC& dc, wxWindow* wnd, const wxRect& rect, wxRibbonButtonKind kind, long state, const wxString& label, const wxBitmap& bitmap_large, const wxBitmap& bitmap_small) override;

    // Ribbon tool bar
    void DrawToolBarBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawToolGroupBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawTool(wxDC& dc, wxWindow* wnd, const wxRect& rect, const wxBitmap& bitmap, wxRibbonButtonKind kind, long state) override;
protected:



    wxColour _tab_background_colour;
    wxColour _tab_active_background_colour;
    wxBrush _tab_ctrl_background_brush;
    wxBrush _tab_ctrl_active_background_brush;
    wxPen _tab_ctrl_border_pen;
    wxColour _tab_active_label_colour;
    wxColour _tab_hover_label_colour;
    wxColour _tab_label_colour;

    wxColour _colour_page_background;
    wxBrush _brush_page_background;
    wxPen _pen_page_border;

    wxColour _panel_border_colour;
    wxPen _pen_panel_border;
    wxColour _panel_hovered_background_colour;
    wxBrush _panel_hovered_background_brush;
};


#endif //LOGVIEWER_MODERNRIBBONARTPROV_HPP
