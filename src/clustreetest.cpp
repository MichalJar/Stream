#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "clustree.hpp"
#include <cmath>

struct cluster_feature
{
    double ls[2];
    double ss[2];
    int num;
};

void update_cf(cluster_feature & old_cf, const cluster_feature & new_cf)
{
    old_cf.ls[0] += new_cf.ls[0];
    old_cf.ls[1] += new_cf.ls[1];
    old_cf.ss[0] += new_cf.ss[0];
    old_cf.ss[1] += new_cf.ss[1];
    old_cf.num += new_cf.num;
}

double distance_between_cf(const cluster_feature & a, const cluster_feature & b)
{
    auto d0 = a.ls[0]/a.num - b.ls[0] / b.num;
    auto d1 = a.ls[1]/a.num - b.ls[1] / b.num;
    return std::sqrt(d0*d0 + d1*d1);
}

TEST_CASE("clustree creation", "[clustree]")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);
}

TEST_CASE("add element to clustree", "[clustree]")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);
    cluster_feature ncf = {{2.0,1.0},{4.0,1.0},1};

    ct.add_element(ncf);

    REQUIRE( ct.root->cluster_features[0].ls[0] == 2.0 );
    REQUIRE( ct.root->cluster_features[0].ls[1] == 1.0 );
    REQUIRE( ct.root->cluster_features[0].ss[0] == 4.0 );
    REQUIRE( ct.root->cluster_features[0].ss[1] == 1.0 );
    REQUIRE( ct.root->cluster_features[0].num == 1 );
}

TEST_CASE("add two elements to clustree -> merge", "[clustree]")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);

    cluster_feature ncf1 = {{2.0,1.0},{4.0,1.0},1};
    cluster_feature ncf2 = {{4.0,2.0},{16.0,1.0},1};

    ct.add_element(ncf1);
    ct.add_element(ncf2);

    REQUIRE( ct.root->cluster_features[0].ls[0] == 6.0 );
    REQUIRE( ct.root->cluster_features[0].ls[1] == 3.0 );
    REQUIRE( ct.root->cluster_features[0].ss[0] == 20.0 );
    REQUIRE( ct.root->cluster_features[0].ss[1] == 2.0 );
    REQUIRE( ct.root->cluster_features[0].num == 2 );
}

TEST_CASE("add two elements to clustree -> two cfs in root", "[clustree]")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);

    cluster_feature ncf1 = {{2.0,1.0},{4.0,1.0},1};
    cluster_feature ncf2 = {{4.0,4.0},{16.0,1.0},1};

    ct.add_element(ncf1);
    ct.add_element(ncf2);

    REQUIRE( ct.root->cluster_features.size() == 2 );

    REQUIRE( ct.root->cluster_features[0].ls[0] == 2.0 );
    REQUIRE( ct.root->cluster_features[0].ls[1] == 1.0 );
    REQUIRE( ct.root->cluster_features[0].ss[0] == 4.0 );
    REQUIRE( ct.root->cluster_features[0].ss[1] == 1.0 );
    REQUIRE( ct.root->cluster_features[0].num == 1 );

    REQUIRE( ct.root->cluster_features[1].ls[0] == 4.0 );
    REQUIRE( ct.root->cluster_features[1].ls[1] == 4.0 );
    REQUIRE( ct.root->cluster_features[1].ss[0] == 16.0 );
    REQUIRE( ct.root->cluster_features[1].ss[1] == 1.0 );
    REQUIRE( ct.root->cluster_features[1].num == 1 );
}

TEST_CASE("add three elements to clustree -> two cfs in root and third in new leaf","[clustree]")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);

    cluster_feature ncf1 = {{2.0,1.0},{4.0,1.0},1};
    cluster_feature ncf2 = {{5.0,5.0},{25.0,25.0},1};
    cluster_feature ncf3 = {{0.0,-2.0},{0.0,4.0},1};

    ct.add_element(ncf1);
    ct.add_element(ncf2);
    ct.add_element(ncf3);

    REQUIRE( ct.root->cluster_features.size() == 2 );
    REQUIRE( ct.root->childs.size() == 2 );
    REQUIRE( ct.root->childs[0] != nullptr );
    REQUIRE( ct.root->childs[1] == nullptr );

    REQUIRE( ct.root->cluster_features[0].ls[0] == 2.0 );
    REQUIRE( ct.root->cluster_features[0].ls[1] == -1.0 );
    REQUIRE( ct.root->cluster_features[0].ss[0] == 4.0 );
    REQUIRE( ct.root->cluster_features[0].ss[1] == 5.0 );
    REQUIRE( ct.root->cluster_features[0].num == 2 );

    REQUIRE( ct.root->cluster_features[1].ls[0] == 5.0 );
    REQUIRE( ct.root->cluster_features[1].ls[1] == 5.0 );
    REQUIRE( ct.root->cluster_features[1].ss[0] == 25.0 );
    REQUIRE( ct.root->cluster_features[1].ss[1] == 25.0 );
    REQUIRE( ct.root->cluster_features[1].num == 1 );

    REQUIRE( ct.root->childs[0]->cluster_features.size() == 2 );

    REQUIRE( ct.root->childs[0]->cluster_features[0].ls[0] == 2.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ls[1] == 1.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ss[0] == 4.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ss[1] == 1.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].num == 1 );

    REQUIRE( ct.root->childs[0]->cluster_features[1].ls[0] == 0.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ls[1] == -2.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ss[0] == 0.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ss[1] == 4.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].num == 1 );
}

TEST_CASE("add element to tree with already three elements -> the older elements shoud be merged during necessary split operation")
{
    stream::clustree<cluster_feature> ct(2,3.0,update_cf, distance_between_cf);

    cluster_feature ncf1 = {{2.0,1.0},{4.0,1.0},1};
    cluster_feature ncf2 = {{5.0,5.0},{25.0,25.0},1};
    cluster_feature ncf3 = {{0.0,-2.0},{0.0,4.0},1};

    ct.add_element(ncf1);
    ct.add_element(ncf2);
    ct.add_element(ncf3);

    cluster_feature ncf4 = {{-2.5, -5.2},{6.25, 27.04},1};
    ct.add_element(ncf4);

    REQUIRE( ct.root->childs[0]->cluster_features.size() == 2 );

    REQUIRE( ct.root->childs[0]->childs[0] != nullptr);
    REQUIRE( ct.root->childs[0]->childs[1] == nullptr);

    REQUIRE( ct.root->childs[0]->cluster_features[0].ls[0] == 2.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ls[1] == -1.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ss[0] == 4.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].ss[1] == 5.0 );
    REQUIRE( ct.root->childs[0]->cluster_features[0].num == 2 );

    REQUIRE( ct.root->childs[0]->cluster_features[1].ls[0] == -2.5 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ls[1] == -5.2 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ss[0] == 6.25 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].ss[1] == 27.04 );
    REQUIRE( ct.root->childs[0]->cluster_features[1].num == 1 );

    REQUIRE( ct.root->childs[0]->childs[0] != nullptr);
    REQUIRE( ct.root->childs[0]->childs[1] == nullptr);

    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[0].ls[0] == 2.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[0].ls[1] == 1.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[0].ss[0] == 4.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[0].ss[1] == 1.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[0].num == 1);

    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[1].ls[0] == 0.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[1].ls[1] == -2.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[1].ss[0] == 0.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[1].ss[1] == 4.0);
    REQUIRE( ct.root->childs[0]->childs[0]->cluster_features[1].num == 1);
}