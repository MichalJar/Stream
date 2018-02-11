#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <subgraph.hpp>
#include <cmath>
#include <algorithm>


#define TEST_LINK_EXISTANCE(links, a, b, length) REQUIRE(test_links(links, a,b,length));

struct element
{
    double x,y;
};

double distance_between_elems(const element & e1, const element & e2)
{
    double d = (e1.x - e2.x)*(e1.x - e2.x) + (e1.y - e2.y)*(e1.y - e2.y);
    return std::sqrt(d);
}


bool test_links(const std::vector<stream::subgraph<element>::link> & links, unsigned a, unsigned b, double length)
{
    using link = stream::subgraph<element>::link;
    return std::count(std::begin(links), std::end(links), link(a,b,length)) == 1 or std::count(std::begin(links), std::end(links), link(b,a,length)) == 1;
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

TEST_CASE("subgraph creation and adding 16 elements - after every chunk - the model should be created / updated", "[subgraph]")
{
    stream::subgraph<element> sg(4, distance_between_elems);

    element e0{1.0,2.0};
    element e1{1.0,3.0};
    element e2{1.0,5.0};
    element e3{3.0,5.0};
    
    element e4{4.0,7.0};
    element e5{4.0,8.0};
    element e6{5.0,10.0};
    element e7{5.0,13.0};

    element e8{6.0,14.0};    
    element e9{13.0,14.0};
    element e10{15.0,14.0};
    element e11{13.0,13.0};

    sg.add_element(e0);
    sg.add_element(e1);
    sg.add_element(e2);
    sg.add_element(e3);

    //

    TEST_LINK_EXISTANCE(sg.get_model(), 0,1,1.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 1,2,2.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 2,3,2.0);

    sg.add_element(e4);
    sg.add_element(e5);
    sg.add_element(e6);
    sg.add_element(e7);


    TEST_LINK_EXISTANCE(sg.get_model(), 0,1,1.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 1,2,2.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 2,3,2.0);

    TEST_LINK_EXISTANCE(sg.get_model(), 3,4,sqrt(5));
    TEST_LINK_EXISTANCE(sg.get_model(), 4,5,1.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 5,6,sqrt(5.0));
    TEST_LINK_EXISTANCE(sg.get_model(), 6,7,3.0);

    sg.add_element(e8);
    sg.add_element(e9);
    sg.add_element(e10);
    sg.add_element(e11);

    TEST_LINK_EXISTANCE(sg.get_model(), 0,1,1.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 1,2,2.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 2,3,2.0);

    TEST_LINK_EXISTANCE(sg.get_model(), 3,4,sqrt(5));
    TEST_LINK_EXISTANCE(sg.get_model(), 4,5,1.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 5,6,sqrt(5.0));
    TEST_LINK_EXISTANCE(sg.get_model(), 6,7,3.0);

    TEST_LINK_EXISTANCE(sg.get_model(), 7,8,sqrt(2));
    TEST_LINK_EXISTANCE(sg.get_model(), 8,9,7.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 9,10,2.0);
    TEST_LINK_EXISTANCE(sg.get_model(), 9,11,1.0);
}