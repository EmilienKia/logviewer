/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* data.cpp
* Copyright (C) 2019-2025 Emilien Kia <Emilien.Kia+dev@gmail.com>
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

#include "data.hpp"

#include <algorithm>
#include <fstream>

//
// File Data source
//
std::string_view FileDataSource::GetName() const
{
    return _path;
}

std::string_view FileDataSource::GetData() const
{
    return _data;
}

void FileDataSource::Load() {
    std::ifstream file(_path);
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        _data = buffer.str();
    } else {
        _data.clear();
    }
}

//
// FileData
//

FileDescriptor& FileData::AddFile(const std::string& path) {
    return AddSource(std::make_shared<FileDataSource>(path));
}

FileDescriptor& FileData::AddSource(std::shared_ptr<DataSource> source)
{
    _fileDescriptors.emplace_back(_fileDescriptors.size(), source);
    return _fileDescriptors.back();
}

FileDescriptor& FileData::GetSource(uint16_t id)
{
    return _fileDescriptors[id];
}

const FileDescriptor& FileData::GetSource(uint16_t id)const
{
    return _fileDescriptors[id];
}

const FileDescriptor* FileData::FindSource(const std::string& name)const
{
    for(const FileDescriptor& fd : _fileDescriptors) {
        if(fd.source->GetName() == name) {
            return &fd;
        }
    }
    return nullptr;
}

FileDescriptor* FileData::FindSource(uint16_t id)
{
    for(FileDescriptor& fd : _fileDescriptors) {
        if(fd.id == id) {
            return &fd;
        }
    }
    return nullptr;
}

const FileDescriptor* FileData::FindSource(uint16_t id)const
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
        desc.levelCounts = {0, 0, 0, 0, 0, 0, 0, 0 };
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

void LogData::AddLog(uint16_t file, const ParsedEntry& entry)
{
    AddLog(file,
        entry.date,
        entry.level,
        entry.pid,
        _threads.Get(entry.thread),
        _loggers.Get(entry.logger),
        _sources.Get(entry.source),
        std::string(entry.message)
    );
}

void LogData::AddLog(uint16_t file, const Timestamp& date, LogLevel level, long pid, const std::string& thread, const std::string& logger, const std::string& source, const std::string& message)
{
    AddLog(file, date, level, pid,
        _threads.Get(thread),
        _loggers.Get(logger),
        _sources.Get(source),
        message
    );
}

void LogData::AddLog(uint16_t file, const Timestamp& date, LogLevel level, long pid, long thread, long logger, long source, const std::string& message)
{
    _entries.push_back(LogEntry{
        file,
        date,
        level,
        pid,
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
    std::sort(/*std::execution::par_unseq,*/
    _entries.begin(), _entries.end(),
        [](const LogEntry& a, const LogEntry& b)->bool
    {
        return a.date < b.date;
    });
}

void LogData::SortAndReindexColumns()
{
    auto reindex = [](const std::vector<std::string>& before, const std::vector<std::string>& after) -> std::vector<int>
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

    std::vector<std::string> loggers = _loggers;
    std::sort(/*std::execution::par_unseq,*/ loggers.begin(), loggers.end());
    std::vector<int> reindexLoggers = reindex(_loggers, loggers);

    std::vector<std::string> sources = _sources;
    std::sort(/*std::execution::par_unseq,*/ sources.begin(), sources.end());
    std::vector<int> reindexSources = reindex(_sources, sources);

    std::vector<std::string> threads = _threads;
    std::sort(/*std::execution::par_unseq,*/ threads.begin(), threads.end());
    std::vector<int> reindexThreads = reindex(_threads, threads);

    for (LogEntry& entry : _entries)
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
    _levelCounts = {0, 0, 0, 0, 0, 0, 0, 0 };
    _loggersEntryCount.clear();
    _loggersEntryCount.resize(_loggers.size(), 0);
    _levelLoggerCounts.clear();
    _levelLoggerCounts.resize(_loggers.size(), {0, 0, 0, 0, 0, 0, 0, 0 });

    GetFileData().ClearStatistics();

    for (auto& entry : _entries)
    {
        _loggersEntryCount[entry.logger]++;
        _levelCounts[entry.level]++;
        _levelLoggerCounts[entry.logger][entry.level]++;

        FileDescriptor& fd = GetFileData().GetSource(entry.file);
        fd.entryCount++;
        fd.levelCounts[entry.level]++;

        /* TODO, count filtered levels by loggers, threads and sources */
    }
}

void LogData::NotifyUpdate()
{
    for (auto listener : _listeners)
    {
        listener->Updated(*this);
    }
}

Timestamp LogData::GetBeginDate()const
{
    return EntryCount()>0 ? GetEntry(0).date : std::chrono::system_clock::now();
}

Timestamp LogData::GetEndDate()const
{
    return EntryCount()>0 ? GetEntry(EntryCount() - 1).date : std::chrono::system_clock::now();
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


Timestamp FilteredLogData::GetBeginDate()const
{
    return EntryCount()>0 ? GetEntry(0).date : std::chrono::system_clock::now();
}

Timestamp FilteredLogData::GetEndDate()const
{
    return EntryCount()>0 ? GetEntry(EntryCount() - 1).date : std::chrono::system_clock::now();
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
    _levelCounts = {0, 0, 0, 0, 0, 0, 0, 0 };

    if (_shownLoggers.size() != GetLogData().GetLoggerCount()) {
        // If logger size doesnt match, reactivate alls.
        _shownLoggers.clear();
        _shownLoggers.resize(GetLogData().GetLoggerCount(), true);
    }

    if(_shownFiles.size() != GetFileData().GetSourceCount()) {
        // If file count doesnt match, reactivate alls.
        _shownFiles.clear();
        _shownFiles.resize(GetFileData().GetSourceCount(), true);
    }

    _data.clear();
    _data.reserve(_src.EntryCount());

    for (size_t n = 0; n<_src.EntryCount(); ++n)
    {
        const LogEntry& entry = _src.GetEntry(n);
        if (entry.level >= _criticality
            && (entry.date >= _start)
            && (entry.date <= _end)
            && (_shownLoggers.at(entry.logger)!=false)
            && (_shownFiles.at(entry.file)!=false)
            )
        {
            _data.push_back(n);
            _levelCounts[entry.level]++;
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
    _criticality = LogLevel::LOG_INFO;
    _start = Timestamp::min();
    _end = Timestamp::max();
    DoSelectAllLoggers();
    Update();
}

void FilteredLogData::SetCriticalityFilterLevel(LogLevel criticality)
{
    _criticality = criticality;
    Update();
}

void FilteredLogData::SetStartDate(const Timestamp& date)
{
    _start = date;
    Update();
}

void FilteredLogData::SetEndDate(const Timestamp& date)
{
    _end = date;
    Update();
}

void FilteredLogData::ResetStartDate()
{
    _start = Timestamp::min();
    Update();
}

void FilteredLogData::ResetEndDate()
{
    _end = Timestamp::max();
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

void FilteredLogData::DisplayLogger(const std::string& logger, bool display)
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

bool FilteredLogData::IsLoggerShown(const std::string& logger)const
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
    _shownFiles.resize(GetFileData().GetSourceCount(), true);
    Update();
}

void FilteredLogData::HideAllFiles()
{
    // TODO optimize it
    _shownFiles.clear();
    _shownFiles.resize(GetFileData().GetSourceCount(), false);
    Update();
}

void FilteredLogData::DisplayFile(const std::string& file, bool display)
{
    const FileDescriptor* fd = GetFileData().FindSource(file);
    if(fd!=nullptr)
        DisplayFile(fd->id, display);
}

void FilteredLogData::DisplayFile(uint16_t file, bool display)
{
    if (file < GetFileData().GetSourceCount()
        && _shownFiles.size() > file) // TODO Review it (shall be implied)
    {
        _shownFiles[file] = display;
        Update();
    }

}

void FilteredLogData::ToggleFile(uint16_t file)
{
    if (file < GetFileData().GetSourceCount()
        && _shownFiles.size() > file) // TODO Review it (shall be implied)
    {
        _shownFiles[file] = !_shownFiles[file];
        Update();
    }
}

bool FilteredLogData::IsFileShown(const std::string& file)const
{
    const FileDescriptor* fd = GetFileData().FindSource(file);
    return fd!=nullptr ?  IsFileShown(fd->id) : false;
}

bool FilteredLogData::IsFileShown(uint16_t file)const
{
    return _shownFiles[file];
}


//
// StringCache
//

StringCache::StringCache()
{
    push_back("");
}


std::string_view StringCache::Trim(const std::string_view& str)
{
    size_t first = 0;
    size_t last = str.size();

    // Avance le début
    while (first < last && (str[first] == ' ' || str[first] == '\t')) ++first;
    // Recule la fin
    while (last > first && (str[last - 1] == ' ' || str[last - 1] == '\t')) --last;

    return std::string_view(str.data() + first, last - first);
}


long StringCache::Find(const std::string& str)const
{
    std::string_view trimmed = Trim(str);
    for (long n = 0; n<size(); ++n)
    {
        if (at(n) == trimmed)
        {
            return n;
        }
    }
    return -1;
}

long StringCache::Find(const std::string_view& str)const
{
    std::string_view trimmed = Trim(str);
    for (long n = 0; n<size(); ++n)
    {
        if (at(n) == trimmed)
        {
            return n;
        }
    }
    return -1;
}

long StringCache::Get(const std::string& str)
{
    std::string_view trimmed = Trim(str);
    for (long n = 0; n<size(); ++n)
    {
        if (at(n) == trimmed)
        {
            return n;
        }
    }
    push_back(std::string(trimmed));
    return size() - 1;
}

long StringCache::Get(const std::string_view& str)
{
    std::string_view trimmed = Trim(str);
    for (long n = 0; n<size(); ++n)
    {
        if (at(n) == trimmed)
        {
            return n;
        }
    }
    push_back(std::string(trimmed));
    return size() - 1;
}


const std::string& StringCache::GetString(long id)const
{
    return at(id);
}
