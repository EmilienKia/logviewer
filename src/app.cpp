/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * app.cpp
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

#include <wx/strconv.h>
#include <wx/convauto.h>
#include <wx/regex.h>

#include <wx/artprov.h>
#include <wx/filedlg.h>

#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include <algorithm>
//#include <execution>

#include "app.hpp"
#include "frame.hpp"


IMPLEMENT_APP(LogViewerApp)

LogViewerApp::LogViewerApp()
{
}

bool LogViewerApp::OnInit()
{
	SetAppName("LogViewer");
	SetAppDisplayName("LogViewer");

	_frame = new Frame();
	_frame->Show(TRUE);
	SetTopWindow(_frame);

	return TRUE;
}


BEGIN_EVENT_TABLE(LogViewerApp, wxApp)
	EVT_MENU(wxID_OPEN, LogViewerApp::OnOpen)
	EVT_MENU(wxID_EXIT, LogViewerApp::OnExit)
END_EVENT_TABLE()

void LogViewerApp::OnExit(wxCommandEvent& event)
{
	_frame->Close();
}

void LogViewerApp::OnOpen(wxCommandEvent& event)
{
	wxFileDialog fd(_frame, _("Open log files"), "", "", "Log files (*.log;*.log.*)|*.log;*.log.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE);
	if(fd.ShowModal() == wxID_CANCEL)
		return;

	wxArrayString files;
	fd.GetPaths(files);

	Parser parser(*this);
	parser.ParseLogFiles(files);
}


//
// Log parser
//

void Parser::ParseLogFiles(const wxArrayString& paths)
{
	for (auto path : paths)
	{
		ParseLogFile(path);
	}

	_data.Synchronize();
}

void Parser::ParseLogFile(const wxString& path)
{
	wxFFileInputStream file(path);
	if (!file.IsOk())
	{
		wxLogError("Cannot open file %s", path);
		return;
	}

	_tempExtra.Empty();

	wxTextInputStream text(file);
	while (!file.Eof())
	{
		ParseLogLine(text.ReadLine());
	}
	AppendExtraLine();
}


void Parser::ParseLogLine(const wxString& line)
{
	if (!line.IsEmpty())
	{
		wxArrayString arr = SplitLine(line);
		if (!arr.IsEmpty())
		{
			if (arr[0].Length() < 28)
				// 28 : arbitrary value greater than any supported text date length
			{
				if (arr.GetCount() == 3)
				{
					AddLogLine(arr[0], arr[1], arr[2]);
					return;
				}
				else if (arr.GetCount() == 6)
				{
					AddLogLine(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
					return;
				}
			}
			// Consider as extra line
			_tempExtra.Append(line).Append("\n");
		}
	}
}

wxArrayString Parser::SplitLine(const wxString& line)
{
	wxArrayString arr;
	size_t cur = 0;
	size_t pos = 0;
	while (pos = line.find(" | ", cur), pos != wxString::npos)
	{
		arr.Add(line.SubString(cur, pos - 1));
		cur = pos + 3;
		if (cur >= line.Length()) {
			break;
		}
	}
	if (cur<line.Length()) {
		arr.Add(line.Mid(cur));
	}
	return arr;
}

void Parser::AppendExtraLine()
{
	if (!_tempExtra.IsEmpty())
	{
		if (!_data._entries.empty())
		{
			_data._entries.back().extra = _tempExtra;
		}
		_tempExtra.clear();
	}
}

void Parser::AddLogLine(wxString date, wxString criticality, wxString thread, wxString logger, wxString source, wxString message)
{
	AppendExtraLine();
	_data.AddLog(
		ParseDate(date.Trim(false).Trim(true)),
		ParseCriticality(criticality.Trim(false)),
		thread,
		logger,
		source,
		message
	);
}

void Parser::AddLogLine(wxString date, wxString logger, wxString message)
{
	AppendExtraLine();
	_data.AddLog(
		ParseDate(date.Trim(false).Trim(true)),
		CRITICALITY_LEVEL::LOG_INFO,
		"",
		logger.Trim(false).Trim(true),
		"",
		message.Trim(false).Trim(true)
	);
}


wxDateTime Parser::ParseDate(const wxString& str)
{
	long y = 0, mo = 0, d = 0, h = 0, m = 0, s = 0, ms = 0;
	size_t start, len;

	wxRegEx reDate;;
	if (!reDate.Compile("^(\\d{4})[\\-\\/ ]?(\\d{2})[\\-\\/ ]?(\\d{2})[T\\- ](\\d{2})[\\: ]?(\\d{2})[\\: ]?(\\d{2})([,.](\\d{3}))?$", wxRE_ADVANCED))
	{
		std::cerr << "RegEx compilation error" << std::endl;
		goto error;
	}
	if (reDate.Matches(str))
	{
		// Grp 1 : year
		if (reDate.GetMatch(&start, &len, 1))
		{
			if (!str.substr(start, len).ToLong(&y)) goto error;
		}
		else
		{
			std::cerr << "Regex not match year" << std::endl;
			goto error;
		}

		// Grp 2 : month
		if (reDate.GetMatch(&start, &len, 2))
		{
			if (!str.substr(start, len).ToLong(&mo)) goto error;
		}
		else
		{
			std::cerr << "Regex not match month" << std::endl;
			goto error;
		}

		// Grp 3 : day
		if (reDate.GetMatch(&start, &len, 3))
		{
			if (!str.substr(start, len).ToLong(&d)) goto error;
		}
		else
		{
			std::cerr << "Regex not match day" << std::endl;
			goto error;
		}


		// Grp 4 : hours
		if (reDate.GetMatch(&start, &len, 4))
		{
			if (!str.substr(start, len).ToLong(&h)) goto error;
		}
		else
		{
			std::cerr << "Regex not match hours" << std::endl;
			goto error;
		}


		// Grp 5 : minutes
		if (reDate.GetMatch(&start, &len, 5))
		{
			if (!str.substr(start, len).ToLong(&m)) goto error;
		}
		else
		{
			std::cerr << "Regex not match minutes" << std::endl;
			goto error;
		}


		// Grp 6 : seconds
		if (reDate.GetMatch(&start, &len, 6))
		{
			if (!str.substr(start, len).ToLong(&s)) goto error;
		}
		else
		{
			std::cerr << "Regex not match seconds" << std::endl;
			goto error;
		}


		// Grp 8 : milliseconds
		if (reDate.GetMatch(&start, &len, 8))
		{
			if (!str.substr(start, len).ToLong(&ms)) goto error;
		}
		else ms = 0;

		return wxDateTime(d, (wxDateTime::Month)(mo - 1), y, h, m, s, ms);
	}
	else
	{
		std::cerr << "Regex not match" << std::endl;
	}

error:
	return wxDateTime();
}

CRITICALITY_LEVEL Parser::ParseCriticality(const wxString& str)
{
	if (str.IsEmpty())
	{
		return CRITICALITY_LEVEL::LOG_UNKNWON;
	}
	switch ((char)str[0])
	{
	case 'I':
		return CRITICALITY_LEVEL::LOG_INFO;
	case 'D':
		return CRITICALITY_LEVEL::LOG_DEBUG;
	case 'T':
		return CRITICALITY_LEVEL::LOG_TRACE;
	case 'E':
		return CRITICALITY_LEVEL::LOG_ERROR;
	case 'W':
		return CRITICALITY_LEVEL::LOG_WARNING;
	case 'C':
		return CRITICALITY_LEVEL::LOG_CRITICAL;
	case 'F':
		return CRITICALITY_LEVEL::LOG_FATAL;
	default:
		return CRITICALITY_LEVEL::LOG_UNKNWON;
	}
}


//
// Log database
//

LogData::LogData()
{
}

void LogData::AddLog(const wxDateTime& date, CRITICALITY_LEVEL criticality, wxString thread, wxString logger, wxString source, wxString message)
{
	AddLog(date, criticality,
		_threads.Get(thread.Trim(false).Trim(true)),
		_loggers.Get(logger.Trim(false).Trim(true)),
		_sources.Get(source.Trim(false).Trim(true)),
		message.Trim(false).Trim(true));
}

void LogData::AddLog(const wxDateTime& date, CRITICALITY_LEVEL criticality, long thread, long logger, long source, const wxString& message)
{
	_entries.push_back({
		date,
		criticality,
		thread,
		logger,
		source,
		message
		});
}

void LogData::Synchronize()
{
	SortLogsByDate();
	SortAndReindexColumns();
	UpdateStatistics();
	NotifyUpdate();
}

const wxString& LogData::FormatCriticality(CRITICALITY_LEVEL c)
{
	static const wxString criticalities[] = {"UNKNWON", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "FATAL"};
	return criticalities[c];
}

void LogData::SortLogsByDate()
{
	std::sort(/*std::execution::par_unseq,*/ _entries.begin(), _entries.end(),
		[](const Entry& a, const Entry& b)->bool
	{
		return a.date < b.date;
	});
}

void LogData::SortAndReindexColumns()
{
	auto reindex = [](const std::vector<wxString>& before, const std::vector<wxString>& after) ->std::vector<int>
	{
		std::vector<int> reindex;
		for (int i = 0; i < before.size(); ++i)
		{
			for (int pos = 0; pos < after.size(); ++pos)
			{
				if (before[i] == after[pos])
				{
					reindex.push_back(pos);
					break;
				}
			}
		}
		return reindex;
	};

	std::vector<wxString> loggers = _loggers;
	std::sort(/*std::execution::par_unseq,*/ loggers.begin(), loggers.end());
	std::vector<int> reindexLoggers = reindex(_loggers, loggers);

	std::vector<wxString> sources = _sources;
	std::sort(/*std::execution::par_unseq,*/ sources.begin(), sources.end());
	std::vector<int> reindexSources = reindex(_sources, sources);

	std::vector<wxString> threads = _threads;
	std::sort(/*std::execution::par_unseq,*/ threads.begin(), threads.end());
	std::vector<int> reindexThreads = reindex(_threads, threads);

	for (Entry& entry : _entries)
	{
		entry.logger = reindexLoggers[entry.logger];
		entry.source = reindexSources[entry.source];
		entry.thread = reindexThreads[entry.thread];
	}

	std::swap(_loggers, loggers);
	std::swap(_sources, sources);
	std::swap(_threads, threads);
}



wxString LogData::FormatDate(const wxDateTime& date)
{
	return date.IsValid()
		? date.FormatISOCombined(' ') << "," << wxString::Format("%03ld", (long)date.GetMillisecond())
		: "";
}


void LogData::UpdateStatistics()
{
	_criticalityCounts = {0, 0, 0, 0, 0, 0, 0, 0};

	for(auto& entry : _entries)
	{
		_criticalityCounts[entry.criticality]++;
	}
}

void LogData::NotifyUpdate()
{
	for(auto listener : _listeners)
	{
		listener->Updated(*this);
	}
}



wxDateTime LogData::GetBeginDate()const
{
	return !_entries.empty() ? _entries.front().date : wxDateTime() ;
}

wxDateTime LogData::GetEndDate()const
{
	return !_entries.empty() ? _entries.back().date : wxDateTime();
}





//
// Model
//

LogListModel::LogListModel(LogData& logData):
_logData(logData)
{
	logData.AddListener(this);
}

size_t LogListModel::Count()const
{
	return _ids.size();
}

LogData::Entry& LogListModel::Get(size_t id)const
{
	return _logData._entries[_ids[id]];
}

LogData::Entry& LogListModel::Get(wxDataViewItem item)const
{
	return Get(GetRow(item));
}

unsigned int LogListModel::GetPos(wxDataViewItem item)const
{
	return GetRow(item);
}

void LogListModel::Updated(LogData& data)
{
	Update();
}

unsigned int LogListModel::GetColumnCount()const
{
	return LogListModel::COLUMN_COUNT;
}

wxString LogListModel::GetColumnType(unsigned int col)const
{
	if(col==LogListModel::EXTRA)
		return "bool";
	else
		return "string";
}

void LogListModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
	auto& entry = Get(row);
	switch(col)
	{
		case LogListModel::DATE:
			variant = LogData::FormatDate(entry.date);
			return;
		case LogListModel::CRITICALITY:
			variant = LogData::FormatCriticality(entry.criticality);
			return;
		case LogListModel::THREAD:
			variant = _logData._threads.GetString(entry.thread);
			return;
		case LogListModel::LOGGER:
			variant = _logData._loggers.GetString(entry.logger);
			return;
		case LogListModel::SOURCE:
			variant = _logData._sources.GetString(entry.source);
			return;
		case LogListModel::MESSAGE:
			variant = entry.message;
			return;
		case LogListModel::EXTRA:
			variant = !entry.extra.IsEmpty();
			return;
		default:
			return;
	}
}

bool LogListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr )const
{
	// TODO
	return false;
}

bool LogListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
	return false;
}

void LogListModel::ClearFilter()
{
	_criticality = CRITICALITY_LEVEL::LOG_INFO;
	_start = _end = wxDateTime();
	Update();
}

void LogListModel::SetCriticalityFilterLevel(CRITICALITY_LEVEL criticality)
{
	_criticality = criticality;
	Update();
}

void LogListModel::SetStartDate(const wxDateTime& date)
{
	_start = date;
	Update();
}

void LogListModel::SetEndDate(const wxDateTime& date)
{
	_end = date;
	Update();
}

void LogListModel::ResetStartDate()
{
	_start = wxDateTime();
	Update();
}

void LogListModel::ResetEndDate()
{
	_end = wxDateTime();
	Update();
}

void LogListModel::DisplayAllLoggers()
{
	_displayedLoggers.clear();
	for (int i = 0; i < _logData._loggers.size(); i++)
	{
		_displayedLoggers.push_back(i);
	}
	Update();
}

void LogListModel::HideAllLoggers()
{
	_displayedLoggers.clear();
	Update();
}

void LogListModel::DisplayLogger(const wxString& logger, bool display)
{
	long logid = _logData._loggers.Find(logger);
	DisplayLogger(logid, display);
}

void LogListModel::DisplayLogger(int logger, bool display)
{
	if (logger != wxNOT_FOUND)
	{
		if (display)
		{
			if (_displayedLoggers.Index(logger) == wxNOT_FOUND)
			{
				_displayedLoggers.Add(logger);
				Update();
			}
		}
		else /* hide */
		{
			if (_displayedLoggers.Index(logger) != wxNOT_FOUND)
			{
				_displayedLoggers.Remove(logger);
				Update();
			}
		}
	}
}


void LogListModel::Update()
{
	_ids.clear();
	_ids.reserve(_logData._entries.size());

	for(size_t n=0; n<_logData._entries.size(); ++n)
	{
		LogData::Entry& entry = _logData._entries[n];
		if(		entry.criticality >= _criticality
			&& (!_start.IsValid() || entry.date >= _start )
			&& (!_end.IsValid() || entry.date <= _end )
			&& _displayedLoggers.Index(entry.logger) !=  wxNOT_FOUND
		)
		{
			_ids.push_back(n);
		}
	}

	Reset(Count());
}



//
// wxStringCache
//

wxStringCache::wxStringCache()
{
	push_back("");
}

long wxStringCache::Find(const wxString& str)const
{
	for (long n = 0; n<size(); ++n)
	{
		if (at(n) == str)
		{
			return n;
		}
	}
	return wxNOT_FOUND;
}

long wxStringCache::Get(const wxString& str)
{
	for(long n=0; n<size(); ++n)
	{
		if(at(n) == str)
		{
			return n;
		}
	}
	push_back(str);
	return size()-1;
}

wxString wxStringCache::GetString(long id)const
{
	return at(id);
}
