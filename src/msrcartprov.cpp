/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * msrcartprov.cpp
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

#include "msrcartprov.hpp"

#if __WXMSW__

wxMicrosoftResourceArtProvider::wxMicrosoftResourceArtProvider()
{
}

wxBitmap wxMicrosoftResourceArtProvider::CreateBitmap(const wxArtID &id, const wxArtClient &/*client*/, const wxSize &size)
{
	wxString resname = "IDB_" + id.Upper();
	resname.Replace("-", "_");

    wxBitmap bmp;
	bmp.LoadFile(resname, wxBITMAP_TYPE_PNG_RESOURCE);
    return bmp;
}

#endif // __WXMSW__
