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
	ParseLogFiles(files);
}



//
// Log database
//

LogData::LogData()
{
}

const wxString& LogData::FormatCriticality(CRITICALITY_LEVEL c)
{
	static const wxString criticalities[] = {"UNKNWON", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "FATAL"};
	return criticalities[c];
}

LogData::CRITICALITY_LEVEL LogData::ParseCriticality(const wxString& str)
{
	if(str.IsEmpty())
	{
		return LogData::UNKNWON;
	}
	switch((char)str[0])
	{
	case 'I':
		return LogData::INFO;
	case 'D':
		return LogData::DEBUG;
	case 'T':
		return LogData::TRACE;
	case 'E':
		return LogData::ERROR;
	case 'W':
		return LogData::WARNING;
	case 'C':
		return LogData::CRITICAL;
	case 'F':
		return LogData::FATAL;
	default:
		return LogData::UNKNWON;
	}
}

void LogData::ParseLogFiles(const wxArrayString& paths)
{
	for(auto path : paths)
	{
		ParseLogFile(path);
	}
	UpdateStatistics();
	NotifyUpdate();
}

void LogData::ParseLogFile(const wxString& path)
{
	wxFFileInputStream file(path);
	if(!file.IsOk())
	{
		wxLogError("Cannot open file %s", path);
		return;
	}

	_tempExtra.Empty();

	wxTextInputStream text(file);
	while(!file.Eof())
	{
		ParseLogLine(text.ReadLine());
	}
	AppendExtraLine();

	std::sort(/*std::execution::par_unseq,*/ _entries.begin(), _entries.end(),
		[](const Entry& a, const Entry& b)->bool
		{
			return a.date < b.date;
		});

	UpdateStatistics();
}

wxArrayString LogData::SplitLine(const wxString& line)
{
	wxArrayString arr;
	size_t cur = 0;
	size_t pos = 0;
	while(pos = line.find(" | ", cur), pos != wxString::npos)
	{
		arr.Add(line.SubString(cur, pos-1));
		cur = pos + 3;
		if(cur>=line.Length()){
			break;
		}
	}
	if(cur<line.Length()){
		arr.Add(line.Mid(cur));
	}
	return arr;
}

void LogData::AppendExtraLine()
{
	if(!_tempExtra.IsEmpty())
	{
		if(!_entries.empty())
		{
			_entries.back().extra = _tempExtra;
		}
		_tempExtra.clear();
	}
}

void LogData::AddLogLine(const wxDateTime& date, CRITICALITY_LEVEL criticality, long thread, long logger, long source, const wxString& message)
{
	AppendExtraLine();
	_entries.push_back({
		date,
		criticality,
		thread,
		logger,
		source,
		message
	});
}

void LogData::AddLogLine(wxString date, wxString criticality, wxString thread, wxString logger, wxString source, wxString message)
{
	AddLogLine(
		ParseDate(date.Trim(false).Trim(true)),
		ParseCriticality(criticality.Trim(false)),
		_threads.Get(thread.Trim(false).Trim(true)),
		_loggers.Get(logger.Trim(false).Trim(true)),
		_sources.Get(source.Trim(false).Trim(true)),
		message.Trim(false).Trim(true)
	);
}

void LogData::AddLogLine(wxString date, wxString logger, wxString message)
{
	AddLogLine(
		ParseDate(date.Trim(false).Trim(true)),
		LogData::INFO,
		0,
		_loggers.Get(logger.Trim(false).Trim(true)),
		0,
		message.Trim(false).Trim(true)
	);
}


void LogData::ParseLogLine(const wxString& line)
{
	if(!line.IsEmpty())
	{
		wxArrayString arr = SplitLine(line);
		if(!arr.IsEmpty())
		{
			if(arr[0].Length() < 28 )
			// 28 : arbitrary value greater than any supported text date length
			{
				if(arr.GetCount()==3)
				{
					AddLogLine(arr[0], arr[1], arr[2]);
					return;
				}
				else if(arr.GetCount()==6)
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

wxString LogData::FormatDate(const wxDateTime& date)
{
	return date.IsValid()
		? date.FormatISOCombined(' ') << "," << wxString::Format("%03ld", (long)date.GetMillisecond())
		: "";
}

wxDateTime LogData::ParseDate(const wxString& str)
{
	long y=0, mo=0, d=0, h=0, m=0, s=0, ms=0;
	size_t start, len;

	wxRegEx reDate;;
	if(!reDate.Compile("^(\\d{4})[\\-\\/ ]?(\\d{2})[\\-\\/ ]?(\\d{2})[T\\- ](\\d{2})[\\: ]?(\\d{2})[\\: ]?(\\d{2})([,.](\\d{3}))?$", wxRE_ADVANCED ))
	{
		std::cerr << "RegEx compilation error" << std::endl;
		goto error;
	}
	if(reDate.Matches(str))
	{
		// Grp 1 : year
		if(reDate.GetMatch(&start, &len, 1))
		{
			if(!str.substr(start, len).ToLong(&y)) goto error;
		}
		else
		{
			std::cerr << "Regex not match year" << std::endl;
			goto error;
		}

		// Grp 2 : month
		if(reDate.GetMatch(&start, &len, 2))
		{
			if(!str.substr(start, len).ToLong(&mo)) goto error;
		}
		else
		{
			std::cerr << "Regex not match month" << std::endl;
			goto error;
		}

		// Grp 3 : day
		if(reDate.GetMatch(&start, &len, 3))
		{
			if(!str.substr(start, len).ToLong(&d)) goto error;
		}
		else
		{
			std::cerr << "Regex not match day" << std::endl;
			goto error;
		}


		// Grp 4 : hours
		if(reDate.GetMatch(&start, &len, 4))
		{
			if(!str.substr(start, len).ToLong(&h)) goto error;
		}
		else
		{
			std::cerr << "Regex not match hours" << std::endl;
			goto error;
		}


		// Grp 5 : minutes
		if(reDate.GetMatch(&start, &len, 5))
		{
			if(!str.substr(start, len).ToLong(&m)) goto error;
		}
		else
		{
			std::cerr << "Regex not match minutes" << std::endl;
			goto error;
		}


		// Grp 6 : seconds
		if(reDate.GetMatch(&start, &len, 6))
		{
			if(!str.substr(start, len).ToLong(&s)) goto error;
		}
		else
		{
			std::cerr << "Regex not match seconds" << std::endl;
			goto error;
		}


		// Grp 8 : milliseconds
		if(reDate.GetMatch(&start, &len, 8))
		{
			if(!str.substr(start, len).ToLong(&ms)) goto error;
		}
		else ms = 0;

		return wxDateTime(d, (wxDateTime::Month)(mo-1), y, h, m, s, ms);
	}
	else
	{
		std::cerr << "Regex not match" << std::endl;
	}

error:
	return wxDateTime();
}

void LogData::UpdateStatistics()
{
	if(_entries.empty())
	{
		_beginDate = _endDate = wxDateTime();
	}
	else
	{
		_beginDate = _entries.front().date;
		_endDate   = _entries.back().date;
	}

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
	if(_filter)
	{
		return _ids.size();
	}
	else
	{
		return _logData._entries.size();
	}
}

LogData::Entry& LogListModel::Get(size_t id)const
{
	if(_filter)
	{
		return _logData._entries[_ids[id]];
	}
	else
	{
		return _logData._entries[id];
	}
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
}

bool LogListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
	return false;
}

void LogListModel::ClearFilter()
{
	_filter = false;
	_criticality = LogData::INFO;
	_start = _end = wxDateTime();
	Update();
}

void LogListModel::SetCriticalityFilterLevel(LogData::CRITICALITY_LEVEL criticality)
{
	_filter = true;
	_criticality = criticality;
	Update();
}

void LogListModel::SetStartDate(const wxDateTime& date)
{
	_filter = true;
	_start = date;
	Update();
}

void LogListModel::SetEndDate(const wxDateTime& date)
{
	_filter = true;
	_end = date;
	Update();
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
			&& _loggers.Index(entry.logger) !=  wxNOT_FOUND
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
