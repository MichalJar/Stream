#include <iostream>
#include <vector>
#include <chrono>
#include "loader.hpp"
#include "clustree.hpp"

std::chrono::microseconds get_now()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
}

stream::StandardCF cf_from(const Element& e)
{
    stream::StandardCF cf;
    cf.ls[0] = e.val[0];
    cf.ls[1] = e.val[1];

    cf.ss[0] = e.val[0]*e.val[0];
    cf.ss[1] = e.val[1]*e.val[1];

    cf.n = 1;
    return cf;
}

int main(int argc, char * argv[])
{
    const std::string input_filename = argv[1];

    auto elems = load_elements_from(input_filename);
    stream::clustree<stream::StandardCF> cl(3, 0.5, stream::update_cf, stream::distance_between_cf);
    
    auto start = get_now();
    for(const auto& e: elems)
    {
        auto start = get_now();
        
        cl.add_element(cf_from(e));
        //std::cout << cl.get_node_num() << std::endl;
        auto end = get_now();

        std::chrono::microseconds distance = end - start;
        std::cout << "t," << distance.count() << std::endl;  
    }

    auto cfs = cl.get_cfs();
    cfs.shrink_to_fit();

    std::cout << "elems num: " << cl.get_node_num() << std::endl;
    std::cout << "nodes num: " << cfs.size() << std::endl;
    for(const auto& cf: cfs)
    {
        //std::cout << cf.ls[0] << " " << cf.ls[1] << std::endl;
    }
}
