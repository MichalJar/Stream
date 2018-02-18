#include <iostream>
#include <map>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

struct element2d
{
    double val[2];
};

std::vector<element2d> load_elems_from(const std::string & file_name)
{
    
}

class InvalidParamsException: std::exception
{
public:
    InvalidParamsException(): message("improper params"){}
    InvalidParamsException(const std::string & message): message(message){}
    const char * what() const noexcept override
    {
        return message.c_str();
    }
private:
    const std::string message;
};
void add_param_to_map(const std::string & param, std::map<std::string, std::string> & params)
{
    auto i = std::find(std::begin(param), std::end(param), '=');
    if(i == std::end(param) or i == (std::end(param)-1) or i == std::begin(param))
    {
        throw InvalidParamsException();
    }
    else
    {
        std::string param_key, param_val;
        std::copy(std::begin(param), i, std::back_inserter(param_key));
        std::copy(i+1, std::end(param), std::back_inserter(param_val));
        if(params.count(param_key)) 
        {
            throw InvalidParamsException();
        }
        params[param_key] = param_val;
    }
}
std::map<std::string, std::string> create_map_from(int argc, char * argv[])
{
    std::map<std::string, std::string> params;
    for(int i=1; i < argc; i++)
    {
        std::string param = argv[i];
        add_param_to_map(param, params);
    }
    return params;
}

int run_subgraph(const unsigned chunk_size, 
const unsigned max_chunk_num, 
const unsigned column_num, 
const std::string & result_file, 
const std::string & times_file,
const std::string & input_file )
{
    return 0;
}
int main(int argc, char * argv[])
{
    auto params = create_map_from(argc, argv);
    auto type = params.at("type");
    if(type == "subgraph")
    {
        unsigned chunk_size    = std::stoul(params.at("chunksize"));
        unsigned max_chunk_num = std::stoul(params.at("maxchunknum"));
        unsigned column_num    = std::stoul(params.at("columnnum"));
        std::string result_file   = params.at("resultfile");
        std::string times_file    = params.at("timesfile");
        std::string times_file    = params.at("inputfile");
    }
    else if(type == "clustree")
    {

    }
    else
    {
        throw InvalidParamsException();
    }
}

