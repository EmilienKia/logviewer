/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* parser.hpp
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

#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include "data.hpp"

enum CRITICALITY_LEVEL;

class Parser
{
protected:
	FileData & _files;
	LogData & _data;

	wxString _tempExtra;

	FileDescriptor* _fileDesc;

	void ParseLogLine(const wxString& line);

	void AddLogLine(wxString date, wxString logger, wxString message);
	void AddLogLine(wxString date, wxString criticality, wxString thread, wxString logger, wxString source, wxString message);

	void AppendExtraLine();

public:
	Parser(LogData& data, FileData& files) :_data(data), _files(files) {}

	void ParseLogFiles(const wxArrayString& paths);
	void ParseLogFile(const wxString& path);

	void Parse(FileDescriptor& fd);

	static wxArrayString SplitLine(const wxString& line);
	static wxDateTime ParseDate(const wxString& str);
	static CRITICALITY_LEVEL ParseCriticality(const wxString& str);
};




#endif /* _PARSER_HPP_ */
