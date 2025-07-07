#include "catch.hpp"
#include "../src/parser.hpp"

TEST_CASE("PCRE2 Regex - basic named group match", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<word>\w+)\s+(?<number>\d+))");
    auto result = regex->match("hello 123");

    REQUIRE(result.has_value());
    CHECK(result->named.at("word") == "hello");
    CHECK(result->named.at("number") == "123");
}

TEST_CASE("PCRE2 Regex - no match", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<word>\w+)\s+(?<number>\d+))");
    auto result = regex->match("no-match-here");

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("PCRE2 Regex - optional group present", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<first>\w+)(\s+(?<second>\w+))?)");
    auto result = regex->match("hello world");

    REQUIRE(result.has_value());
    CHECK(result->named.at("first") == "hello");
    CHECK(result->named.at("second") == "world");
}

TEST_CASE("PCRE2 Regex - group boundaries", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<a>.)(?<b>.) (?<c>.+))");
    auto result = regex->match("ab cdef");

    REQUIRE(result.has_value());
    CHECK(result->named.at("a") == "a");
    CHECK(result->named.at("b") == "b");
    CHECK(result->named.at("c") == "cdef");
}

#if 0 // TODO Support advanced features

TEST_CASE("PCRE2 Regex - optional group missing", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<first>\w+)(\s+(?<second>\w+))?)");
    auto result = regex->match("hello");

    REQUIRE(result.has_value());
    CHECK(result->named.at("first") == "hello");
    CHECK(result->named.at("second").empty()); // Still exists but is empty
}

TEST_CASE("PCRE2 Regex - Unicode UTF-8 support", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<emoji>[\x{1F600}-\x{1F64F}]))"); // emoji range
    auto result = regex->match("😊");

    REQUIRE(result.has_value());
    CHECK(result->named.at("emoji") == "😊");
}

TEST_CASE("PCRE2 Regex - multiple matches (manual iteration)", "[pcre2]") {
    auto regex = Regex::createRegex(R"((?<word>\w+))");

    std::string input = "one two three";
    std::istringstream iss(input);
    std::string word;
    std::vector<std::string_view> results;

    while (iss >> word) {
        auto r = regex->match(word);
        REQUIRE(r.has_value());
        results.push_back(r->named.at("word"));
    }

    REQUIRE(results.size() == 3);
    CHECK(results[0] == "one");
    CHECK(results[1] == "two");
    CHECK(results[2] == "three");
}
#endif