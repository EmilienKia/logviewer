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


class Frame;

class LogViewerApp : public wxApp
{
    DECLARE_EVENT_TABLE();
protected:
	Frame * _frame;

	LogData			_data;
	FilteredLogData _filteredData;

public:
	LogViewerApp();

	const LogData& GetLogData() const { return _data; }
	LogData& GetLogData() { return _data; }

	const FilteredLogData& GetFilteredLogData() const { return _filteredData; }
	FilteredLogData& GetFilteredLogData() { return _filteredData; }

protected:
	virtual bool OnInit();

private:
	void OnOpen(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
};

DECLARE_APP(LogViewerApp);




#endif // _APP_HPP_
