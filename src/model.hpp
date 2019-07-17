/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* model.hpp
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

#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <wx/dataview.h>

#include "data.hpp"


class LogListModel : public wxDataViewVirtualListModel, protected FilteredLogData::Listener
{
public:
	LogListModel(FilteredLogData& data);

	// DVVLM definitions:
	virtual unsigned int GetColumnCount()const;
	virtual wxString GetColumnType(unsigned int col)const;

	virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
	virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);
	virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const;

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

	const FilteredLogData& GetData() const { return _data; }
	FilteredLogData& GetData() { return _data; }

	// Model helpers
	size_t Count()const;
	const Entry& Get(size_t id)const;
	Entry& Get(size_t id);
	const Entry& Get(wxDataViewItem item)const;
	Entry& Get(wxDataViewItem item);
	unsigned int GetPos(wxDataViewItem item)const;

protected:
	void Update();
	virtual void Updated(FilteredLogData& data) override;

	FilteredLogData& _data;

};


class LoggerListModel : public wxDataViewVirtualListModel, protected FilteredLogData::Listener
{
public:
	LoggerListModel(FilteredLogData& data);

	const FilteredLogData& GetData() const { return _data; }
	FilteredLogData& GetData() { return _data; }

	// DVVLM definitions:
	virtual unsigned int GetColumnCount()const;
	virtual wxString GetColumnType(unsigned int col)const;

	virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
	virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);
	virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const;

	// Model definition
	enum LoggerListModelColumns {
		SHOWN,
		LOGGER,

		COLUMN_COUNT
	};

	// Model helpers
	long GetLoggerId(wxDataViewItem item)const;

protected:
	void Update();
	virtual void Updated(FilteredLogData& data) override;

	FilteredLogData& _data;
};


#endif /* _MODEL_HPP_ */
