#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <subgraph.hpp>
#include <cmath>

struct element
{
    double x,y;
};

double distance_between_elems(const element & e1, const element & e2)
{
    double d = (e1.x - e2.x)*(e1.x - e2.x) + (e1.y - e2.y)*(e1.y - e2.y);
    return std::sqrt(d);
}

TEST_CASE("subgraph creation and adding three elems - only to input buffer", "[subgraph]")
{
    stream::subgraph<element> sg(5, distance_between_elems);

    element e1{10.3,15.1};
    element e2{5.1,3.4};
    element e3{7.4,9.6};

    sg.add_element(e1);
    sg.add_element(e2);
    sg.add_element(e3);

    REQUIRE( sg.chunks_num() == 0);
    REQUIRE( sg.input_buffer_size() == 3);
}

TEST_CASE("subgraph creation and adding five elems - first simple clusterization model", "[subgraph]")
{
    stream::subgraph<element> sg(5, distance_between_elems);

    element e1{1.0,1.0};
    element e2{1.0,5.0};
    element e3{4.0,5.0};
    element e4{4.0,7.0};
    element e5{5.0,5.0};

    sg.add_element(e1);
    sg.add_element(e2);
    sg.add_element(e3);
    sg.add_element(e4);
    sg.add_element(e5);

    REQUIRE(sg.chunks_num() == 1);
    REQUIRE(sg.input_buffer_size() == 0);

    REQUIRE( sg.get_model()[0] == stream::subgraph<element>::link(2,4,1.0) );
    REQUIRE( sg.get_model()[1] == stream::subgraph<element>::link(2,3,2.0) );
    REQUIRE( sg.get_model()[2] == stream::subgraph<element>::link(1,2,3.0) );
    REQUIRE( sg.get_model()[3] == stream::subgraph<element>::link(0,1,4.0) );
}

