#include "catch.hpp"
//#define CATCH_CONFIG_MAIN
#include <iostream>
#include "Main_Algorithm.hpp"



TEST_CASE("Test: Main Algorithm") {

    Main_Algorithm mainAlgo;
    int16_t actual = mainAlgo.Runner();
    int16_t expected = 1;

    REQUIRE(actual == expected);


}

