/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * files.hpp
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

#ifndef _FILES_HPP_
#define _FILES_HPP_

#include "data.hpp"

#include <wx/dialog.h>
#include <wx/dataview.h>

#include "data.hpp"


class FileDialogListModel : public wxDataViewVirtualListModel, protected FileData::Listener
{
public:
	FileDialogListModel(FileData& data);
	~FileDialogListModel();

	const FileData& GetData() const { return _data; }
	FileData& GetData() { return _data; }

	// DVVLM definitions:
	virtual unsigned int GetColumnCount()const;
	virtual wxString GetColumnType(unsigned int col)const;

	virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const;
	virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col);
	virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const;

	// Model definition
	enum FileDialogListModelColumns {
		STATUS,
		FILENAME,

		READER,
		LAYOUT,

		COLUMN_COUNT
	};

	// Model helpers
	uint16_t GetFileId(wxDataViewItem item)const;
	const FileDescriptor* GetFile(wxDataViewItem item)const;

	void Update();

protected:
	virtual void Updated(FileData& data) override;

	FileData& _data;
};



class FileOpenDialog : public wxDialog
{
	DECLARE_EVENT_TABLE()
public:
	FileOpenDialog() = default;
	FileOpenDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize)
	{
		Create(parent, id, title, pos, size);
	}

	bool Create (wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

protected:
	wxDataViewCtrl* _files;

	FileDialogListModel* _filesModel;

	void OnOpenFilesButton(wxCommandEvent& event);
	void OnReloadFilesButton(wxCommandEvent& event);
	void OnRemoveFilesButton(wxCommandEvent& event);

	void OnUpdateReloadFilesButton(wxUpdateUIEvent& event);
	void OnUpdateRemoveFilesButton(wxUpdateUIEvent& event);
};


#endif // _FILES_HPP_
