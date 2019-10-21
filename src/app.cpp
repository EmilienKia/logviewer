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
#include "frame.hpp"
#include "fdartprov.hpp"


IMPLEMENT_APP(LogViewerApp)

LogViewerApp::LogViewerApp():
	_data(),
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

	SetAppName("LogViewer");
	SetAppDisplayName("LogViewer");

	_frame = new Frame();
	_frame->Show(TRUE);
	SetTopWindow(_frame);

	return TRUE;
}

BEGIN_EVENT_TABLE(LogViewerApp, wxApp)
	EVT_MENU(wxID_OPEN, LogViewerApp::OnOpen)
	EVT_MENU(wxID_CLEAR, LogViewerApp::OnClear)
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
	OpenFiles(files);
}

void LogViewerApp::OnClear(wxCommandEvent& event)
{
	GetLogData().Clear();
}

void LogViewerApp::OpenFiles(const wxArrayString& files)
{
	Parser parser(GetLogData());
	parser.ParseLogFiles(files);
}
