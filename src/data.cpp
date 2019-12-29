/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* data.cpp
* Copyright (C) 2019 Emilien Kia <Emilien.Kia+dev@gmail.com>
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
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/strconv.h>
#include <wx/convauto.h>
#include <wx/regex.h>

#include "data.hpp"

#include <algorithm>


//
// Formatter
//


const wxString& Formatter::FormatCriticality(CRITICALITY_LEVEL c)
{
	static const wxString criticalities[] = { "UNKNWON", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "FATAL" };
	return criticalities[c];
}

wxString Formatter::FormatDate(const wxDateTime& date)
{
	return date.IsValid()
		? date.FormatISOCombined(' ') << "," << wxString::Format("%03ld", (long)date.GetMillisecond())
		: "";
}

//
// File Descriptor
//

wxString FileDescriptor::StatusToString(FILE_DESC_STATUS status)
{
	static const wxString arr[] = {"", "New", "Reload", "Removed"};
	return arr[status];
}


//
// FileData
//


FileDescriptor& FileData::GetFile(const wxString& file)
{
	for(auto& fd : _fileDescriptors) {
		if(fd.path == file) {
			return fd;
		}
	}
	_fileDescriptors.emplace_back(_fileDescriptors.size(), file);
	return _fileDescriptors.back();
}

FileDescriptor& FileData::GetFile(uint16_t id)
{
	return _fileDescriptors[id];
}

const FileDescriptor& FileData::GetFile(uint16_t id)const 
{
	return _fileDescriptors[id];
}

const FileDescriptor* FileData::FindFile(const wxString& file)const
{
	for(const FileDescriptor& fd : _fileDescriptors) {
		if(fd.path == file) {
			return &fd;
		}
	}
	return nullptr;
}

const FileDescriptor* FileData::FindFile(uint16_t id)const
{
	for(const FileDescriptor& fd : _fileDescriptors) {
		if(fd.id == id) {
			return &fd;
		}
	}
	return nullptr;
}

void FileData::ClearStatistics()
{
	for(FileDescriptor& desc : _fileDescriptors)
	{
		desc.entryCount = 0;
		desc.criticalityCounts = { 0, 0, 0, 0, 0, 0, 0, 0 };
	}
}

//
// Log database
//

LogData::LogData(FileData& fileData):
_fileData(fileData)
{
}

void LogData::Clear()
{
	_entries.clear();
	Synchronize();
}

void LogData::AddLog(const wxDateTime& date, uint16_t file, CRITICALITY_LEVEL criticality, wxString thread, wxString logger, wxString source, wxString message)
{
	AddLog(date, file, criticality,
		_threads.Get(thread.Trim(false).Trim(true)),
		_loggers.Get(logger.Trim(false).Trim(true)),
		_sources.Get(source.Trim(false).Trim(true)),
		message.Trim(false).Trim(true));
}

void LogData::AddLog(const wxDateTime& date, uint16_t file, CRITICALITY_LEVEL criticality, long thread, long logger, long source, const wxString& message)
{
	_entries.push_back({
		date,
		file,
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


void LogData::UpdateStatistics()
{
	_criticalityCounts = { 0, 0, 0, 0, 0, 0, 0, 0 };
	_loggersEntryCount.clear();
	_loggersEntryCount.resize(_loggers.size(), 0);
	_criticalityLoggerCounts.clear();
	_criticalityLoggerCounts.resize(_loggers.size(), { 0, 0, 0, 0, 0, 0, 0, 0 });

	GetFileData().ClearStatistics();

	for (auto& entry : _entries)
	{
		_loggersEntryCount[entry.logger]++;
		_criticalityCounts[entry.criticality]++;
		_criticalityLoggerCounts[entry.logger][entry.criticality]++;

		FileDescriptor& fd = GetFileData().GetFile(entry.file);
		fd.entryCount++;
		fd.criticalityCounts[entry.criticality]++;

		/* TODO, count filtered criticalities by loggers, threads and sources */
	}
}

void LogData::NotifyUpdate()
{
	for (auto listener : _listeners)
	{
		listener->Updated(*this);
	}
}



wxDateTime LogData::GetBeginDate()const
{
	return EntryCount()>0 ? GetEntry(0).date : wxDateTime();
}

wxDateTime LogData::GetEndDate()const
{
	return EntryCount()>0 ? GetEntry(EntryCount() - 1).date : wxDateTime();
}

//
// FilteredLogData
//

FilteredLogData::FilteredLogData(LogData& data) :
	_src(data)
{
	_src.AddListener(this);
}

FilteredLogData::~FilteredLogData()
{
	_src.RemListener(this);
}


wxDateTime FilteredLogData::GetBeginDate()const
{
	return EntryCount()>0 ? GetEntry(0).date : wxDateTime();
}

wxDateTime FilteredLogData::GetEndDate()const
{
	return EntryCount()>0 ? GetEntry(EntryCount() - 1).date : wxDateTime();
}

void FilteredLogData::Updated(LogData & data)
{
	Update();
}

void FilteredLogData::NotifyUpdate()
{
	for (auto listener : _listeners)
	{
		listener->Updated(*this);
	}
}

void FilteredLogData::Update()
{
	_criticalityCounts = { 0, 0, 0, 0, 0, 0, 0, 0 };

	if (_shownLoggers.size() != GetLogData().GetLoggerCount()) {
		// If logger size doesnt match, reactivate alls.
		_shownLoggers.clear();
		_shownLoggers.resize(GetLogData().GetLoggerCount(), true);
	}

	if(_shownFiles.size() != GetFileData().GetFileCount()) {
		// If file count doesnt match, reactivate alls.
		_shownFiles.clear();
		_shownFiles.resize(GetFileData().GetFileCount(), true);
	}

	_data.clear();
	_data.reserve(_src.EntryCount());

	for (size_t n = 0; n<_src.EntryCount(); ++n)
	{
		const Entry& entry = _src.GetEntry(n);
		if (entry.criticality >= _criticality
			&& (!_start.IsValid() || entry.date >= _start)
			&& (!_end.IsValid() || entry.date <= _end)
			&& (_shownLoggers.at(entry.logger)!=false)
			&& (_shownFiles.at(entry.file)!=false)
			)
		{
			_data.push_back(n);
			_criticalityCounts[entry.criticality]++;
		}
	}

	NotifyUpdate();
}

void FilteredLogData::DoSelectAllLoggers()
{
	_shownLoggers.clear();
	_shownLoggers.resize(_src.GetLoggerCount(), true);
}

void FilteredLogData::ClearFilter()
{
	_criticality = CRITICALITY_LEVEL::LOG_INFO;
	_start = _end = wxDateTime();
	DoSelectAllLoggers();
	Update();
}

void FilteredLogData::SetCriticalityFilterLevel(CRITICALITY_LEVEL criticality)
{
	_criticality = criticality;
	Update();
}

void FilteredLogData::SetStartDate(const wxDateTime& date)
{
	_start = date;
	Update();
}

void FilteredLogData::SetEndDate(const wxDateTime& date)
{
	_end = date;
	Update();
}

void FilteredLogData::ResetStartDate()
{
	_start = wxDateTime();
	Update();
}

void FilteredLogData::ResetEndDate()
{
	_end = wxDateTime();
	Update();
}

void FilteredLogData::DisplayAllLoggers()
{
	// TODO optimize it
	_shownLoggers.clear();
	_shownLoggers.resize(GetLogData().GetLoggerCount(), true);
	Update();
}

void FilteredLogData::HideAllLoggers()
{
	// TODO optimize it
	_shownLoggers.clear();
	_shownLoggers.resize(GetLogData().GetLoggerCount(), false);
	Update();
}

void FilteredLogData::DisplayLogger(const wxString& logger, bool display)
{
	DisplayLogger(GetLogData().FindLogger(logger));
}

void FilteredLogData::DisplayLogger(long logger, bool display)
{
	if (logger > 0 && logger < GetLogData().GetLoggerCount()
		&& _shownLoggers.size() > logger) // TODO Review it (shall be implied)
	{
		_shownLoggers[logger] = display;
		Update();
	}
}

void FilteredLogData::DisplayOnlyLogger(long logger)
{
	// TODO optimize it
	_shownLoggers.clear();
	_shownLoggers.resize(GetLogData().GetLoggerCount(), false);
	_shownLoggers[logger] = true;
	Update();
}

void FilteredLogData::DisplayAllButLogger(long logger)
{
	// TODO optimize it
	_shownLoggers.clear();
	_shownLoggers.resize(GetLogData().GetLoggerCount(), true);
	_shownLoggers[logger] = false;
	Update();
}


void FilteredLogData::ToggleLogger(long logger)
{
	if (logger > 0 && logger < GetLogData().GetLoggerCount()
		&& _shownLoggers.size() > logger) // TODO Review it (shall be implied)
	{
		_shownLoggers[logger] = !_shownLoggers[logger];
		Update();
	}
}

bool FilteredLogData::IsLoggerShown(const wxString& logger)const
{
	return IsLoggerShown(GetLogData().FindLogger(logger));
}

bool FilteredLogData::IsLoggerShown(long logger)const
{
	return _shownLoggers[logger];
}




void FilteredLogData::DisplayAllFiles()
{
	// TODO optimize it
	_shownFiles.clear();
	_shownFiles.resize(GetFileData().GetFileCount(), true);
	Update();
}

void FilteredLogData::HideAllFiles()
{
	// TODO optimize it
	_shownFiles.clear();
	_shownFiles.resize(GetFileData().GetFileCount(), false);
	Update();
}

void FilteredLogData::DisplayFile(const wxString& file, bool display)
{
	const FileDescriptor* fd = GetFileData().FindFile(file);
	if(fd!=nullptr)
		DisplayFile(fd->id, display);
}

void FilteredLogData::DisplayFile(uint16_t file, bool display)
{
	if (file < GetFileData().GetFileCount()
		&& _shownFiles.size() > file) // TODO Review it (shall be implied)
	{
		_shownFiles[file] = display;
		Update();
	}

}

void FilteredLogData::ToggleFile(uint16_t file)
{
	if (file < GetFileData().GetFileCount()
		&& _shownFiles.size() > file) // TODO Review it (shall be implied)
	{
		_shownFiles[file] = !_shownFiles[file];
		Update();
	}
}

bool FilteredLogData::IsFileShown(const wxString& file)const
{
	const FileDescriptor* fd = GetFileData().FindFile(file);
	return fd!=nullptr ?  IsFileShown(fd->id) : false;
}

bool FilteredLogData::IsFileShown(uint16_t file)const
{
	return _shownFiles[file];
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
	for (long n = 0; n<size(); ++n)
	{
		if (at(n) == str)
		{
			return n;
		}
	}
	push_back(str);
	return size() - 1;
}

const wxString& wxStringCache::GetString(long id)const
{
	return at(id);
}
