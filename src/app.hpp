/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * app.hpp
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

#ifndef _APP_HPP_
#define _APP_HPP_


#include "data.hpp"
#include "model.hpp"



enum {

	ID_LOGVIEWER_CUSTOM = wxID_HIGHEST + 1,

	ID_LV_FILE_MANAGE,

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
	ID_LV_FILES_LISTBOX,

	ID_LV_FILEBOX_FILE_DVCTRL
};


class Frame;

class LogViewerApp : public wxApp
{
    DECLARE_EVENT_TABLE();
protected:
	Frame * _frame;

	FileData		_files;
	LogData			_data;
	FilteredLogData _filteredData;

public:
	LogViewerApp();

	const FileData& GetFileData() const { return _files; }
	FileData& GetFileData() { return _files; }

	const LogData& GetLogData() const { return _data; }
	LogData& GetLogData() { return _data; }

	const FilteredLogData& GetFilteredLogData() const { return _filteredData; }
	FilteredLogData& GetFilteredLogData() { return _filteredData; }

	void OpenFiles(const wxArrayString& files);

	int OpenFileDialog(wxWindow* parent, wxArrayString& paths);

	void FileManagement();

protected:
	virtual bool OnInit();

	void ApplyUpdates();

	void CancelUpdates();

private:
	void OnOpen(wxCommandEvent& event);
	void OnManage(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
};

DECLARE_APP(LogViewerApp);




#endif // _APP_HPP_
