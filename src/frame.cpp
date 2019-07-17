/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * frame.cpp
 * Copyright (C) 2018 Emilien Kia <Emilien.Kia+dev@gmail.com>
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <wx/wx.h>

#include <wx/artprov.h>
#include <wx/dirdlg.h>
#include <wx/slider.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/dateevt.h>

#include "frame.hpp"


static inline wxBitmap wxArtIcon(const wxArtID &id, unsigned int sz)
{
	return wxArtProvider::GetBitmap(id, wxART_OTHER, wxSize(sz,sz));
}

static inline wxBitmap wxRibbonBmp(const wxArtID &id)
{
	return wxArtIcon(id, 32);
}



//
// DateTimeCtrl
//
DateTimeCtrl::DateTimeCtrl()
{
}

DateTimeCtrl::DateTimeCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
{
	Create(parent, id, pos, size);
}

bool DateTimeCtrl::Create(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
{
	if(!wxWindow::Create(parent, id, pos, size, wxTAB_TRAVERSAL|wxBORDER_NONE|wxCLIP_CHILDREN|wxTRANSPARENT_WINDOW, "DateTimeCtrl"))
	{
		return false;
	}
	wxSizer* szr = new wxBoxSizer(wxHORIZONTAL);
	_date = new wxDatePickerCtrl(this, wxID_ANY);
	_time = new wxTimePickerCtrl(this, wxID_ANY);
	szr->Add(_date, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);
	szr->Add(_time, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2);
	szr->Add(new wxBitmapButton(this, wxID_REVERT, wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_BUTTON)));
	SetSizer(szr);
	return true;
}

void DateTimeCtrl::SetValue(const wxDateTime& date)
{
	_curr = date;
	_date->SetValue(_curr);
	_time->SetValue(_curr);
}

void DateTimeCtrl::SetDefault(const wxDateTime& date)
{
	_def = date;
}

BEGIN_EVENT_TABLE(DateTimeCtrl, wxWindow)
	EVT_DATE_CHANGED(wxID_ANY, DateTimeCtrl::OnDateEvent)
	EVT_TIME_CHANGED(wxID_ANY, DateTimeCtrl::OnDateEvent)
	EVT_BUTTON(wxID_REVERT, DateTimeCtrl::OnRevertEvent)
END_EVENT_TABLE()

void DateTimeCtrl::OnRevertEvent(wxCommandEvent& event)
{
	SetValue(_def);
	SendNotification();
}

void DateTimeCtrl::OnDateEvent(wxDateEvent& /*event*/)
{
	wxDateTime d = _date->GetValue();
	wxDateTime t = _time->GetValue();
	if(d.IsValid() && t.IsValid())
	{
		_curr.Set(d.GetDay(), d.GetMonth(), d.GetYear(), t.GetHour(), t.GetMinute(), t.GetSecond());
	}
	else if(d.IsValid())
	{
		_curr.Set(d.GetDay(), d.GetMonth(), d.GetYear(), 0, 0, 0);
	}
	else if(t.IsValid())
	{
		_curr.Set(t.GetHour(), t.GetMinute(), t.GetSecond(), 0);
	}
	else
	{
		_curr = wxDateTime();
	}
	SendNotification();
}

void DateTimeCtrl::SendNotification()
{
	wxDateEvent evt(this, _curr, wxEVT_DATE_CHANGED);
	wxPostEvent(GetParent(), evt);
}

//
// Frame
//

Frame::Frame():
wxFrame(NULL, wxID_ANY, wxGetApp().GetAppDisplayName(), wxDefaultPosition, wxSize(1024, 768)/*wxDefaultSize*/, wxDEFAULT_FRAME_STYLE)
{
	init();
}

Frame::~Frame()
{
	_manager.UnInit();
}

void Frame::init()
{
	_logModel = new LogListModel(wxGetApp().GetFilteredLogData());
	_loggerModel = new LoggerListModel(wxGetApp().GetFilteredLogData());
	wxGetApp().GetLogData().AddListener(this);

	_status = CreateStatusBar();

	_manager.SetManagedWindow(this);

	// Ribbon bar init
	{
		_ribbon = new wxRibbonBar(this, wxID_ANY, wxDefaultPosition, /*wxSize(800, 600)*/wxDefaultSize,
								 wxRIBBON_BAR_FLOW_HORIZONTAL
								| wxRIBBON_BAR_SHOW_PAGE_LABELS
								| wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS
								//| wxRIBBON_BAR_SHOW_TOGGLE_BUTTON
								//| wxRIBBON_BAR_SHOW_HELP_BUTTON
								);

		{
			wxRibbonPage* page = new wxRibbonPage(_ribbon, wxID_ANY, "LogViewer");
			{
				wxRibbonPanel* panel = new wxRibbonPanel(page, wxID_ANY, "Files");
				wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
				bar->AddButton(wxID_OPEN, "Open", wxRibbonBmp(wxART_FILE_OPEN));
				bar->AddButton(wxID_CLEAR, "Clear", wxRibbonBmp(wxART_NEW));
			}
			{
				wxRibbonPanel *panel = new wxRibbonPanel(page, wxID_ANY, "Criticality");
				_criticalitySlider = new wxSlider(panel, wxID_ANY, LOG_INFO, LOG_UNKNWON, LOG_FATAL, wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW|wxSL_HORIZONTAL| wxSL_AUTOTICKS);
				_criticalitySlider->SetMinSize(wxSize(128, -1));
				_criticalityText = new wxStaticText(panel, wxID_ANY, "INFO", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL|wxTRANSPARENT_WINDOW);
				wxSizer* szr = new wxBoxSizer(wxVERTICAL);
				szr->Add(_criticalitySlider, 1, wxEXPAND|wxALL, 2);
				szr->Add(_criticalityText, 1, wxEXPAND|wxALL, 2);
				panel->SetSizer(szr);
			}
			{
				wxRibbonPanel *panel = new wxRibbonPanel(page, wxID_ANY, "Time frame");
				_begin = new DateTimeCtrl(panel, ID_LV_BEGIN_DATE);
				_end   = new DateTimeCtrl(panel, ID_LV_END_DATE);

				wxFlexGridSizer* szr = new wxFlexGridSizer(2, 2, 2, 2);
				szr->Add(new wxStaticText(panel, wxID_ANY, "From:", wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW), 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
				szr->Add(_begin, 1, wxEXPAND|wxALL, 2);
				szr->Add(new wxStaticText(panel, wxID_ANY, "To:", wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW), 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
				szr->Add(_end, 1, wxEXPAND|wxALL, 2);
				panel->SetSizer(szr);
			}
			{
				wxRibbonPanel* panel = new wxRibbonPanel(page, ID_LV_LOGGER_PANEL, "Loggers", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_EXT_BUTTON);
				wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
				bar->AddButton(ID_LV_SHOW_ALL_LOGGERS, "All loggers", wxRibbonBmp(wxART_TICK_MARK), "Show entries for all loggers");
				bar->AddButton(ID_LV_SHOW_NO_LOGGERS, "No logger", wxRibbonBmp(wxART_CROSS_MARK), "Show entries for no logger (display nothing)");
				bar->AddButton(ID_LV_SHOW_ONLY_CURRENT_LOGGER, "Only current logger", wxRibbonBmp(wxART_MINUS), "Show entries for logger of currently selected entry only");
				bar->AddButton(ID_LV_SHOW_ALL_BUT_CURRENT_LOGGER, "All but current logger", wxRibbonBmp(wxART_PLUS), "Show entries for all loggers but currently selected entry");
				bar->AddButton(ID_LV_FOCUS_PREVIOUS_CURRENT_LOGGER, "Focus previous", wxRibbonBmp(wxART_GO_UP), "Focus previous entry with current logger");
				bar->AddButton(ID_LV_FOCUS_NEXT_CURRENT_LOGGER, "Focus next", wxRibbonBmp(wxART_GO_DOWN), "Focus next entry with current logger");
			}
		}
		_ribbon->Realise();
		_manager.AddPane(_ribbon, wxAuiPaneInfo().Top().Floatable(false).CaptionVisible(false).CloseButton(false).BestSize(-1, 112).Show(true));
	}

	// Menu
#if 0 // Dont create menu, use ribbon instead
	{
		wxMenuBar* menubar = new wxMenuBar;
		{
			wxMenu* menu = new wxMenu;
			menu->Append(wxID_OPEN);
			menu->AppendSeparator();
			menu->Append(wxID_EXIT);
			menubar->Append(menu, "File");
		}
		SetMenuBar(menubar);
	}
#endif // 0 // Dont create menu, use ribbon instead

	// Log panel
	{
		_logs = new wxDataViewCtrl(this, ID_LV_LOGS);
		_logs->AppendTextColumn("Criticality",	LogListModel::CRITICALITY,	wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);
		_logs->AppendTextColumn("Date", 		LogListModel::DATE,			wxDATAVIEW_CELL_INERT, 180, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);
		_logs->AppendTextColumn("Logger", 		LogListModel::LOGGER,		wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);
		_logs->AppendTextColumn("Source", 		LogListModel::SOURCE,		wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);
		_logs->AppendToggleColumn("Extra", 		LogListModel::EXTRA,		wxDATAVIEW_CELL_INERT, 48, wxALIGN_CENTER, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);
		_logs->AppendTextColumn("Message", 		LogListModel::MESSAGE,		wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_REORDERABLE);

		_logs->AssociateModel(_logModel);

		_manager.AddPane(_logs, wxAuiPaneInfo().CenterPane().Name("Logs").Show(true));
	}

	// Loggers
	{
		_loggers = new wxDataViewCtrl(this, ID_LV_LOGGER_LISTBOX);
		_loggers->AppendToggleColumn("", LoggerListModel::SHOWN, wxDATAVIEW_CELL_ACTIVATABLE, 48, wxALIGN_CENTER, /*wxDATAVIEW_COL_RESIZABLE |*/ wxDATAVIEW_COL_REORDERABLE);
		_loggers->AppendTextColumn("Logger", LoggerListModel::LOGGER, wxDATAVIEW_CELL_INERT, -1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
		_loggers->AssociateModel(_loggerModel);
		_manager.AddPane(_loggers, wxAuiPaneInfo().Left().Floatable().Dockable().Caption("Loggers").BestSize(200, -1).Hide());
	}

	// Extra panel
	{
		_extraText = new wxTextCtrl(this, ID_LV_EXTRA_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_AUTO_URL|wxHSCROLL);
		_manager.AddPane(_extraText, wxAuiPaneInfo().Bottom().Floatable().Dockable().Caption("Extra").BestSize(-1, 200).Hide());
	}

	_manager.Update();
}

void Frame::Updated(LogData& data)
{
	if(_status)
	{
		if(data.EntryCount()==0)
		{
			_status->SetStatusText("No entry");
		}
		else if(data.EntryCount()==1)
		{
			_status->SetStatusText("One entry");
		}
		else
		{
			wxString str;
			str << data.EntryCount() << " entries ";
			if(data.GetCriticalityCount(LOG_FATAL)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_FATAL) << " fatal(s) ";
			}
			if(data.GetCriticalityCount(LOG_CRITICAL)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_CRITICAL) << " critical(s) ";
			}
			if(data.GetCriticalityCount(LOG_ERROR)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_ERROR) << " error(s) ";
			}
			if(data.GetCriticalityCount(LOG_WARNING)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_WARNING) << " warning(s) ";
			}
			if(data.GetCriticalityCount(LOG_INFO)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_INFO) << " info(s) ";
			}
			if(data.GetCriticalityCount(LOG_DEBUG)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_DEBUG) << " debug(s) ";
			}
			if(data.GetCriticalityCount(LOG_TRACE)>0)
			{
				str << " - " << data.GetCriticalityCount(LOG_TRACE) << " trace(s) ";
			}
			_status->SetStatusText(str);
		}
	}
	if(_begin)
	{
		_begin->SetValue(data.GetBeginDate());
		_begin->SetDefault(data.GetBeginDate());
	}
	if(_end)
	{
		_end->SetValue(data.GetEndDate());
		_end->SetDefault(data.GetEndDate());
	}
}

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_DATAVIEW_SELECTION_CHANGED(ID_LV_LOGS, Frame::OnLogSelChanged)
	EVT_DATAVIEW_ITEM_ACTIVATED(ID_LV_LOGS, Frame::OnLogActivated)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(ID_LV_LOGS, Frame::OnLogContextMenu)

	EVT_CUSTOM(wxEVT_COMMAND_RIBBONBUTTON_CLICKED, wxID_ANY, Frame::OnRibbonButtonClicked)
	EVT_SLIDER(wxID_ANY, Frame::OnCriticalitySliderEvent)
	EVT_DATE_CHANGED(ID_LV_BEGIN_DATE, Frame::OnBeginDateEvent)
	EVT_DATE_CHANGED(ID_LV_END_DATE, Frame::OnEndDateEvent)
	EVT_MENU(ID_LV_SHOW_EXTRA, Frame::OnDisplayExtra)
	EVT_MENU(ID_LV_SET_BEGIN_DATE, Frame::OnSetAsBegin)
	EVT_MENU(ID_LV_SET_END_DATE, Frame::OnSetAsEnd)
	EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED(ID_LV_LOGGER_PANEL, Frame::OnLoggersExtButtonActivated)
	EVT_DATAVIEW_ITEM_ACTIVATED(ID_LV_LOGGER_LISTBOX, Frame::OnLoggersItemActivated)
	EVT_MENU(ID_LV_SHOW_ALL_LOGGERS, Frame::OnLoggerShowAll)
	EVT_MENU(ID_LV_SHOW_NO_LOGGERS, Frame::OnLoggerShowNone)
	EVT_MENU(ID_LV_SHOW_ONLY_CURRENT_LOGGER, Frame::OnLoggerShowOnlyCurrent)
	EVT_MENU(ID_LV_SHOW_ALL_BUT_CURRENT_LOGGER, Frame::OnLoggerShowAllButCurrent)
	EVT_MENU(ID_LV_FOCUS_PREVIOUS_CURRENT_LOGGER, Frame::OnLoggerFocusPrevious)
	EVT_MENU(ID_LV_FOCUS_NEXT_CURRENT_LOGGER, Frame::OnLoggerFocusNext)
END_EVENT_TABLE()

void Frame::OnLoggersItemActivated(wxDataViewEvent& event)
{
	long logger = _loggerModel->GetLoggerId(event.GetItem());
	_loggerModel->GetData().ToggleLogger(logger);
}

void Frame::OnLoggerShowAll(wxCommandEvent& event)
{
	wxGetApp().GetFilteredLogData().DisplayAllLoggers();
//	UpdateListBoxFromLoggerFilter();
}

void Frame::OnLoggerShowNone(wxCommandEvent& event)
{
	wxGetApp().GetFilteredLogData().HideAllLoggers();
//	UpdateListBoxFromLoggerFilter();
}

void Frame::OnLoggerShowOnlyCurrent(wxCommandEvent& event)
{
	if (_logs->GetSelectedItemsCount() > 0) {
		Entry& entry = _logModel->Get(_logs->GetSelection());
		_loggerModel->GetData().DisplayOnlyLogger(entry.logger);
	}
}

void Frame::OnLoggerShowAllButCurrent(wxCommandEvent& event)
{
	if (_logs->GetSelectedItemsCount() > 0) {
		Entry& entry = _logModel->Get(_logs->GetSelection());
		_loggerModel->GetData().DisplayAllButLogger(entry.logger);
	}

}

void Frame::OnLoggerFocusPrevious(wxCommandEvent& event)
{
	if (_logs->GetSelectedItemsCount() > 0) {
		int pos = _logModel->GetPos(_logs->GetSelection());
		Entry& entry = _logModel->Get(pos);
		int logger = entry.logger;
		while (pos > 0)
		{
			--pos;
			Entry& e = _logModel->Get(pos);
			if (e.logger == logger)
			{
				wxDataViewItem item = _logModel->GetItem(pos);
				_logs->Select(item);
				_logs->EnsureVisible(item);
				break;
			}
		}
	}
}

void Frame::OnLoggerFocusNext(wxCommandEvent& event)
{
	if (_logs->GetSelectedItemsCount() > 0) {
		int pos = _logModel->GetPos(_logs->GetSelection());
		Entry& entry = _logModel->Get(pos);
		int logger = entry.logger;
		while (++pos < (int)_logModel->GetCount())
		{
			Entry& e = _logModel->Get(pos);
			if (e.logger == logger)
			{
				wxDataViewItem item = _logModel->GetItem(pos);
				_logs->Select(item);
				_logs->EnsureVisible(item);
				break;
			}
		}
	}
}

void Frame::OnLoggersExtButtonActivated(wxRibbonPanelEvent& event)
{
	_manager.GetPane(_loggers).Show();
	_manager.Update();
}

void Frame::OnDisplayExtra(wxCommandEvent& event)
{
	_manager.GetPane(_extraText).Show();
	_manager.Update();
}

void Frame::OnSetAsBegin(wxCommandEvent& event)
{
	if(_logs->GetSelectedItemsCount()>0)
	{
		wxDataViewItem sel = _logs->GetSelection();
		size_t row = _logModel->GetRow(sel);
		Entry& entry = _logModel->Get(row);
		_begin->SetValue(entry.date);
		_logModel->GetData().SetStartDate(entry.date);
	}
}

void Frame::OnSetAsEnd(wxCommandEvent& event)
{
	if(_logs->GetSelectedItemsCount()>0)
	{
		wxDataViewItem sel = _logs->GetSelection();
		size_t row = _logModel->GetRow(sel);
		Entry& entry = _logModel->Get(row);
		_end->SetValue(entry.date);
		_logModel->GetData().SetEndDate(entry.date);
	}
}

void Frame::OnLogContextMenu(wxDataViewEvent& event)
{
	if(_logs->GetSelectedItemsCount()>0)
	{
		wxMenu menu;
		menu.Append(ID_LV_SHOW_EXTRA, "Show extra data");
		menu.AppendSeparator();
		menu.Append(ID_LV_SET_BEGIN_DATE, "Set as begin of time frame");
		menu.Append(ID_LV_SET_END_DATE, "Set as end of time frame");
		_logs->PopupMenu(&menu);
	}
}

void Frame::OnLogSelChanged(wxDataViewEvent& /*event*/)
{
	if(_logs->GetSelectedItemsCount()>0)
	{
		wxDataViewItem sel = _logs->GetSelection();
		size_t row = _logModel->GetRow(sel);
		Entry& entry = _logModel->Get(row);
		_extraText->SetValue(entry.extra);
	}
	else
	{
		_extraText->Clear();
	}
}

void Frame::OnLogActivated(wxDataViewEvent& /*event*/)
{
	_manager.GetPane(_extraText).Show();
	_manager.Update();
}

void Frame::OnRibbonButtonClicked(wxEvent/*wxRibbonButtonBarEvent*/& event)
{
    wxRibbonButtonBarEvent *ribbonevent = dynamic_cast<wxRibbonButtonBarEvent*>(&event);

    // Process an equivalent wxEVT_COMMAND_MENU_SELECTED event for simulating menu events.
    // Used to unify RibbonButton click and accelerator entries.

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, event.GetId());
    if(ribbonevent)
        evt.SetInt(ribbonevent->GetInt());
    ProcessEvent(evt);

    event.Skip();
}

void Frame::OnCriticalitySliderEvent(wxCommandEvent& event)
{
	CRITICALITY_LEVEL level = (CRITICALITY_LEVEL)_criticalitySlider->GetValue();
	_criticalityText->SetLabelText(Formatter::FormatCriticality(level));
	wxGetApp().GetFilteredLogData().SetCriticalityFilterLevel(level);
}

void Frame::OnBeginDateEvent(wxDateEvent& event)
{
	wxGetApp().GetFilteredLogData().SetStartDate(event.GetDate());
}

void Frame::OnEndDateEvent(wxDateEvent& event)
{
	wxGetApp().GetFilteredLogData().SetEndDate(event.GetDate());
}
