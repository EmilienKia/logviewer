#include "catch.hpp"
#include "../src/parser.hpp"


TEST_CASE("Simple log parsing") {

    std::shared_ptr<Regex> regex = Regex::createRegex(
            Regex::SpringBoot
    );

    std::string log = "2025-07-06T12:34:56.789 INFO 12345 --- [main] [  restartedMain] com.example.Demo : Started DemoApplication";

    auto parsed = Parser::ParseLogs(log, regex);

    REQUIRE(parsed.size() == 1);
    CHECK(parsed[0].level == LOG_INFO);
    CHECK(parsed[0].pid == 12345);
    CHECK(parsed[0].thread == "main");
    CHECK(parsed[0].logger == "com.example.Demo");
    CHECK(parsed[0].message == "Started DemoApplication");
}


TEST_CASE("SpringBoot log parsing", "[parser][springboot]") {
    std::shared_ptr<Regex> regex = Regex::createRegex(Regex::SpringBoot);

    SECTION("Spring Boot log - single well-formed line", "[logparser][springboot]") {
        std::string log = "2025-07-06T12:34:56.789 INFO 12345 --- [main] com.example.Demo : Started DemoApplication";

        auto parsed = Parser::ParseLogs(log, regex);

        REQUIRE(parsed.size() == 1);
// TODO Enable this again
//        CHECK(parsed[0].date == "2025-07-06T12:34:56.789");
        CHECK(parsed[0].level == LOG_INFO);
        CHECK(parsed[0].pid == 12345);
        CHECK(parsed[0].thread == "main");
        CHECK(parsed[0].logger == "com.example.Demo");
        CHECK(parsed[0].message == "Started DemoApplication");
    }

    SECTION("Spring Boot log - multi-line message", "[logparser][springboot]") {
        std::string log =
                "2025-07-06T12:00:00.000 ERROR 4242 --- [main] com.example.Service : Exception occurred\n"
                "java.lang.RuntimeException: Boom\n"
                "\tat com.example.Service.doSomething(Service.java:42)\n"
                "\tat com.example.App.main(App.java:10)";

        auto parsed = Parser::ParseLogs(log, regex);

        REQUIRE(parsed.size() == 1);
        CHECK(parsed[0].level == LOG_ERROR);
// TODO Enable this again
//        CHECK(parsed[0].message.find("Exception occurred") != std::string::npos);
//        CHECK(parsed[0].message.find("RuntimeException") != std::string::npos);
//        CHECK(parsed[0].message.find("App.main") != std::string::npos);
    }

    SECTION("Spring Boot log - missing PID", "[logparser][springboot]") {
        std::string log =
                "2025-07-06T12:01:00.123 WARN  --- [main] com.example.MissingPID : PID was missing";

        auto parsed = Parser::ParseLogs(log, regex);

        REQUIRE(parsed.size() == 1);
        CHECK(parsed[0].level == LOG_WARNING);
        CHECK(parsed[0].pid == 0);
        CHECK(parsed[0].logger == "com.example.MissingPID");
    }

    SECTION("Spring Boot log - unmatched first line fallback", "[logparser][springboot]") {
        std::string log =
                "This is an unmatched first line\n"
                "2025-07-06T13:00:00.000 INFO 1111 --- [main] com.example.Parser : Then we match";

        auto parsed = Parser::ParseLogs(log, regex);

        REQUIRE(parsed.size() == 2);
        CHECK(parsed[0].message == "This is an unmatched first line");
        CHECK(parsed[1].level == LOG_INFO);
        CHECK(parsed[1].logger == "com.example.Parser");
    }

    SECTION("Spring Boot log - multiple entries", "[logparser][springboot]") {
        std::string log =
                "2025-07-06T10:00:00.000 DEBUG 8888 --- [worker-1] com.example.Job : Job started\n"
                "2025-07-06T10:00:01.000 INFO 8888 --- [worker-1] com.example.Job : Job finished";

        auto parsed = Parser::ParseLogs(log, regex);

        REQUIRE(parsed.size() == 2);
        CHECK(parsed[0].level == LOG_DEBUG);
        CHECK(parsed[1].level == LOG_INFO);
        CHECK(parsed[0].logger == "com.example.Job");
        CHECK(parsed[1].logger == "com.example.Job");
    }

}

TEST_CASE("Parse Log4j2 formatted log", "[parser][log4j2]") {
    std::shared_ptr<Regex> regex = Regex::createRegex(Regex::Log4J2);

    std::string_view log =
            "12:34:56.789 [main] INFO  com.example.MyClass - Application started\n"
            "12:34:57.001 [main] ERROR com.example.MyClass - Something failed\n"
            "Stack trace follows:\n"
            "java.lang.Exception: Dummy\n";

    auto entries = Parser::ParseLogs(log, regex);

    REQUIRE(entries.size() == 2);
    REQUIRE(entries[0].level == LOG_INFO);
    REQUIRE(entries[0].thread == "main");
    REQUIRE(entries[0].logger == "com.example.MyClass");
    REQUIRE(entries[0].message == "Application started");

    REQUIRE(entries[1].level == LOG_ERROR);
    REQUIRE(entries[1].message.starts_with("Something failed"));
    REQUIRE(entries[1].message.find("Dummy") == std::string_view::npos);
}







