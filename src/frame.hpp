/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * frame.hpp
 * Copyright (C) 2018-2019 Emilien Kia <Emilien.Kia+dev@gmail.com>
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

#ifndef _FRAME_HPP_
#define _FRAME_HPP_

#include <wx/frame.h>
#include <wx/aui/framemanager.h>
#include <wx/aui/auibook.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/ribbon/toolbar.h>
#include <wx/log.h>
#include <wx/srchctrl.h>

#include "app.hpp"

class wxSlider;
class wxStaticText;
class wxDateEvent;
class wxDatePickerCtrl;
class wxTimePickerCtrl;


enum {

	ID_LOGVIEWER_CUSTOM = wxID_HIGHEST + 1,

	ID_LV_LOGS,

	ID_LV_BEGIN_DATE,
	ID_LV_END_DATE,

	ID_LV_SHOW_EXTRA,
	ID_LV_EXTRA_TEXT,
	ID_LV_SET_BEGIN_DATE,
	ID_LV_SET_END_DATE,

	ID_LV_LOGGER_PANEL,
	ID_LV_LOGGER_LISTBOX,
	ID_LV_SHOW_ALL_LOGGERS,
	ID_LV_SHOW_NO_LOGGERS,
	ID_LV_SHOW_ONLY_CURRENT_LOGGER,
	ID_LV_SHOW_ALL_BUT_CURRENT_LOGGER,
	ID_LV_FOCUS_PREVIOUS_CURRENT_LOGGER,
	ID_LV_FOCUS_NEXT_CURRENT_LOGGER,

	ID_LV_SEARCH_PANEL,
	ID_LV_SEARCH_CTRL,
	ID_LV_SEARCH_CTRL_FOCUS,
	ID_LV_SEARCH_DIRECTION_ASC,
	ID_LV_SEARCH_DIRECTION_DESC,
	ID_LV_SEARCH_CYCLE,
	ID_LV_SEARCH_CASE_SENSITIVE,
	ID_LV_SEARCH_ESCAPE,
	ID_LV_SEARCH_REGEX,
	ID_LV_SEARCH_NEXT,
	ID_LV_SEARCH_PREV,

	ID_LV_FILES_PANEL,
	ID_LV_FILES_LISTBOX
};


class DateTimeCtrl : public wxWindow
{
	DECLARE_EVENT_TABLE()
public:
	DateTimeCtrl();
	DateTimeCtrl(wxWindow *parent, wxWindowID id, const wxBitmap &revert_bitmap, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~DateTimeCtrl() = default;
	bool Create(wxWindow *parent, wxWindowID id, const wxBitmap &revert_bitmap, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

	wxDateTime GetValue()const {return _curr;}
	void SetValue(const wxDateTime& date);

	void SetDefault(const wxDateTime& date);

protected:
	wxDateTime _curr, _def;

	wxDatePickerCtrl* _date;
	wxTimePickerCtrl* _time;

	void OnDateEvent(wxDateEvent& event);
	void OnRevertEvent(wxCommandEvent& event);
	void SendNotification();
};


class Frame: public wxFrame, public LogData::Listener
{
	DECLARE_EVENT_TABLE()
public:
	Frame();
	~Frame();

	void Search(bool dirNext = true);

	void SearchNext(){Search(true);}
	void SearchPrev(){Search(false);}

protected:
	void init();

	virtual void Updated(LogData& data) override;

	//void UpdateLoggerFilterFromListBox();
	void UpdateListBoxFromLoggerFilter();

	wxAuiManager _manager;
	wxRibbonBar*  _ribbon;

	LogListModel* _logModel;
	LoggerListModel* _loggerModel;
	FileListModel* _fileModel;

	wxDataViewCtrl* _logs;
	wxDataViewCtrl* _loggers;
	wxDataViewCtrl* _files;
	wxStatusBar* _status;

	wxSlider*		_criticalitySlider;
	wxStaticText*	_criticalityText;

	DateTimeCtrl*	_begin;
	DateTimeCtrl*	_end;

	wxTextCtrl*	_extraText;

	wxSearchCtrl* _search;
	bool _searchDir = true;
	bool _searchCycle = true;
	bool _searchCaseSensitive = false;
	bool _searchEscape = false;
	bool _searchRegex = false;

private:
	void OnRibbonButtonClicked(wxEvent/*wxRibbonButtonBarEvent*/& event);

	void OnCriticalitySliderEvent(wxCommandEvent& event);

	void OnLogSelChanged(wxDataViewEvent& event);
	void OnLogActivated(wxDataViewEvent& event);
	void OnLogContextMenu(wxDataViewEvent& event);

	void OnBeginDateEvent(wxDateEvent& event);
	void OnEndDateEvent(wxDateEvent& event);

	void OnDisplayExtra(wxCommandEvent& event);
	void OnSetAsBegin(wxCommandEvent& event);
	void OnSetAsEnd(wxCommandEvent& event);

	void OnLoggersExtButtonActivated(wxRibbonPanelEvent& event);
	void OnLoggersItemActivated(wxDataViewEvent& event);
	void OnLoggerShowAll(wxCommandEvent& event);
	void OnLoggerShowNone(wxCommandEvent& event);
	void OnLoggerShowOnlyCurrent(wxCommandEvent& event);
	void OnLoggerShowAllButCurrent(wxCommandEvent& event);
	void OnLoggerFocusPrevious(wxCommandEvent& event);
	void OnLoggerFocusNext(wxCommandEvent& event);

	void OnSearch(wxCommandEvent& event);
	void OnSearchAscent(wxRibbonToolBarEvent& event);
	void OnSearchDescent(wxRibbonToolBarEvent& event);
	void OnSearchAscentUpdate(wxUpdateUIEvent& event);
	void OnSearchDescentUpdate(wxUpdateUIEvent& event);
	void OnSearchCycle(wxRibbonToolBarEvent& event);
	void OnSearchCycleUpdate(wxUpdateUIEvent& event);
	void OnSearchCaseSensitive(wxRibbonToolBarEvent& event);
	void OnSearchCaseSensitiveUpdate(wxUpdateUIEvent& event);
	void OnSearchEscape(wxRibbonToolBarEvent& event);
	void OnSearchEscapeUpdate(wxUpdateUIEvent& event);
	void OnSearchRegex(wxRibbonToolBarEvent& event);
	void OnSearchRegexUpdate(wxUpdateUIEvent& event);
	void OnSearchCtrlFocus(wxCommandEvent& event);
	void OnSearchNext(wxCommandEvent& event);
	void OnSearchPrev(wxCommandEvent& event);

	void OnFilesExtButtonActivated(wxRibbonPanelEvent& event);
	void OnFilesItemActivated(wxDataViewEvent& event);

};

#endif // _FRAME_HPP_
