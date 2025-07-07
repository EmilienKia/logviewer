/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* parser.hpp
* Copyright (C) 2019-2025 Emilien Kia <Emilien.Kia+dev@gmail.com>
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

#include <optional>
#include <memory>
#include <unordered_map>

#include "data.hpp"

enum LogLevel;


struct MatchResult {
    std::unordered_map<std::string, std::string_view> named;
};

class Regex {
public:
    virtual ~Regex() = default;
    virtual std::optional<MatchResult> match(std::string_view input) const = 0;

    static const std::string SpringBoot;
    static std::unique_ptr<Regex> createRegex(const std::string& pattern);
};




class Parser
{
protected:
    FileData & _files;
    LogData & _data;

    std::string _tempExtra;

    FileDescriptor* _fileDesc;

    void ParseLogLine(const std::string& line);

    void AddLogLine(const std::string& date, const std::string& logger, const std::string& message);
    void AddLogLine(const std::string& date, const std::string& level, const std::string& thread, const std::string& logger, const std::string& source, const std::string& message);

    void AppendExtraLine();

public:
    Parser(LogData& data, FileData& files) :_data(data), _files(files) {}

    void ParseLogFiles(const std::vector<std::string>& paths);
    void ParseLogFile(const std::string& path);

    void Parse(FileDescriptor& fd);

    static std::vector<ParsedEntry> ParseLogs(std::string_view input, std::shared_ptr<Regex> regex);


    static std::vector<std::string> SplitLine(const std::string& line);

    static Timestamp ParseTimestamp(const std::string& str);
    static Timestamp ParseISO8601(const std::string& date);
    static LogLevel ParseLogLevel(const std::string_view& str);

    static std::string_view Trim(const std::string_view& str);
    static std::string Trim(const std::string& str);

};







#endif /* _PARSER_HPP_ */
