/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* data.hpp
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

#ifndef _DATA_HPP_
#define _DATA_HPP_

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <cstdint>
#include <chrono>
#include <optional>

class LogData;

typedef std::chrono::system_clock::time_point Timestamp;

enum LogLevel
{
    LOG_UNKNOWN,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL,
    LOG_FATAL,

    LOG_LEVEL_COUNT
};

struct ParsedEntry
{
    Timestamp date;
    LogLevel level = LogLevel::LOG_UNKNOWN;
    long pid = 0;
    std::string_view thread;
    std::string_view logger;
    std::string_view source;
    std::string_view message;
// TODO Support extra line data
    std::optional<std::string> extra;
};

class DataSource
{
public:
    virtual ~DataSource() = default;

    virtual std::string_view GetName() const = 0;
    virtual std::string_view GetData() const = 0;

    virtual void Load() =0;
};

class FileDataSource : public DataSource
{
protected:
    std::string _path;
    std::string _data;
public:
    FileDataSource(const std::string& path) : _path(path) {}

    std::string_view GetName() const override;
    std::string_view GetData() const override;

    void Load() override;
};


struct FileDescriptor
{
    FileDescriptor(): id(0) {}

    FileDescriptor(uint16_t id, std::shared_ptr<DataSource> source): id(id), source(source) {}
    FileDescriptor(const FileDescriptor& fd) = default;
    FileDescriptor(FileDescriptor&& fd) = default;

    FileDescriptor& operator=(const FileDescriptor&) = default;
    FileDescriptor& operator=(FileDescriptor&&) = default;

    uint16_t id;
    std::shared_ptr<DataSource> source;

    enum LOG_FORMAT {
        LOG_FORMAT_SPRING_BOOT, // Spring Boot format
        LOG_FORMAT_LOG4J, // Log4J format
        LOG_FORMAT_CUSTOM, // Custom format
        LOG_FORMAT_DEFAULT = LOG_FORMAT_SPRING_BOOT
    } logFormat = LOG_FORMAT_DEFAULT;
    std::string logRegex;

    enum LOG_DATE {
        DATE_FORMAT_FULL_ISO8601, // Full ISO 8601 format
        DATE_FORMAT_CUSTOM,
        DATE_FORMAT_DEFAULT = DATE_FORMAT_FULL_ISO8601,
    } dateFormat = DATE_FORMAT_DEFAULT;
    std::string dateRegex;

    enum FILE_DESC_STATUS
    {
        FILE_LOADED,	// The file has already been loaded
        FILE_NEW,		// The file is new, never loaded
        FILE_RELOAD,	// The file is to Load, have already been loaded.
        FILE_REMOVED	// The file will be removed
    } status = FILE_NEW;

    size_t entryCount = 0;
    std::array<size_t, LOG_LEVEL_COUNT> levelCounts{0, 0, 0, 0, 0, 0, 0, 0};
};

class FileData
{
public:
    struct Listener
    {
        virtual void Updated(FileData& data) = 0;
    };

    typedef std::vector<FileDescriptor>::iterator iterator;
    typedef std::vector<FileDescriptor>::const_iterator const_iterator;

protected:
    std::vector<FileDescriptor> _fileDescriptors;

private:
    std::set<Listener*> _fileListeners;

public:
    FileData() = default;
    FileData(const FileData&) = default;

    FileDescriptor& AddFile(const std::string& path);
    FileDescriptor& AddSource(std::shared_ptr<DataSource> source);

    size_t GetSourceCount()const {return _fileDescriptors.size();}

    FileDescriptor& GetSource(uint16_t id);
    const FileDescriptor& GetSource(uint16_t id)const;

    const FileDescriptor* FindSource(const std::string& name)const;
    FileDescriptor* FindSource(uint16_t id);
    const FileDescriptor* FindSource(uint16_t id)const;

    long GetSourceEntryCount(uint16_t fileid) const {return GetSource(fileid).entryCount; }
    long GetSourceLogLevelEntryCount(uint16_t fileid, LogLevel criticality) const {return GetSource(fileid).levelCounts[criticality]; }

    iterator begin() {return _fileDescriptors.begin();}
    const_iterator begin()const {return _fileDescriptors.begin();}
    iterator end() {return _fileDescriptors.end();}
    const_iterator end()const {return _fileDescriptors.end();}

    template<typename Pred>
    void RemoveSourceIf(Pred pred) {
        _fileDescriptors.erase(std::remove_if(_fileDescriptors.begin(), _fileDescriptors.end(), pred), _fileDescriptors.end());
    }

    // @name Stats
    // @{
    void ClearStatistics();
    // @}

    // @name Listener management
    // @{
    void AddListener(Listener* listener) { _fileListeners.insert(listener); }
    void RemListener(Listener* listener) { _fileListeners.erase(listener); }
    //

protected:
    void notifyUpdated() {
        for(Listener* listener : _fileListeners)
            listener->Updated(*this);
    }
};



struct LogEntry {
    uint16_t file = 0;
    Timestamp date;
    LogLevel level = LogLevel::LOG_UNKNOWN;
    long pid    = -1;
    long thread = -1;
    long logger = -1;
    long source = -1;
    std::string message;
    std::string extra;
};


class StringCache : public std::vector<std::string>
{
public:
    StringCache();

    long Find(const std::string& str)const;
    long Find(const std::string_view& str)const;
    long Get(const std::string& str);
    long Get(const std::string_view& str);

    const std::string& GetString(long id)const;

    static std::string_view Trim(const std::string_view& str);
};


class LogData
{
public:
    struct Listener
    {
        virtual void Updated(LogData& data) = 0;
    };

    typedef std::vector<LogEntry>::iterator iterator;
    typedef std::vector<LogEntry>::const_iterator const_iterator;

protected:
    FileData& _fileData;

    StringCache _threads, _loggers, _sources;

    std::vector<LogEntry> _entries;

    // Entry count per log level
    std::array<size_t, LOG_LEVEL_COUNT> _levelCounts{0, 0, 0, 0, 0, 0, 0, 0};

    std::vector<long> _loggersEntryCount;
    std::vector<std::array<size_t, LOG_LEVEL_COUNT>> _levelLoggerCounts;

    std::set<Listener*> _listeners;
    void NotifyUpdate();

public:
    LogData(FileData& fileData);

    FileData& GetFileData() {return _fileData; }
    const FileData& GetFileData() const {return _fileData; }

    void Clear();

    void AddLog(uint16_t file, const ParsedEntry& entry);
    void AddLog(uint16_t file, const Timestamp& date, LogLevel level, long pid, const std::string& thread, const std::string& logger, const std::string& source, const std::string& message);
    void AddLog(uint16_t file, const Timestamp& date, LogLevel level, long pid, long thread, long logger, long source, const std::string& message);


    template<typename Pred>
    void RemoveLogIf(Pred pred) {
        _entries.erase(std::remove_if(_entries.begin(), _entries.end(), pred), _entries.end());
    }

    void Synchronize();

    void SortLogsByDate();
    void SortAndReindexColumns();
    void UpdateStatistics();

    size_t EntryCount()const { return _entries.size(); }

    LogEntry& GetEntry(size_t index) { return _entries[index]; }
    const LogEntry& GetEntry(size_t index) const { return _entries[index]; }

    LogEntry& front() { return _entries.front(); }
    const LogEntry& front() const { return _entries.front(); }

    LogEntry& back() { return _entries.back(); }
    const LogEntry& back() const { return _entries.back(); }

    iterator begin() {return _entries.begin();}
    const_iterator begin()const {return _entries.begin();}
    iterator end() {return _entries.end();}
    const_iterator end()const {return _entries.end();}

    size_t GetLevelLogCount(LogLevel level)const { return _levelCounts[level]; }
    Timestamp GetBeginDate()const;
    Timestamp GetEndDate()const;

    size_t GetThreadCount()const { return _threads.size(); }
    size_t GetLoggerCount()const { return _loggers.size(); }
    size_t GetSourceCount()const { return _sources.size(); }

    const std::string& GetThreadLabel(long id)const { return _threads.GetString(id); }
    const std::string& GetLoggerLabel(long id)const { return _loggers.GetString(id); }
    const std::string& GetSourceLabel(long id)const { return _sources.GetString(id); }

    long GetThread(const std::string& name) { return _threads.Get(name); }
    long GetLogger(const std::string& name) { return _loggers.Get(name); }
    long GetSource(const std::string& name) { return _sources.Get(name); }

    long FindThread(const std::string& name) const { return _threads.Find(name); }
    long FindLogger(const std::string& name) const { return _loggers.Find(name); }
    long FindSource(const std::string& name) const { return _sources.Find(name); }

    long GetLoggerEntryCount(long logger) const {return _loggersEntryCount[logger]; }
    long GetLevelLoggerEntryCount(long logger, LogLevel criticality) const {return _levelLoggerCounts[logger][criticality]; }

    // @name Listener management
    // @{
    void AddListener(Listener* listener) { _listeners.insert(listener); }
    void RemListener(Listener* listener) { _listeners.erase(listener); }
    // @}

};






class FilteredLogData : protected LogData::Listener
{
public:
    struct Listener
    {
        virtual void Updated(FilteredLogData& data) = 0;
    };

protected:
    LogData & _src;

    std::vector<long> _data;
    std::array<size_t, LOG_LEVEL_COUNT> _levelCounts;

    std::vector<bool> _shownLoggers, _shownFiles;

    LogLevel _criticality = LogLevel::LOG_INFO;
    Timestamp _start = Timestamp::min(), _end = Timestamp::max();

    virtual void Updated(LogData & data) override;

    void Update();

    std::set<Listener*> _listeners;
    void NotifyUpdate();

private:
    void DoSelectAllLoggers();

public:
    FilteredLogData(LogData& data);
    ~FilteredLogData();

    const LogData & GetLogData() const { return _src; }
    LogData & GetLogData() { return _src; }

    FileData& GetFileData() {return _src.GetFileData(); }
    const FileData& GetFileData() const {return _src.GetFileData(); }

    size_t EntryCount()const { return _data.size(); }

    LogEntry& GetEntry(size_t index) { return GetLogData().GetEntry(_data[index]); }
    const LogEntry& GetEntry(size_t index) const { return GetLogData().GetEntry(_data[index]); }

    size_t GetCriticalityCount(LogLevel level)const { return _levelCounts[level]; }
    Timestamp GetBeginDate()const;
    Timestamp GetEndDate()const;

    void ClearFilter();
    void SetCriticalityFilterLevel(LogLevel criticality);
    void SetStartDate(const Timestamp& date);
    void SetEndDate(const Timestamp& date);
    void ResetStartDate();
    void ResetEndDate();

    void DisplayAllLoggers();
    void HideAllLoggers();
    void DisplayLogger(const std::string& logger, bool display = true);
    void DisplayLogger(long logger, bool display = true);
    void DisplayOnlyLogger(long logger);
    void DisplayAllButLogger(long logger);
    void ToggleLogger(long logger);

    bool IsLoggerShown(const std::string& logger)const;
    bool IsLoggerShown(long logger)const;

    void DisplayAllFiles();
    void HideAllFiles();
    void DisplayFile(const std::string& file, bool display = true);
    void DisplayFile(uint16_t file, bool display = true);
    void ToggleFile(uint16_t file);

    bool IsFileShown(const std::string& file)const;
    bool IsFileShown(uint16_t file)const;

    void AddListener(Listener* listener) { _listeners.insert(listener); }
    void RemListener(Listener* listener) { _listeners.erase(listener); }
};




#endif /* _DATA_HPP_ */
