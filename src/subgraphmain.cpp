#include <iostream>
#include <vector>
#include <csv.h>
#include "subgraph.hpp"
#include <chrono>
#include "loader.hpp"

struct Times
{
    std::chrono::milliseconds all;
    std::chrono::milliseconds computing_new_chunk;
    std::chrono::milliseconds updating_final_model;
};

std::chrono::milliseconds get_now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

//using to_microseconds = std::chrono::duration_cast<std::chrono::microseconds>;
int main(int argc, char * argv[])
{
    const std::string input_filename = argv[1];
    const unsigned chunk_size        = std::stoul(argv[2]);
    const unsigned chunks_num        = std::stoul(argv[3]);

    auto distance = [](const Element &e1, const Element &e2){
        return e1.val[0] * e2.val[0] + e1.val[1] * e2.val[1];
    };

    stream::subgraph<Element> sgraph(chunk_size, distance);

    auto elements = load_elements_from(input_filename);
    unsigned chunk_i = 0;
    unsigned chunks_num_i = 0;

    std::vector<Times> tims;

    unsigned i = 0;
    for(auto e: elements)
    {
        i++;
        chunk_i++;
        if(chunk_i >= chunk_size)
        {
            //std::cout << "--- " << i << " ---"<<std::endl;
            chunk_i = 0;
            chunks_num_i++;
            auto start = get_now();
            sgraph.add_element(e);
            auto after_computing_chunk = get_now();
            if(chunks_num_i >= chunks_num)
            {
                sgraph.remove_oldest_chunk();
            }
            else sgraph.update_links_in_model();
            auto after_updating_final_model = get_now();

            Times times = {
                after_updating_final_model - start,
                after_computing_chunk - start,
                after_updating_final_model - after_computing_chunk
            };

            tims.push_back(times);
        }
        else sgraph.add_element(e);
    }

    for(const auto & t: tims)
    {
        std::cout 
//        << "all: " 
        << t.all.count() 
//        << " new chunk computing: " 
//        << t.computing_new_chunk.count() 
//        << " updating final model: " 
//        << t.updating_final_model.count() 
        << std::endl;
    }
}
