/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* helpers.hpp
* Copyright (C) 2025 Emilien Kia <Emilien.Kia+dev@gmail.com>
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
#ifndef LOGVIEWER_HELPERS_HPP
#define LOGVIEWER_HELPERS_HPP


#include <wx/datetime.h>
#include <wx/string.h>

#include "data.hpp"

inline wxString str2wx(const std::string& str)
{
    return !str.empty() ? wxString(str.data(), wxConvUTF8) : wxString();
}

inline std::string wx2str(const wxString str)
{
    return std::string(str.utf8_str());
}

inline wxDateTime TimestampToDateTime(const Timestamp& ts)
{
    return wxDateTime(std::chrono::duration_cast<std::chrono::seconds>(ts.time_since_epoch()).count());
}

inline Timestamp wxDateTimeToTimestamp(const wxDateTime& dt)
{
    return Timestamp(std::chrono::seconds(dt.GetTicks()));
}


class Formatter
{
public:
    static wxString StatusToString(FileDescriptor::FILE_DESC_STATUS status);
    static const wxString& FormatCriticality(LogLevel c);
    static wxString FormatDate(const wxDateTime& date);
    static wxString FormatDate(const Timestamp& date);
};



#endif //LOGVIEWER_HELPERS_HPP
