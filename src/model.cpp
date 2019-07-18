/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* model.cpp
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

#include "model.hpp"


//
// Log List Model
//

LogListModel::LogListModel(FilteredLogData& data) :
	_data(data)
{
	data.AddListener(this);
}

size_t LogListModel::Count()const
{
	return GetData().EntryCount();
}

const Entry& LogListModel::Get(size_t id)const
{
	return GetData().GetEntry(id);
}

Entry& LogListModel::Get(size_t id)
{
	return GetData().GetEntry(id);
}

const Entry& LogListModel::Get(wxDataViewItem item)const
{
	return Get(GetRow(item));
}

Entry& LogListModel::Get(wxDataViewItem item)
{
	return Get(GetRow(item));
}


unsigned int LogListModel::GetPos(wxDataViewItem item)const
{
	return GetRow(item);
}

unsigned int LogListModel::GetColumnCount()const
{
	return LogListModel::COLUMN_COUNT;
}

wxString LogListModel::GetColumnType(unsigned int col)const
{
	if (col == LogListModel::EXTRA)
		return "bool";
	else
		return "string";
}

void LogListModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
	auto& entry = GetData().GetEntry(row);
	switch (col)
	{
	case LogListModel::DATE:
		variant = Formatter::FormatDate(entry.date);
		return;
	case LogListModel::CRITICALITY:
		variant = Formatter::FormatCriticality(entry.criticality);
		return;
	case LogListModel::THREAD:
		variant = GetData().GetLogData().GetThreadLabel(entry.thread);
		return;
	case LogListModel::LOGGER:
		variant = GetData().GetLogData().GetLoggerLabel(entry.logger);
		return;
	case LogListModel::SOURCE:
		variant = GetData().GetLogData().GetSourceLabel(entry.source);
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

bool LogListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const
{
	// TODO
	return false;
}

bool LogListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
	return false;
}

void LogListModel::Update()
{
	Reset(GetData().EntryCount());
}

void LogListModel::Updated(FilteredLogData& data)
{
	Update();
}

//
// Logger List Model
//

LoggerListModel::LoggerListModel(FilteredLogData& data) :
	_data(data)
{
	data.AddListener(this);
}

unsigned int LoggerListModel::GetColumnCount()const
{
	return LoggerListModel::COLUMN_COUNT;
}

wxString LoggerListModel::GetColumnType(unsigned int col)const
{
	if (col == LoggerListModel::SHOWN)
		return "bool";
	else
		return "string";
}

static wxString wxFormatCount(long num) {
	wxString str;
	if (num != 0) {
		str << num;
	}
	return str;
}

void LoggerListModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
	switch (col)
	{
	case LoggerListModel::SHOWN:
		variant = GetData().IsLoggerShown(row);
		return;
	case LoggerListModel::LOGGER:
		variant = GetData().GetLogData().GetLoggerLabel(row);
		return;
	case LoggerListModel::COUNT:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerEntryCount(row));
		return;
	case LoggerListModel::CRIT_FATAL:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_FATAL));
		return;
	case LoggerListModel::CRIT_CRITICAL:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_CRITICAL));
		return;
	case LoggerListModel::CRIT_ERROR:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_ERROR));
		return;
	case LoggerListModel::CRIT_WARNING:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_WARNING));
		return;
	case LoggerListModel::CRIT_INFO:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_INFO));
		return;
	case LoggerListModel::CRIT_DEBUG:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_DEBUG));
		return;
	case LoggerListModel::CRIT_TRACE:
		variant = wxFormatCount(GetData().GetLogData().GetLoggerCriticalityEntryCount(row, LOG_TRACE));
		return;
	default:
		return;
	}
}

bool LoggerListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const
{
	// TODO
	return false;
}

bool LoggerListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
	if (col == LoggerListModel::SHOWN)
	{
		GetData().DisplayLogger(row, variant.GetBool());
		return true;
	}
	return false;
}

void LoggerListModel::Update()
{
	long count = GetData().GetLogData().GetLoggerCount();
	Reset(count);
}

void LoggerListModel::Updated(FilteredLogData& data)
{
	Update();
}

long LoggerListModel::GetLoggerId(wxDataViewItem item)const
{
	return GetRow(item);
}
