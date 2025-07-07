/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
* parser.cpp
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

#include <stdexcept>
#include <charconv>
#include <regex>
#include <iomanip>
#include <sstream>
#include <ctime>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <fstream>
#include <iostream>

#include "parser.hpp"


//
// PCRE2-based regex parser
//

class PCRE2Regex : public Regex {
public:
    explicit PCRE2Regex(const std::string& pattern) {
        int error;
        PCRE2_SIZE error_offset;
        regex_ = pcre2_compile(
                (PCRE2_SPTR)pattern.c_str(), pattern.size(), 0, &error, &error_offset, nullptr);
        if (!regex_) throw std::runtime_error("Invalid regex pattern");

        name_count_ = 0;
        pcre2_pattern_info(regex_, PCRE2_INFO_NAMECOUNT, &name_count_);
        pcre2_pattern_info(regex_, PCRE2_INFO_NAMETABLE, &name_table_);
        pcre2_pattern_info(regex_, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size_);
    }

    ~PCRE2Regex() override {
        if (regex_) pcre2_code_free(regex_);
    }

    std::optional<MatchResult> match(std::string_view input) const override {
        pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(regex_, nullptr);
        int rc = pcre2_match(regex_,
                             (PCRE2_SPTR)input.data(), input.size(),
                             0, 0, match_data, nullptr);

        if (rc < 0) {
            pcre2_match_data_free(match_data);
            return std::nullopt;
        }

        MatchResult result;
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
        for (uint32_t i = 0; i < name_count_; ++i) {
            PCRE2_SPTR entry = name_table_ + i * name_entry_size_;
            uint16_t group_index = (entry[0] << 8) | entry[1];
            std::string name((char*)entry + 2);
            PCRE2_SIZE start = ovector[2 * group_index];
            PCRE2_SIZE end = ovector[2 * group_index + 1];
            result.named[name] = input.substr(start, end - start);
        }

        pcre2_match_data_free(match_data);
        return result;
    }

private:
    pcre2_code* regex_ = nullptr;
    uint32_t name_count_ = 0;
    PCRE2_SPTR name_table_ = nullptr;
    uint32_t name_entry_size_ = 0;
};

//
// Factory for regex
//

std::unique_ptr<Regex> Regex::createRegex(const std::string& pattern) {
    return std::make_unique<PCRE2Regex>(pattern);
}

const std::string Regex::SpringBoot = R"((?<date>\S+)\s+)"
                                R"((?<level>[\w]+)?\s*)"
                                R"((?<pid>\d+)?\s*---\s*)"
                                R"((\[(?<thread>[^\]]*)\])\s*)"
//                                R"((\[(?<reason>[^\]]*)\])?\s*)"
                                R"((?<logger>[^: ]*)\s*:\s*)"
                                R"((?<message>.*))";

//R"(   (?<date>\S+)    \s+   (?<level>[\w]+)?   \s+   (?<pid>\d+)?   \s*---\s*    (\[(?<thread>[^\]]*)\])?    \s*    (?<logger>[^: ]*)    \s*:\s*    (?<message>.*))"

//
// Log parser
//

void Parser::ParseLogFiles(const std::vector<std::string>& paths)
{
    for (auto path : paths)
    {
        ParseLogFile(path);
    }

    _data.Synchronize();
}

void Parser::ParseLogFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open() && !file.good())
    {
        // TODO Log error
        std::cerr << "Cannot open file " << path << std::endl;
        return;
    }
    Parse(_files.GetFile(path));
}

void Parser::Parse(FileDescriptor& fd)
{
    std::ifstream file(fd.path);
    if (!file.is_open() && !file.good())
    {
        // TODO Log error
        std::cerr << "Cannot open file " << fd.path << std::endl;
        return;
    }
/*
    _fileDesc = &fd;

    _tempExtra.clear();

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        if (line.empty() && file.eof()) {
            break;
        }
        ParseLogLine(line);
    }
    AppendExtraLine();

    _fileDesc = nullptr;
*/

    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::vector<ParsedEntry> entries = ParseLogs(file_content, Regex::createRegex(Regex::SpringBoot));
    for (const auto& entry : entries) {
        _data.AddLog(fd.id, entry);
    }
}



std::vector<ParsedEntry> Parser::ParseLogs(std::string_view input, std::shared_ptr<Regex> regex) {
    std::vector<ParsedEntry> entries;
    size_t start = 0;
    ParsedEntry* current = nullptr;

    while (start < input.size()) {
        size_t end = input.find_first_of("\n", start);
        if (end == std::string_view::npos) {
            end = input.size();
        }

        std::string_view line = input.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        auto match = regex->match(line);
        if (match.has_value()) {
            ParsedEntry entry;
            auto& named = match->named;

            if (auto it = named.find("date"); it != named.end()) {
                entry.date = ParseTimestamp(std::string(it->second));
            }
            if (auto it = named.find("level"); it != named.end()) {
                entry.level = ParseLogLevel(it->second);
            }
            if (auto it = named.find("pid"); it != named.end() && !it->second.empty()) {
                long value;
                if (std::from_chars(it->second.data(), it->second.data() + it->second.size(), value).ec == std::errc()) {
                    entry.pid = value;
                }
            }
            if (auto it = named.find("thread"); it != named.end()) {
                entry.thread = it->second;
            }
            if (auto it = named.find("logger"); it != named.end()) {
                entry.logger = it->second;
            }
            if (auto it = named.find("message"); it != named.end()) {
                entry.message = it->second;
            }

            entries.push_back(std::move(entry));
            current = &entries.back();
        } else if (current) {
// TODO Support extra line data
/*
            // Append line to current extra message
            auto msg_begin = current->message.data();
            auto msg_size = current->message.size();
            auto appended_size = msg_size + 1 + line.size();

            std::string full_message(msg_begin, msg_size);
            full_message += '\n';
            full_message += line;

            current->message = std::string_view(full_message); // efficiency lost, safe fallback
*/
        } else {
            ParsedEntry fallback;
            fallback.level = LogLevel::LOG_UNKNOWN;
            fallback.message = line;
            entries.push_back(std::move(fallback));
            current = &entries.back();
        }

        start = end + 1;
    }

    return entries;
}



void Parser::ParseLogLine(const std::string& line)
{
    if (!line.empty())
    {
        std::vector<std::string> arr = SplitLine(line);
        if (!arr.empty())
        {
            if (arr[0].size() < 28)
                // 28 : arbitrary value greater than any supported text date length
            {
                if (arr.size() == 3)
                {
                    AddLogLine(arr[0], arr[1], arr[2]);
                    return;
                }
                else if (arr.size() == 6)
                {
                    AddLogLine(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
                    return;
                }
            }
            // Consider as extra line
            _tempExtra.append(line).append("\n");
        }
    }
}

std::vector<std::string> Parser::SplitLine(const std::string& line)
{
    std::vector<std::string> arr;
    size_t cur = 0;
    size_t pos = 0;
    while (pos = line.find(" | ", cur), pos != std::string::npos)
    {
        arr.push_back(line.substr(cur, pos - 1));
        cur = pos + 3;
        if (cur >= line.size()) {
            break;
        }
    }
    if (cur<line.size()) {
        arr.push_back(line.substr(cur));
    }
    return arr;
}

void Parser::AppendExtraLine()
{
    if (!_tempExtra.empty())
    {
        if (_data.EntryCount()>0)
        {
            _data.back().extra = _tempExtra;
        }
        _tempExtra.clear();
    }
}

void Parser::AddLogLine(const std::string& date, const std::string& level, const std::string& thread, const std::string& logger, const std::string& source, const std::string& message)
{
    AppendExtraLine();
    _data.AddLog(
            _fileDesc->id,
            ParseTimestamp(Trim(date)),
            ParseLogLevel(Trim(level)),
            -1, // pid not available
        thread,
            logger,
            source,
            message
    );
}

void Parser::AddLogLine(const std::string& date, const std::string& logger, const std::string& message)
{
    AppendExtraLine();
    _data.AddLog(
            _fileDesc->id,
            ParseTimestamp(Trim(date)),
            LogLevel::LOG_INFO,
            -1, // pid not available
            "",
            logger,
            "",
            message
    );
}

std::string_view Parser::Trim(const std::string_view& str)
{
    size_t first = 0;
    size_t last = str.size();

    while (first < last && (str[first] == ' ' || str[first] == '\t')) ++first;
    while (last > first && (str[last - 1] == ' ' || str[last - 1] == '\t')) --last;

    return std::string_view(str.data() + first, last - first);
}

std::string Parser::Trim(const std::string& str)
{
    size_t first = 0;
    size_t last = str.size();

    while (first < last && (str[first] == ' ' || str[first] == '\t')) ++first;
    while (last > first && (str[last - 1] == ' ' || str[last - 1] == '\t')) --last;

    return std::string(str.data() + first, last - first);
}


LogLevel Parser::ParseLogLevel(const std::string_view& str)
{
    if (str.starts_with('T') /*== "TRACE"*/) return LogLevel::LOG_TRACE;
    if (str.starts_with('D') /*== "DEBUG"*/) return LogLevel::LOG_DEBUG;
    if (str.starts_with('I')/* == "INFO"*/) return LogLevel::LOG_INFO;
    if (str.starts_with('W')/* == "WARN"*/) return LogLevel::LOG_WARNING;
    if (str.starts_with('E')/* == "ERROR"*/) return LogLevel::LOG_ERROR;
    if (str.starts_with('C')/* == "CRITICAL"*/) return LogLevel::LOG_CRITICAL;
    if (str.starts_with('F')/* == "FATAL"*/) return LogLevel::LOG_FATAL;
    return LogLevel::LOG_UNKNOWN;
}

Timestamp Parser::ParseTimestamp(const std::string& str)
{
    return ParseISO8601(Trim(str));
}

Timestamp Parser::ParseISO8601(const std::string& date) {
    static const std::regex iso8601_regex(
            R"(^(\d{4})(?:-?(\d{2})(?:-?(\d{2}))?)?)"         // YYYY[-MM[-DD]]
            R"((?:[T ](\d{2})(?::?(\d{2})(?::?(\d{2}))?)?)"   // [T ]HH[:MM[:SS]]
            R"((?:\.(\d{1,3}))?)?)"                           // [.fff]
            R"((?:(Z)|([+-])(\d{2}):?(\d{2}))?$)"            // [Z|±HH:MM]
    );

    std::smatch match;
    if (!std::regex_match(date, match, iso8601_regex)) {
        // throw ISO8601ParseError("Invalid date/time format");
        return Timestamp();
    }

    int year = std::stoi(match[1].str());
    int month = match[2].matched ? std::stoi(match[2].str()) : 1;
    int day = match[3].matched ? std::stoi(match[3].str()) : 1;
    int hour = match[4].matched ? std::stoi(match[4].str()) : 0;
    int minute = match[5].matched ? std::stoi(match[5].str()) : 0;
    int second = match[6].matched ? std::stoi(match[6].str()) : 0;

    int milliseconds = 0;
    if (match[7].matched) {
        std::string ms_str = match[7].str();
        if (ms_str.length() == 1) ms_str += "00";
        else if (ms_str.length() == 2) ms_str += "0";
        else if (ms_str.length() > 3) ms_str = ms_str.substr(0, 3);
        milliseconds = std::stoi(ms_str);
    }

    if (month < 1 || month > 12) {
        //throw ISO8601ParseError("Invalid month: " + std::to_string(month));
        return Timestamp();
    }
    if (day < 1 || day > 31) {
        //throw ISO8601ParseError("Invalid day: " + std::to_string(day));
        return Timestamp();
    }
    if (hour < 0 || hour > 23) {
        //throw ISO8601ParseError("Invalid hour: " + std::to_string(hour));
        return Timestamp();
    }
    if (minute < 0 || minute > 59) {
        //throw ISO8601ParseError("Invalid minute: " + std::to_string(minute));
        return Timestamp();
    }
    if (second < 0 || second > 59) {
        //throw ISO8601ParseError("Invalid second: " + std::to_string(second));
        return Timestamp();
    }

    // Create UTC time_point
    std::tm tm_utc = {};
    tm_utc.tm_year = year - 1900;
    tm_utc.tm_mon = month - 1;
    tm_utc.tm_mday = day;
    tm_utc.tm_hour = hour;
    tm_utc.tm_min = minute;
    tm_utc.tm_sec = second;
    tm_utc.tm_isdst = 0;

    // Convert to time_t UTC
    std::time_t time_utc = std::mktime(&tm_utc);
    if (time_utc == -1) {
        //throw ISO8601ParseError("Invalid date/time combination");
        return Timestamp();
    }

    // Ajust for UTC (mktime assume local time)
    std::time_t time_local = std::mktime(&tm_utc);
    std::tm* tm_gmt = std::gmtime(&time_local);
    std::tm* tm_local = std::localtime(&time_local);

    if (tm_gmt && tm_local) {
        // Calc offset timezone local
        std::time_t offset = std::mktime(tm_local) - std::mktime(tm_gmt);
        time_utc = time_local - offset;
    }

    auto tp = std::chrono::system_clock::from_time_t(time_utc);
    tp += std::chrono::milliseconds(milliseconds);

    // Timezone handling
    if (match[8].matched) {
        // 'Z' = UTC
    } else if (match[9].matched) {
        // Explicit timezone ±HH:MM
        bool is_positive = (match[9].str() == "+");
        int tz_hours = std::stoi(match[10].str());
        int tz_minutes = std::stoi(match[11].str());

        if (tz_hours > 14 || (tz_hours == 14 && tz_minutes > 0)) {
            //throw ISO8601ParseError("Invalid timezone offset");
            return Timestamp();
        }

        auto offset = std::chrono::hours(tz_hours) + std::chrono::minutes(tz_minutes);
        if (is_positive) {
            tp -= offset;
        } else {
            tp += offset;
        }
    }
    // else // No timezone specified, assume local time

    return tp;
}

