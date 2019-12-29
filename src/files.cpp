/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * files.cpp
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

#include "files.hpp"

#include "app.hpp"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/filepicker.h>




//
// File Dialog List Model
//

FileDialogListModel::FileDialogListModel(FileData& data) :
	_data(data)
{
	data.AddListener(this);
}

FileDialogListModel::~FileDialogListModel()
{
	_data.RemListener(this);
}


unsigned int FileDialogListModel::GetColumnCount()const
{
	return FileDialogListModel::COLUMN_COUNT;
}

wxString FileDialogListModel::GetColumnType(unsigned int col)const
{
	return "string";
}

void FileDialogListModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
	switch (col)
	{
	case FileDialogListModel::STATUS:
		variant = FileDescriptor::StatusToString(GetData().GetFile(row).status);
		return;
	case FileDialogListModel::FILENAME:
		variant = GetData().GetFile(row).path;
		return;
	case FileDialogListModel::READER:
		variant = "<reader>";
		return;
	case FileDialogListModel::LAYOUT:
		variant = "<layout>";
		return;
	default:
		return;
	}
}

bool FileDialogListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const
{
	return false;
}

bool FileDialogListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
	return false;
}

void FileDialogListModel::Update()
{
	long count = GetData().GetFileCount();
	Reset(count);
}

void FileDialogListModel::Updated(FileData& data)
{
	Update();
}

uint16_t FileDialogListModel::GetFileId(wxDataViewItem item)const
{
	return GetRow(item);
}

const FileDescriptor* FileDialogListModel::GetFile(wxDataViewItem item)const
{
	return _data.FindFile(GetFileId(item));
}



//
// FileOpenDialog
// 



BEGIN_EVENT_TABLE(FileOpenDialog, wxDialog)
	EVT_UPDATE_UI(wxID_REVERT, FileOpenDialog::OnUpdateReloadFilesButton)
	EVT_UPDATE_UI(wxID_CLOSE, FileOpenDialog::OnUpdateRemoveFilesButton)

	EVT_BUTTON(wxID_OPEN, FileOpenDialog::OnOpenFilesButton)
	EVT_BUTTON(wxID_REVERT, FileOpenDialog::OnReloadFilesButton)
	EVT_BUTTON(wxID_CLOSE, FileOpenDialog::OnRemoveFilesButton)
END_EVENT_TABLE()


bool FileOpenDialog::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size)
{
	if(!wxDialog::Create(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX)) {
		return false;		
	}

	_filesModel = new FileDialogListModel(wxGetApp().GetFileData());

	wxSizer* gsz = new wxBoxSizer(wxVERTICAL);

	wxSizer* tbsz = new wxBoxSizer(wxHORIZONTAL);

	wxButton* btn;
	btn = new wxButton(this, wxID_OPEN, "Open files...");
	btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON));
	tbsz->Add(btn , 0, wxALL, 4);

	btn = new wxButton(this, wxID_REVERT, "Reload");
	btn->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_BUTTON));
	tbsz->Add(btn , 0, wxALL, 4);

	btn = new wxButton(this, wxID_CLOSE, "Remove");
	btn->SetBitmap(wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON));
	tbsz->Add(btn , 0, wxALL, 4);

	gsz->Add(tbsz, 0, wxEXPAND);

	_files = new wxDataViewCtrl(this, ID_LV_FILEBOX_FILE_DVCTRL, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES);
	_files->AppendTextColumn("Status",    FileDialogListModel::STATUS,  wxDATAVIEW_CELL_INERT, 64, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	_files->AppendTextColumn("File",     FileDialogListModel::FILENAME, wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE)
		->GetRenderer()->EnableEllipsize(wxELLIPSIZE_START);
//	_files->AppendTextColumn("Reader",    FileDialogListModel::READER,  wxDATAVIEW_CELL_INERT, 128, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
//	_files->AppendTextColumn("Layout", FileDialogListModel::LAYOUT, 	wxDATAVIEW_CELL_INERT, 128, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
	_files->AssociateModel(_filesModel);
	

	gsz->Add(_files, 1, wxALL|wxEXPAND, 4);

	gsz->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND|wxALIGN_BOTTOM, 4);
	SetSizer(gsz);

	_filesModel->Update();
}


void FileOpenDialog::OnOpenFilesButton(wxCommandEvent& event)
{
	wxArrayString paths;
	int res = wxGetApp().OpenFileDialog(this, paths);
	if(res == wxID_CANCEL)
		return;

	wxVector<uint16_t> fids;
	for (const wxString& path : paths)
	{
		fids.push_back(_filesModel->GetData().GetFile(path).id);
	}

	_filesModel->Update();
}

void FileOpenDialog::OnReloadFilesButton(wxCommandEvent& event)
{
	if(_files->HasSelection())
	{
		FileDescriptor* fd = const_cast<FileDescriptor*>(_filesModel->GetFile(_files->GetSelection()));
		if(fd)
		{
			fd->status = FileDescriptor::FILE_RELOAD;
			_filesModel->Update();
		}
	}
}

void FileOpenDialog::OnRemoveFilesButton(wxCommandEvent& event)
{
	if(_files->HasSelection())
	{
		FileDescriptor* fd = const_cast<FileDescriptor*>(_filesModel->GetFile(_files->GetSelection()));
		if(fd)
		{
			fd->status = FileDescriptor::FILE_REMOVED;
			_filesModel->Update();
		}
	}	
}

void FileOpenDialog::OnUpdateReloadFilesButton(wxUpdateUIEvent& event)
{
	event.Enable(_files->HasSelection());
}

void FileOpenDialog::OnUpdateRemoveFilesButton(wxUpdateUIEvent& event)
{
	event.Enable(_files->HasSelection());
}
