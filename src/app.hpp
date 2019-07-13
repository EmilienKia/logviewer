/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * app.hpp
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

#ifndef _APP_HPP_
#define _APP_HPP_

#include <array>
#include <vector>
#include <set>

#include <wx/arrstr.h>
#include <wx/dataview.h>

class Frame;

struct LogData;
class LogListModel;


inline wxString str2wx(const std::string& str)
{
    return !str.empty() ? wxString(str.data(), wxConvUTF8) : wxString();
}

inline std::string wx2str(const wxString str)
{
    return std::string(str.utf8_str());
}


class wxStringCache : public std::vector<wxString>
{
public:
	wxStringCache();

	long Get(const wxString& str);
	wxString GetString(long id)const;
};

struct LogDatalListener
{
	virtual void Updated(LogData& data) =0;
};

struct LogData
{
	enum CRITICALITY_LEVEL
	{
		LOG_UNKNWON,
		LOG_TRACE,
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
		LOG_CRITICAL,
		LOG_FATAL,

		LOG_CRITICALITY_COUNT
	};

	struct Entry
	{
		wxDateTime date;
		CRITICALITY_LEVEL criticality;
		long thread;
		long logger;
		long source;
		wxString message;
		wxString extra;
	};

	LogData();

	void ParseLogFiles(const wxArrayString& paths);
	void ParseLogFile(const wxString& path);
	void ParseLogLine(const wxString& line);

	void AddLogLine(const wxDateTime& date, CRITICALITY_LEVEL criticality, long thread, long logger, long source, const wxString& message);

	void AddLogLine(wxString date, wxString logger, wxString message);
	void AddLogLine(wxString date, wxString criticality, wxString thread, wxString logger, wxString source, wxString message);

	void AppendExtraLine();

	void UpdateStatistics();

	static wxArrayString SplitLine(const wxString& line);

	static const wxString& FormatCriticality(CRITICALITY_LEVEL c);
	static CRITICALITY_LEVEL ParseCriticality(const wxString& str);

	static wxString FormatDate(const wxDateTime& date);
	static wxDateTime ParseDate(const wxString& str);




	size_t Count()const {return _entries.size();}
	const std::array<size_t,LOG_CRITICALITY_COUNT>& CriticalityCounts()const {return _criticalityCounts;}
	const wxDateTime& BeginDate()const {return _beginDate;}
	const wxDateTime& EndDate()const {return _endDate;}

	void AddListener(LogDatalListener* listener) {_listeners.insert(listener);}
	void RemListener(LogDatalListener* listener) {_listeners.erase(listener);}

	wxStringCache _threads, _loggers, _sources;
	std::vector<Entry> _entries;

	std::array<size_t,LOG_CRITICALITY_COUNT> _criticalityCounts;
	wxDateTime _beginDate, _endDate;

	wxString _tempExtra;

	std::set<LogDatalListener*> _listeners;
	void NotifyUpdate();
};




class LogListModel : public wxDataViewVirtualListModel, protected LogDatalListener
{
public:
	LogListModel(LogData& logData);

	// DVVLM definitions:
	virtual unsigned int GetColumnCount()const;
	virtual wxString GetColumnType(unsigned int col)const;

	virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
	virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);
	virtual bool GetAttrByRow( unsigned int row, unsigned int col, wxDataViewItemAttr &attr )const;

	// Model definition
	enum LogListModelColumns {
		DATE,
		CRITICALITY,
		THREAD,
		LOGGER,
		SOURCE,
		MESSAGE,
		EXTRA,

		COLUMN_COUNT
	};

	size_t Count()const;
	LogData::Entry& Get(size_t id)const;

	void ClearFilter();
	void SetCriticalityFilterLevel(LogData::CRITICALITY_LEVEL criticality);
	void SetStartDate(const wxDateTime& date);
	void SetEndDate(const wxDateTime& date);
	void ResetStartDate();
	void ResetEndDate();

	wxArrayInt& GetLoggerArray() {return _loggers;}

	void Update();

protected:
	void Updated(LogData& data);
	LogData& _logData;

	bool _filter = false;
	LogData::CRITICALITY_LEVEL _criticality = LogData::LOG_INFO;
	wxDateTime _start, _end;
	wxArrayInt _loggers;
	std::vector<size_t> _ids;
};








class LogViewerApp : public wxApp, public LogData
{
    DECLARE_EVENT_TABLE();

public:
	LogViewerApp();

	virtual bool OnInit();

	void LoadFile(const wxString& path);

protected:
	Frame* _frame;

private:
	void OnOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
};

DECLARE_APP(LogViewerApp);




#endif // _APP_HPP_
