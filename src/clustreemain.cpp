#include <iostream>
#include <vector>
#include <chrono>
#include "loader.hpp"
#include "clustree.hpp"

std::chrono::milliseconds get_now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

struct CF
{

};

int main(int argc, char * argv[])
{
    const std::string input_filename = argv[1];
    stream::clustree<stream::StandardCF> cl(3, 0.5, stream::update_cf, stream::distance_between_cf);    
}