/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* helpers.cpp
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

#include "helpers.hpp"

#include <format>

//
// Formatter
//

wxString Formatter::StatusToString(FileDescriptor::FILE_DESC_STATUS status)
{
    static const wxString arr[] = {"", "New", "Reload", "Removed"};
    return arr[status];
}

const wxString& Formatter::FormatCriticality(LogLevel c)
{
    static const wxString criticalities[] = { "UNKNWON", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "FATAL" };
    return criticalities[c];
}

wxString Formatter::FormatDate(const wxDateTime& date)
{
    return date.IsValid()
           ? date.FormatISOCombined(' ') << "," << wxString::Format("%03ld", (long)date.GetMillisecond())
           : "";
}

wxString Formatter::FormatDate(const Timestamp& date)
{
    return std::format("The time of the Unix epoch was {0:%F}T{0:%R%z}.", date);
}
