#pragma once
#include <iostream>
#include <vector>
#include <csv.h>

struct Element
{
    double val[2];
};

std::vector<Element> load_elements_from(const std::string & filename)
{
    std::vector<Element> elements;
    Element e;
    io::CSVReader<2> reader(filename);
    double a; double b;
    while(reader.read_row(a,b))
    {
        e.val[0] = a;
        e.val[1] = b;
        elements.push_back(e);
    }
    return elements;
}
