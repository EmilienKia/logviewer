/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * frame.hpp
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

#ifndef _FRAME_HPP_
#define _FRAME_HPP_

#include <wx/frame.h>
#include <wx/aui/framemanager.h>
#include <wx/aui/auibook.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>
#include <wx/log.h>

#include "app.hpp"

class wxSlider;
class wxStaticText;
class wxDateEvent;
class wxDatePickerCtrl;
class wxTimePickerCtrl;


enum {

	ID_LOGVIEWER_CUSTOM = wxID_HIGHEST + 1,

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
};


class DateTimeCtrl : public wxWindow
{
	DECLARE_EVENT_TABLE()
public:
	DateTimeCtrl();
	DateTimeCtrl(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~DateTimeCtrl() = default;
	bool Create(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

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


class Frame: public wxFrame, public LogDatalListener
{
	DECLARE_EVENT_TABLE()
public:
	Frame();
	~Frame();


protected:
	void init();

	virtual void Updated(LogData& data) override;

	void UpdateLoggerFilterFromListBox();

	wxAuiManager _manager;
	wxRibbonBar*  _ribbon;

	LogListModel* _logModel;

	wxDataViewCtrl* _logs;
	wxStatusBar* _status;

	wxSlider*		_criticalitySlider;
	wxStaticText*	_criticalityText;

	DateTimeCtrl*	_begin;
	DateTimeCtrl*	_end;

	wxTextCtrl*	_extraText;
	wxCheckListBox* _loggers;

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
	void OnLoggersListBoxItemChecked(wxCommandEvent& event);
	void OnLoggerShowAll(wxCommandEvent& event);
	void OnLoggerShowNone(wxCommandEvent& event);
};

#endif // _FRAME_HPP_
