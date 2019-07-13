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
	if(!wxWindow::Create(parent, id, pos, size, wxTAB_TRAVERSAL|wxBORDER_NONE|wxCLIP_CHILDREN, "DateTimeCtrl"))
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
	_logModel = new LogListModel(wxGetApp());
	wxGetApp().AddListener(this);

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
			}
			{
				wxRibbonPanel *panel = new wxRibbonPanel(page, wxID_ANY, "Criticality");
				_criticalitySlider = new wxSlider(panel, wxID_ANY, LogData::LOG_INFO, LogData::LOG_UNKNWON, LogData::LOG_FATAL);
				_criticalitySlider->SetMinSize(wxSize(128, -1));
				_criticalityText = new wxStaticText(panel, wxID_ANY, "INFO", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
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
				szr->Add(new wxStaticText(panel, wxID_ANY, "From:"), 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
				szr->Add(_begin, 1, wxEXPAND|wxALL, 2);
				szr->Add(new wxStaticText(panel, wxID_ANY, "To:"), 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
				szr->Add(_end, 1, wxEXPAND|wxALL, 2);
				panel->SetSizer(szr);
			}
			{
				wxRibbonPanel* panel = new wxRibbonPanel(page, ID_LV_LOGGER_PANEL, "Loggers", wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_EXT_BUTTON);
				wxRibbonButtonBar* bar = new wxRibbonButtonBar(panel, wxID_ANY);
				bar->AddButton(ID_LV_SHOW_ALL_LOGGERS, "All loggers", wxRibbonBmp(wxART_TICK_MARK), "Show entries for all loggers");
				bar->AddButton(ID_LV_SHOW_NO_LOGGERS, "No logger", wxRibbonBmp(wxART_CROSS_MARK), "Show entries for no logger (display nothing)");
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
		_logs = new wxDataViewCtrl(this, wxID_ANY);
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
		_loggers = new wxCheckListBox(this, ID_LV_LOGGER_LISTBOX);
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
		if(data.Count()==0)
		{
			_status->SetStatusText("No entry");
		}
		else if(data.Count()==1)
		{
			_status->SetStatusText("One entry");
		}
		else
		{
			wxString str;
			str << data.Count() << " entries ";
			if(data.CriticalityCounts()[LogData::LOG_FATAL]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_FATAL] << " fatal(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_CRITICAL]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_CRITICAL] << " critical(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_ERROR]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_ERROR] << " error(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_WARNING]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_WARNING] << " warning(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_INFO]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_INFO] << " info(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_DEBUG]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_DEBUG] << " debug(s) ";
			}
			if(data.CriticalityCounts()[LogData::LOG_TRACE]>0)
			{
				str << " - " << data.CriticalityCounts()[LogData::LOG_TRACE] << " trace(s) ";
			}
			_status->SetStatusText(str);
		}
	}
	if(_begin)
	{
		_begin->SetValue(data._beginDate);
		_begin->SetDefault(data._beginDate);
	}
	if(_end)
	{
		_end->SetValue(data._endDate);
		_end->SetDefault(data._endDate);
	}
	if(_loggers)
	{
		_loggers->Freeze();
		_loggers->Clear();
		for(const auto& logger : data._loggers)
		{
			_loggers->Append(logger);
		}
		for(unsigned int u=0; u<_loggers->GetCount(); ++u)
		{
			_loggers->Select(u);
		}
		_loggers->Thaw();
	}
}

void Frame::UpdateLoggerFilterFromListBox()
{
	if(_loggers)
	{
		_logModel->GetLoggerArray().Clear();
		_loggers->GetCheckedItems(_logModel->GetLoggerArray());
		_logModel->Update();
	}
}

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, Frame::OnLogSelChanged)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, Frame::OnLogActivated)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, Frame::OnLogContextMenu)
	EVT_CUSTOM(wxEVT_COMMAND_RIBBONBUTTON_CLICKED, wxID_ANY, Frame::OnRibbonButtonClicked)
	EVT_SLIDER(wxID_ANY, Frame::OnCriticalitySliderEvent)
	EVT_DATE_CHANGED(ID_LV_BEGIN_DATE, Frame::OnBeginDateEvent)
	EVT_DATE_CHANGED(ID_LV_END_DATE, Frame::OnEndDateEvent)
	EVT_MENU(ID_LV_SHOW_EXTRA, Frame::OnDisplayExtra)
	EVT_MENU(ID_LV_SET_BEGIN_DATE, Frame::OnSetAsBegin)
	EVT_MENU(ID_LV_SET_END_DATE, Frame::OnSetAsEnd)
	EVT_CHECKLISTBOX(ID_LV_LOGGER_LISTBOX, Frame::OnLoggersListBoxItemChecked)
	EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED(ID_LV_LOGGER_PANEL, Frame::OnLoggersExtButtonActivated)
	EVT_MENU(ID_LV_SHOW_ALL_LOGGERS, Frame::OnLoggerShowAll)
	EVT_MENU(ID_LV_SHOW_NO_LOGGERS, Frame::OnLoggerShowNone)
END_EVENT_TABLE()

void Frame::OnLoggersListBoxItemChecked(wxCommandEvent& event)
{
	UpdateLoggerFilterFromListBox();
}

void Frame::OnLoggerShowAll(wxCommandEvent& event)
{
	if(_loggers)
	{
		for(unsigned int u=0; u<_loggers->GetCount(); ++u)
		{
			_loggers->Check(u, true);
		}
		UpdateLoggerFilterFromListBox();
	}
}

void Frame::OnLoggerShowNone(wxCommandEvent& event)
{
	if(_loggers)
	{
		for(unsigned int u=0; u<_loggers->GetCount(); ++u)
		{
			_loggers->Check(u, false);
		}
		UpdateLoggerFilterFromListBox();
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
		LogData::Entry& entry = _logModel->Get(row);
		_begin->SetValue(entry.date);
		_logModel->SetStartDate(entry.date);
	}
}

void Frame::OnSetAsEnd(wxCommandEvent& event)
{
	if(_logs->GetSelectedItemsCount()>0)
	{
		wxDataViewItem sel = _logs->GetSelection();
		size_t row = _logModel->GetRow(sel);
		LogData::Entry& entry = _logModel->Get(row);
		_end->SetValue(entry.date);
		_logModel->SetEndDate(entry.date);
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
		LogData::Entry& entry = _logModel->Get(row);
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
	LogData::CRITICALITY_LEVEL level = (LogData::CRITICALITY_LEVEL)_criticalitySlider->GetValue();
	_criticalityText->SetLabelText(LogData::FormatCriticality(level));
	_logModel->SetCriticalityFilterLevel(level);
}

void Frame::OnBeginDateEvent(wxDateEvent& event)
{
	_logModel->SetStartDate(event.GetDate());
}

void Frame::OnEndDateEvent(wxDateEvent& event)
{
	_logModel->SetEndDate(event.GetDate());
}
