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

#include <wx/artprov.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>

#include <algorithm>
//#include <execution>

#include "app.hpp"
#include "parser.hpp"
#include "frame.hpp"
#include "fdartprov.hpp"
#include "msrcartprov.hpp"

#include "files.hpp"

IMPLEMENT_APP(LogViewerApp)

LogViewerApp::LogViewerApp():
	_files(),
	_data(_files),
	_filteredData(_data)
{
}

bool LogViewerApp::OnInit()
{
    if(!wxApp::OnInit())
        return false;

    // Init all image handlers
    wxInitAllImageHandlers();

    // Plug additionnal art providers
#ifdef ICONSETDIR
	wxArtProvider::Push(new wxFreedesktopArtProvider(ICONSETDIR));
#endif // ICONSETDIR
#ifdef __UNIX__
//    wxArtProvider::Push(new wxFreedesktopArtProvider("/usr/share/icons/gnome"));
//    wxArtProvider::Push(new wxFreedesktopArtProvider(wxStandardPaths::Get().GetDataDir()+"/icons/hicolor"));
#endif
#if __WXMSW__
	wxArtProvider::Push(new wxMicrosoftResourceArtProvider);
#endif

	SetAppName("LogViewer");
	SetAppDisplayName("LogViewer");

	_frame = new Frame();
	_frame->Show(TRUE);
	SetTopWindow(_frame);

	return TRUE;
}

BEGIN_EVENT_TABLE(LogViewerApp, wxApp)
	EVT_MENU(wxID_OPEN, LogViewerApp::OnOpen)
	EVT_MENU(ID_LV_FILE_MANAGE, LogViewerApp::OnManage)
	EVT_MENU(wxID_CLEAR, LogViewerApp::OnClear)
	EVT_MENU(wxID_EXIT, LogViewerApp::OnExit)
END_EVENT_TABLE()

void LogViewerApp::OnExit(wxCommandEvent& event)
{
	_frame->Close();
}

void LogViewerApp::OnOpen(wxCommandEvent& event)
{
	wxArrayString paths;
	int res = OpenFileDialog(_frame, paths);
	if(res == wxID_CANCEL)
		return;

	for (const wxString& path : paths)
	{
		GetFileData().GetFile(path);
	}

	FileManagement();
}

void LogViewerApp::OnManage(wxCommandEvent& event)
{
	FileManagement();
}

void LogViewerApp::OnClear(wxCommandEvent& event)
{
	for(FileDescriptor& fd : GetFileData())
	{
		fd.status = FileDescriptor::FILE_REMOVED;
	}
	FileManagement();
}

int LogViewerApp::OpenFileDialog(wxWindow* parent, wxArrayString& paths)
{
	wxFileDialog fd(parent, _("Open log files"), "", "", "Log files (*.log;*.log.*)|*.log;*.log.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE);
	int res = fd.ShowModal();
	if(res == wxID_CANCEL)
		return res;

	fd.GetPaths(paths);
	return res;
}

void LogViewerApp::OpenFiles(const wxArrayString& files)
{
	for (const wxString& path : files)
	{
		GetFileData().GetFile(path);
	}	

	FileManagement();
}


void LogViewerApp::FileManagement()
{
	FileOpenDialog fd(_frame, wxID_ANY, _("Manage log files"));
	if(fd.ShowModal() == wxID_OK)
		ApplyUpdates();
	else
		CancelUpdates();
}

void LogViewerApp::ApplyUpdates()
{
	// First: Remove logs from removed and reloaded files
	std::vector<uint16_t> filesToRemove;
	for(const FileDescriptor& fd : GetFileData()) {
		if(fd.status==FileDescriptor::FILE_REMOVED || fd.status==FileDescriptor::FILE_RELOAD)
			filesToRemove.push_back(fd.id);
	}

	GetLogData().RemoveLogIf([&](const Entry& entry){
			return std::find(filesToRemove.begin(), filesToRemove.end(), entry.file) != filesToRemove.end();
		});

	// Second: Effectively revome files
	GetFileData().RemoveFileIf([&](FileDescriptor& entry){
			return entry.status==FileDescriptor::FILE_REMOVED;
	});

	// Third: Load logs from new and reloaded files
	Parser parser(GetLogData(), GetFileData());
	for(FileDescriptor& fd : GetFileData()) {
		if(fd.status==FileDescriptor::FILE_NEW || fd.status==FileDescriptor::FILE_RELOAD) {
			parser.Parse(fd);
		}
	}

	// Fourth: Mark all files as loaded
	for(FileDescriptor& fd : GetFileData())
		fd.status = FileDescriptor::FILE_LOADED;

	// Finally: Update stats and notify update
	GetLogData().Synchronize();

}

void LogViewerApp::CancelUpdates()
{
	// First: Remove new files
	GetFileData().RemoveFileIf([&](FileDescriptor& entry){
			return entry.status==FileDescriptor::FILE_NEW;
	});

	// Second: Mark removed and reloaded files to loaded.
	for(FileDescriptor& fd : GetFileData())
		fd.status = FileDescriptor::FILE_LOADED;

	// Finally: Update stats and notify update
	GetLogData().Synchronize();
}
