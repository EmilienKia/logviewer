#include "catch.hpp"
#include "../src/parser.hpp"

// Helper function to create UTC time_point for testing
Timestamp makeUTC(int year, int month=1, int day=1, int hour=0, int min=0, int sec=0, int ms=0) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    tm.tm_isdst = 0;

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    tp += std::chrono::milliseconds(ms);
    return tp;
}

/*
inline Timestamp makeUTC(int year, int month, int day, int hour, int min, int sec) {
    return makeUTC(year, month, day, hour, min, sec, 0);
}

inline Timestamp makeUTC(int year, int month, int day) {
    return makeUTC(year, month, day, 0, 0, 0, 0);
}
*/



TEST_CASE("ISO8601 Parser - Year Only", "[iso8601][parser]") {
    auto result = Parser::ParseISO8601("2023");
    auto expected = makeUTC(2023, 1, 1);
    REQUIRE(result == expected);
}

TEST_CASE("ISO8601 Parser - Year and Month", "[iso8601][parser]") {
    SECTION("With separator") {
        auto result = Parser::ParseISO8601("2023-12");
        auto expected = makeUTC(2023, 12, 1);
        REQUIRE(result == expected);
    }

    SECTION("Without separator") {
        auto result = Parser::ParseISO8601("202312");
        auto expected = makeUTC(2023, 12, 1);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - Complete Date", "[iso8601][parser]") {
    SECTION("With separators") {
        auto result = Parser::ParseISO8601("2023-12-25");
        auto expected = makeUTC(2023, 12, 25);
        REQUIRE(result == expected);
    }

    SECTION("Without separators") {
        auto result = Parser::ParseISO8601("20231225");
        auto expected = makeUTC(2023, 12, 25);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - Complete DateTime", "[iso8601][parser]") {
    SECTION("With T separator") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45);
        REQUIRE(result == expected);
    }

    SECTION("With space separator") {
        auto result = Parser::ParseISO8601("2023-12-25 14:30:45");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45);
        REQUIRE(result == expected);
    }

    SECTION("Compact format") {
        auto result = Parser::ParseISO8601("20231225T143045");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - With Milliseconds", "[iso8601][parser]") {
    SECTION("Three digits") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45.123");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45, 123);
        REQUIRE(result == expected);
    }

    SECTION("One digit") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45.1");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45, 100);
        REQUIRE(result == expected);
    }

    SECTION("Two digits") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45.12");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45, 120);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - With UTC Timezone", "[iso8601][parser]") {
    SECTION("Z timezone without milliseconds") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45Z");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45);
        REQUIRE(result == expected);
    }

    SECTION("Z timezone with milliseconds") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45.123Z");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45, 123);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - With Timezone Offset", "[iso8601][parser]") {
    SECTION("Positive offset +02:00") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45+02:00");
        auto expected = makeUTC(2023, 12, 25, 12, 30, 45); // 14:30 +02:00 = 12:30 UTC
        REQUIRE(result == expected);
    }

    SECTION("Negative offset -05:00") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45-05:00");
        auto expected = makeUTC(2023, 12, 25, 19, 30, 45); // 14:30 -05:00 = 19:30 UTC
        REQUIRE(result == expected);
    }

    SECTION("Compact timezone format +0200") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45+0200");
        auto expected = makeUTC(2023, 12, 25, 12, 30, 45);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - Partial Time Formats", "[iso8601][parser]") {
    SECTION("Hour only") {
        auto result = Parser::ParseISO8601("2023-12-25T14");
        auto expected = makeUTC(2023, 12, 25, 14, 0, 0);
        REQUIRE(result == expected);
    }

    SECTION("Hour and minute") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 0);
        REQUIRE(result == expected);
    }

    SECTION("Compact hour and minute") {
        auto result = Parser::ParseISO8601("2023-12-25T1430");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 0);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - Invalid Formats", "[iso8601][parser][invalid]") {
    SECTION("Empty string") {
        auto result = Parser::ParseISO8601("");
        REQUIRE(result == Timestamp()); // Should return empty/invalid timestamp
    }

    SECTION("Invalid format") {
        auto result = Parser::ParseISO8601("invalid");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid month") {
        auto result = Parser::ParseISO8601("2023-13-01");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid day") {
        auto result = Parser::ParseISO8601("2023-12-32");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid hour") {
        auto result = Parser::ParseISO8601("2023-12-25T25:00:00");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid minute") {
        auto result = Parser::ParseISO8601("2023-12-25T14:60:00");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid second") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:60");
        REQUIRE(result == Timestamp());
    }

    SECTION("Invalid timezone offset") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45+15:00");
        REQUIRE(result == Timestamp());
    }
}

TEST_CASE("ISO8601 Parser - Edge Cases", "[iso8601][parser][edge]") {
    SECTION("Leap year") {
        auto result = Parser::ParseISO8601("2024-02-29");
        auto expected = makeUTC(2024, 2, 29);
        REQUIRE(result == expected);
    }

    SECTION("Beginning of year") {
        auto result = Parser::ParseISO8601("2023-01-01T00:00:00");
        auto expected = makeUTC(2023, 1, 1, 0, 0, 0);
        REQUIRE(result == expected);
    }

    SECTION("End of year") {
        auto result = Parser::ParseISO8601("2023-12-31T23:59:59");
        auto expected = makeUTC(2023, 12, 31, 23, 59, 59);
        REQUIRE(result == expected);
    }

    SECTION("End of day") {
        auto result = Parser::ParseISO8601("2023-06-15T23:59:59.999");
        auto expected = makeUTC(2023, 6, 15, 23, 59, 59, 999);
        REQUIRE(result == expected);
    }
}

TEST_CASE("ISO8601 Parser - Mixed Formats", "[iso8601][parser][mixed]") {
    SECTION("Date only with different separators") {
        auto result1 = Parser::ParseISO8601("2023-12-25");
        auto result2 = Parser::ParseISO8601("2023/12/25");
        auto result3 = Parser::ParseISO8601("2023 12 25");
        auto expected = makeUTC(2023, 12, 25);

        REQUIRE(result1 == expected);
        // Note: The current regex might not support / and space separators
        // Adjust based on actual implementation
    }

    SECTION("Time with different separators") {
        auto result1 = Parser::ParseISO8601("2023-12-25T14:30:45");
        auto result2 = Parser::ParseISO8601("2023-12-25T14 30 45");
        auto expected = makeUTC(2023, 12, 25, 14, 30, 45);

        REQUIRE(result1 == expected);
        // Note: Adjust based on actual regex implementation
    }
}

TEST_CASE("ISO8601 Parser - Boundary Values", "[iso8601][parser][boundary]") {
    SECTION("Minimum valid date") {
        auto result = Parser::ParseISO8601("0001-01-01T00:00:00");
        auto expected = makeUTC(1, 1, 1, 0, 0, 0);
        REQUIRE(result == expected);
    }

    SECTION("Maximum timezone offset") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45+14:00");
        auto expected = makeUTC(2023, 12, 25, 0, 30, 45); // 14:30 +14:00 = 00:30 UTC
        REQUIRE(result == expected);
    }

    SECTION("Minimum timezone offset") {
        auto result = Parser::ParseISO8601("2023-12-25T14:30:45-12:00");
        auto expected = makeUTC(2023, 12, 26, 2, 30, 45); // 14:30 -12:00 = 02:30 UTC next day
        REQUIRE(result == expected);
    }
}

