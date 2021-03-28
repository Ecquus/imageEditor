#include "catch.hpp"
#include <util.h>

TEST_CASE("Test clamp", "[util/clamp]")
{
    CHECK(util::clamp(-1 , 0, 255) == 0);
    CHECK(util::clamp(100, 0, 255) == 100);
    CHECK(util::clamp(300, 0, 255) == 255);
}

TEST_CASE("Test history", "[util/history]")
{
    SECTION("Test size")
    {
        auto hist = util::history<uint, 10u>{ 1u };

        REQUIRE(hist.size() == 1u);

        for (uint i = 2; i < 11; ++i)
            hist.append(i);

        REQUIRE(hist.size() == 10u);

        hist.append(42u);
        hist.append(42u);
        hist.append(42u);

        REQUIRE(hist.size() == 10u);
        REQUIRE(hist.undo() == 42u);
        REQUIRE(hist.undo() == 42u);
        REQUIRE(hist.undo() == 10u);
    }
    SECTION("Test undo for underflow")
    {
        auto hist = util::history<uint, 10u>{ 1u };

        hist.append(2u);
        hist.append(3u);
        REQUIRE(hist.undo() == 2u);
        REQUIRE(hist.undo() == 1u);
        REQUIRE(hist.undo() == 1u);
    }
    SECTION("Test redo for overflow")
    {
        auto hist = util::history<uint, 10u>{ 1u };

        for (uint i = 2; i < 10; ++i)
            hist.append(i);

        REQUIRE(hist.redo() == 9u);
        REQUIRE(hist.redo() == 9u);
    }
    SECTION("Test append")
    {
                auto hist = util::history<uint, 10u>{ 1u };

        for (uint i = 2; i < 11; ++i)
            hist.append(i);

        REQUIRE(hist.size() == 10u);

        for (uint i = 0; i < 4; ++i)
            (void)hist.undo();

        hist.append(42u);

        REQUIRE(hist.undo() == 6u);
        REQUIRE(hist.redo() == 42u);
        REQUIRE(hist.undo() == 6u);
    }
}
