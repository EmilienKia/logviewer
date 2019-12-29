/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* parser.cpp
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

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/strconv.h>
#include <wx/convauto.h>
#include <wx/regex.h>


#include "parser.hpp"



//
// Log parser
//

void Parser::ParseLogFiles(const wxArrayString& paths)
{
	for (auto path : paths)
	{
		ParseLogFile(path);
	}

	_data.Synchronize();
}

void Parser::ParseLogFile(const wxString& path)
{

	wxFFileInputStream file(path);
	if (!file.IsOk())
	{
		wxLogError("Cannot open file %s", path);
		return;
	}

	Parse(_files.GetFile(path));
}

void Parser::Parse(FileDescriptor& fd)
{
	wxFFileInputStream file(fd.path);
	if (!file.IsOk())
	{
		wxLogError("Cannot open file %s", fd.path);
		return;
	}
	_fileDesc = &fd;

	_tempExtra.Empty();

	wxTextInputStream text(file);
	while (!file.Eof())
	{
		ParseLogLine(text.ReadLine());
	}
	AppendExtraLine();

	_fileDesc = nullptr;

}


void Parser::ParseLogLine(const wxString& line)
{
	if (!line.IsEmpty())
	{
		wxArrayString arr = SplitLine(line);
		if (!arr.IsEmpty())
		{
			if (arr[0].Length() < 28)
				// 28 : arbitrary value greater than any supported text date length
			{
				if (arr.GetCount() == 3)
				{
					AddLogLine(arr[0], arr[1], arr[2]);
					return;
				}
				else if (arr.GetCount() == 6)
				{
					AddLogLine(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
					return;
				}
			}
			// Consider as extra line
			_tempExtra.Append(line).Append("\n");
		}
	}
}

wxArrayString Parser::SplitLine(const wxString& line)
{
	wxArrayString arr;
	size_t cur = 0;
	size_t pos = 0;
	while (pos = line.find(" | ", cur), pos != wxString::npos)
	{
		arr.Add(line.SubString(cur, pos - 1));
		cur = pos + 3;
		if (cur >= line.Length()) {
			break;
		}
	}
	if (cur<line.Length()) {
		arr.Add(line.Mid(cur));
	}
	return arr;
}

void Parser::AppendExtraLine()
{
	if (!_tempExtra.IsEmpty())
	{
		if (_data.EntryCount()>0)
		{
			_data.GetLastEntry().extra = _tempExtra;
		}
		_tempExtra.clear();
	}
}

void Parser::AddLogLine(wxString date, wxString criticality, wxString thread, wxString logger, wxString source, wxString message)
{
	AppendExtraLine();
	_data.AddLog(
		ParseDate(date.Trim(false).Trim(true)),
		_fileDesc->id,
		ParseCriticality(criticality.Trim(false)),
		thread,
		logger,
		source,
		message
	);
}

void Parser::AddLogLine(wxString date, wxString logger, wxString message)
{
	AppendExtraLine();
	_data.AddLog(
		ParseDate(date.Trim(false).Trim(true)),
		_fileDesc->id,
		CRITICALITY_LEVEL::LOG_INFO,
		"",
		logger.Trim(false).Trim(true),
		"",
		message.Trim(false).Trim(true)
	);
}


wxDateTime Parser::ParseDate(const wxString& str)
{
	long y = 0, mo = 0, d = 0, h = 0, m = 0, s = 0, ms = 0;
	size_t start, len;

	wxRegEx reDate;;
	if (!reDate.Compile("^(\\d{4})[\\-\\/ ]?(\\d{2})[\\-\\/ ]?(\\d{2})[T\\- ](\\d{2})[\\: ]?(\\d{2})[\\: ]?(\\d{2})([,.](\\d{3}))?$", wxRE_ADVANCED))
	{
		std::cerr << "RegEx compilation error" << std::endl;
		goto error;
	}
	if (reDate.Matches(str))
	{
		// Grp 1 : year
		if (reDate.GetMatch(&start, &len, 1))
		{
			if (!str.substr(start, len).ToLong(&y)) goto error;
		}
		else
		{
			std::cerr << "Regex not match year" << std::endl;
			goto error;
		}

		// Grp 2 : month
		if (reDate.GetMatch(&start, &len, 2))
		{
			if (!str.substr(start, len).ToLong(&mo)) goto error;
		}
		else
		{
			std::cerr << "Regex not match month" << std::endl;
			goto error;
		}

		// Grp 3 : day
		if (reDate.GetMatch(&start, &len, 3))
		{
			if (!str.substr(start, len).ToLong(&d)) goto error;
		}
		else
		{
			std::cerr << "Regex not match day" << std::endl;
			goto error;
		}


		// Grp 4 : hours
		if (reDate.GetMatch(&start, &len, 4))
		{
			if (!str.substr(start, len).ToLong(&h)) goto error;
		}
		else
		{
			std::cerr << "Regex not match hours" << std::endl;
			goto error;
		}


		// Grp 5 : minutes
		if (reDate.GetMatch(&start, &len, 5))
		{
			if (!str.substr(start, len).ToLong(&m)) goto error;
		}
		else
		{
			std::cerr << "Regex not match minutes" << std::endl;
			goto error;
		}


		// Grp 6 : seconds
		if (reDate.GetMatch(&start, &len, 6))
		{
			if (!str.substr(start, len).ToLong(&s)) goto error;
		}
		else
		{
			std::cerr << "Regex not match seconds" << std::endl;
			goto error;
		}


		// Grp 8 : milliseconds
		if (reDate.GetMatch(&start, &len, 8))
		{
			if (!str.substr(start, len).ToLong(&ms)) goto error;
		}
		else ms = 0;

		return wxDateTime(d, (wxDateTime::Month)(mo - 1), y, h, m, s, ms);
	}
	else
	{
		std::cerr << "Regex not match" << std::endl;
	}

error:
	return wxDateTime();
}

CRITICALITY_LEVEL Parser::ParseCriticality(const wxString& str)
{
	if (str.IsEmpty())
	{
		return CRITICALITY_LEVEL::LOG_UNKNWON;
	}
	switch ((char)str[0])
	{
	case 'I':
		return CRITICALITY_LEVEL::LOG_INFO;
	case 'D':
		return CRITICALITY_LEVEL::LOG_DEBUG;
	case 'T':
		return CRITICALITY_LEVEL::LOG_TRACE;
	case 'E':
		return CRITICALITY_LEVEL::LOG_ERROR;
	case 'W':
		return CRITICALITY_LEVEL::LOG_WARNING;
	case 'C':
		return CRITICALITY_LEVEL::LOG_CRITICAL;
	case 'F':
		return CRITICALITY_LEVEL::LOG_FATAL;
	default:
		return CRITICALITY_LEVEL::LOG_UNKNWON;
	}
}
