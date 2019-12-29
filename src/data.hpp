/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* data.hpp
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

#ifndef _DATA_HPP_
#define _DATA_HPP_

#include <algorithm>
#include <array>
#include <vector>
#include <set>

#include <wx/arrstr.h>

class LogData;

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

	long Find(const wxString& str)const;

	long Get(const wxString& str);
	const wxString& GetString(long id)const;
};


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
	uint16_t file;
	CRITICALITY_LEVEL criticality;
	long thread;
	long logger;
	long source;
	wxString message;
	wxString extra;
};


struct FileDescriptor
{
	FileDescriptor():id(0), path() {}

	FileDescriptor(uint16_t id, wxString path):id(id), path(path) {}
	FileDescriptor(const FileDescriptor& fd) = default; //:id(fd.id), path(fd.path), entryCount(fd.entryCount), criticalityCounts(fd.criticalityCounts) {}
	FileDescriptor(FileDescriptor&& fd) = default;

	FileDescriptor& operator=(const FileDescriptor&) = default;
	FileDescriptor& operator=(FileDescriptor&&) = default;

	uint16_t id;
	wxString path;

	enum FILE_DESC_STATUS
	{
		FILE_LOADED,	// The file have already been loaded
		FILE_NEW,		// The file is new, never loaded
		FILE_RELOAD,	// The file is to reload, have already been loaded.
		FILE_REMOVED	// The file will be removed
	} status = FILE_NEW;

	size_t entryCount;
	std::array<size_t, LOG_CRITICALITY_COUNT> criticalityCounts;


	static wxString StatusToString(FILE_DESC_STATUS status);
};


class FileData
{
public:
	struct Listener
	{
		virtual void Updated(FileData& data) = 0;
	};


	typedef std::vector<FileDescriptor> FileVector;
	typedef std::vector<FileDescriptor>::iterator iterator;
	typedef std::vector<FileDescriptor>::const_iterator const_iterator;

protected:
	std::vector<FileDescriptor> _fileDescriptors;

private:
	std::set<Listener*> _fileListeners;

public:
	FileData() = default;


	uint16_t AddFile(const wxString path);

	size_t GetFileCount()const {return _fileDescriptors.size();}
	FileDescriptor& GetFile(const wxString& file);
	FileDescriptor& GetFile(uint16_t id);
	const FileDescriptor& GetFile(uint16_t id)const;
	const FileDescriptor* FindFile(const wxString& file)const;
	const FileDescriptor* FindFile(uint16_t id)const;

	long GetFileEntryCount(uint16_t fileid) const {return GetFile(fileid).entryCount; }
	long GetFileCriticalityEntryCount(uint16_t fileid, CRITICALITY_LEVEL criticality) const {return GetFile(fileid).criticalityCounts[criticality]; }

	iterator begin() {return _fileDescriptors.begin();}
	const_iterator begin()const {return _fileDescriptors.begin();}
	iterator end() {return _fileDescriptors.end();}
	const_iterator end()const {return _fileDescriptors.end();}
	
	template<typename Pred>
	void RemoveFileIf(Pred pred) {
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


class Formatter
{
public:
	static const wxString& FormatCriticality(CRITICALITY_LEVEL c);
	static wxString FormatDate(const wxDateTime& date);
};



class LogData
{
public:
	struct Listener
	{
		virtual void Updated(LogData& data) = 0;
	};

	typedef std::vector<Entry> FileVector;
	typedef std::vector<Entry>::iterator iterator;
	typedef std::vector<Entry>::const_iterator const_iterator;

protected:
	FileData& _fileData;

	wxStringCache _threads, _loggers, _sources;

	std::vector<Entry> _entries;

	std::array<size_t, LOG_CRITICALITY_COUNT> _criticalityCounts;

	std::vector<long> _loggersEntryCount;
	std::vector<std::array<size_t, LOG_CRITICALITY_COUNT>> _criticalityLoggerCounts;

	std::set<Listener*> _listeners;
	void NotifyUpdate();

public:
	LogData(FileData& fileData);

	FileData& GetFileData() {return _fileData; }
	const FileData& GetFileData() const {return _fileData; }

	void Clear();
	void AddLog(const wxDateTime& date, uint16_t file, CRITICALITY_LEVEL criticality, wxString thread, wxString logger, wxString source, wxString message);
	void AddLog(const wxDateTime& date, uint16_t file, CRITICALITY_LEVEL criticality, long thread, long logger, long source, const wxString& message);

	template<typename Pred>
	void RemoveLogIf(Pred pred) {
		_entries.erase(std::remove_if(_entries.begin(), _entries.end(), pred), _entries.end());
	}

	void Synchronize();

	void SortLogsByDate();
	void SortAndReindexColumns();
	void UpdateStatistics();

	size_t EntryCount()const { return _entries.size(); }

	Entry& GetEntry(size_t index) { return _entries[index]; }
	const Entry& GetEntry(size_t index) const { return _entries[index]; }

	Entry& GetLastEntry() { return _entries.back(); }
	const Entry& GetLastEntry() const { return _entries.back(); }

	iterator begin() {return _entries.begin();}
	const_iterator begin()const {return _entries.begin();}
	iterator end() {return _entries.end();}
	const_iterator end()const {return _entries.end();}


	size_t GetCriticalityCount(CRITICALITY_LEVEL level)const { return _criticalityCounts[level]; }
	wxDateTime GetBeginDate()const;
	wxDateTime GetEndDate()const;

	size_t GetThreadCount()const { return _threads.size(); }
	size_t GetLoggerCount()const { return _loggers.size(); }
	size_t GetSourceCount()const { return _sources.size(); }

	const wxString& GetThreadLabel(long id)const { return _threads.GetString(id); }
	const wxString& GetLoggerLabel(long id)const { return _loggers.GetString(id); }
	const wxString& GetSourceLabel(long id)const { return _sources.GetString(id); }

	long GetThread(const wxString& name) { return _threads.Get(name); }
	long GetLogger(const wxString& name) { return _loggers.Get(name); }
	long GetSource(const wxString& name) { return _sources.Get(name); }

	long FindThread(const wxString& name) const { return _threads.Find(name); }
	long FindLogger(const wxString& name) const { return _loggers.Find(name); }
	long FindSource(const wxString& name) const { return _sources.Find(name); }

	long GetLoggerEntryCount(long logger) const {return _loggersEntryCount[logger]; }
	long GetLoggerCriticalityEntryCount(long logger, CRITICALITY_LEVEL criticality) const {return _criticalityLoggerCounts[logger][criticality]; }


	// @name Listener management
	// @{
	void AddListener(Listener* listener) { _listeners.insert(listener); }
	void RemListener(Listener* listener) { _listeners.erase(listener); }
	// 
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
	std::array<size_t, LOG_CRITICALITY_COUNT> _criticalityCounts;

	std::vector<bool> _shownLoggers, _shownFiles;

	CRITICALITY_LEVEL _criticality = CRITICALITY_LEVEL::LOG_INFO;
	wxDateTime _start, _end;

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

	Entry& GetEntry(size_t index) { return GetLogData().GetEntry(_data[index]); }
	const Entry& GetEntry(size_t index) const { return GetLogData().GetEntry(_data[index]); }

	size_t GetCriticalityCount(CRITICALITY_LEVEL level)const { return _criticalityCounts[level]; }
	wxDateTime GetBeginDate()const;
	wxDateTime GetEndDate()const;

	void ClearFilter();
	void SetCriticalityFilterLevel(CRITICALITY_LEVEL criticality);
	void SetStartDate(const wxDateTime& date);
	void SetEndDate(const wxDateTime& date);
	void ResetStartDate();
	void ResetEndDate();

	void DisplayAllLoggers();
	void HideAllLoggers();
	void DisplayLogger(const wxString& logger, bool display = true);
	void DisplayLogger(long logger, bool display = true);
	void DisplayOnlyLogger(long logger);
	void DisplayAllButLogger(long logger);
	void ToggleLogger(long logger);

	bool IsLoggerShown(const wxString& logger)const;
	bool IsLoggerShown(long logger)const;

	void DisplayAllFiles();
	void HideAllFiles();
	void DisplayFile(const wxString& file, bool display = true);
	void DisplayFile(uint16_t file, bool display = true);
	void ToggleFile(uint16_t file);

	bool IsFileShown(const wxString& file)const;
	bool IsFileShown(uint16_t file)const;


	void AddListener(Listener* listener) { _listeners.insert(listener); }
	void RemListener(Listener* listener) { _listeners.erase(listener); }
};




#endif /* _DATA_HPP_ */
