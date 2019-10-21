/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * fdartprov.cpp
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
#  include <config.h>
#endif
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#include "fdartprov.hpp"

wxFreedesktopArtProvider::wxFreedesktopArtProvider()
{
}

wxFreedesktopArtProvider::wxFreedesktopArtProvider(const wxString& path)
{
    Load(path);
}

void wxFreedesktopArtProvider::Load(const wxString& path)
{
    m_path = path;

    wxUniChar sep = wxFileName::GetPathSeparator();
    m_dirList.clear();

    wxFileConfig conf(wxEmptyString, wxEmptyString, path + sep + "index.theme", wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
    wxString fileliststr = conf.Read("/Icon Theme/Directories", "");

    wxStringTokenizer tokenizer(fileliststr, ",");
    while(tokenizer.HasMoreTokens())
    {
        wxString dirname = tokenizer.GetNextToken();
        conf.SetPath("/" + dirname);

        IconDirDescription desc;
        desc.path = m_path + sep + dirname;
        desc.size = (int) conf.ReadLong("Size", 0);

        if(desc.size!=0)
        {
            m_dirList.push_back(desc);
        }
    }
}


wxBitmap wxFreedesktopArtProvider::CreateBitmap(const wxArtID &id, const wxArtClient &/*client*/, const wxSize &size)
{
    wxBitmap bmp;
    wxUniChar sep = wxFileName::GetPathSeparator();
    for(std::list<IconDirDescription>::iterator iter = m_dirList.begin(); iter != m_dirList.end(); ++iter)
    {
        IconDirDescription &dir = *iter;
        if(dir.size == size.GetWidth())
        {
            wxString path;
            // Look at png
            path = dir.path + sep + id + ".png";
            if(wxFile::Exists(path))
                if(bmp.LoadFile(path, wxBITMAP_TYPE_PNG))
                    return bmp;
            // Look at xpm
            path = dir.path + sep + id + ".xpm";
            if(wxFile::Exists(path))
                if(bmp.LoadFile(path, wxBITMAP_TYPE_XPM))
                    return bmp;
        }
    }
    return bmp;
}

wxIconBundle wxFreedesktopArtProvider::CreateIconBundle(const wxArtID &id, const wxArtClient &/*client*/)
{
    wxIconBundle bundle;
    wxUniChar sep = wxFileName::GetPathSeparator();
    for(std::list<IconDirDescription>::iterator iter = m_dirList.begin(); iter != m_dirList.end(); ++iter)
    {
        IconDirDescription &dir = *iter;
        wxIcon icon;
        wxString path;
        // Look at png
        path = dir.path + sep + id + ".png";
        if(wxFile::Exists(path))
            if(icon.LoadFile(path, wxBITMAP_TYPE_PNG))
                bundle.AddIcon(icon);
        // Look at xpm
        path = dir.path + sep + id + ".xpm";
        if(wxFile::Exists(path))
            if(icon.LoadFile(path, wxBITMAP_TYPE_XPM))
                bundle.AddIcon(icon);
    }
    return bundle;
}
