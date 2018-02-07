#include <vector>
#include <functional>
#include <algorithm>

namespace stream
{
template<typename E>
class subgraph
{
public:
    struct link
    {
        link(unsigned a, unsigned b, double lenght): a(a), b(b), lenght(lenght) {}
        unsigned a;
        unsigned b;
        double lenght;
        bool operator==(const link & l) const {return a == l.a && b == l.b && lenght == l.lenght;}
    };

    using distance_between_elems = std::function<double (const E & e1, const E & e2)>;
private:
    struct chunk
    {
        chunk(unsigned index, unsigned size): index(index), size(size) {}
        
        unsigned index;
        unsigned size;

        std::vector<E> elements;
        std::vector<link> links;
    };
public:    
    subgraph(unsigned chunk_size, distance_between_elems distance): chunk_size(chunk_size), distance(distance)
    {}
    void add_element(const E & element)
    {
        input_buffer.push_back(element);
        if(input_buffer.size() >= chunk_size)
        {
            update_model();   
        }
    }

    void update_model()
    {
        auto chunk_ptr = create_new_chunk();
        // important assumption - new chunk isn't in chunks collection yet
        compute_chunk_links(chunk_ptr);
        chunks.push_back(chunk_ptr);
    }

    void compute_chunk_links(std::shared_ptr<chunk> chunk_ptr)
    {
        const auto & elements = chunk_ptr->elements;
        auto & links = chunk_ptr->links;

        std::vector<double> distances(elements.size(),std::numeric_limits<double>::max());
        std::vector<bool> out_of_mst(elements.size(),true);
        std::vector<unsigned> ids(elements.size(),0);

        out_of_mst[0] = false;

        unsigned last_inserted_to_mst_index = 0;
        
        for(unsigned i=1; i<elements.size(); i++)
        {
            for(unsigned y=0; y<elements.size(); y++)
            {
                auto d = distance(elements[last_inserted_to_mst_index], elements[y]);
                if(out_of_mst[y] && d < distances[y])
                {
                    distances[y] = d;
                    ids[y] = last_inserted_to_mst_index;
                }
            }

            unsigned best = find_min_distance_index(distances, out_of_mst);

            links.push_back(link(ids[best] + chunk_ptr->index, best + chunk_ptr->index, distances[best]));

            out_of_mst[best] = false;
            last_inserted_to_mst_index = best;
        }

        std::sort(std::begin(links), std::end(links), [](const auto & l, const auto & r){return l.lenght < r.lenght; });
    }

    unsigned find_min_distance_index(const std::vector<double> & distances, const std::vector<bool> & out_of_mst) const
    {
        unsigned best = 1;
        double best_distance = std::numeric_limits<double>::max();
        for(int i=1; i<distances.size(); i++)
        {
            if(out_of_mst[i] && distances[i] < best_distance)
            {
                best = i;
                best_distance = distances[i];
            }
        }
        return best;
    }

    std::shared_ptr<chunk> create_new_chunk()
    {
        auto chunk_ptr = std::make_shared<chunk>(global_index, chunk_size);
        chunk_ptr->elements = std::move(input_buffer);
        global_index += chunk_size;
        return chunk_ptr;
    }

    int input_buffer_size()
    {
        return input_buffer.size();
    }
    int chunks_num()
    {
        return chunks.size();
    }
    const std::vector<link> & get_model()
    {
        return chunks.back()->links;
    }
private:
    std::vector<E> input_buffer;
    std::vector<std::shared_ptr<chunk>> chunks;

    int chunk_size;
    unsigned global_index = 0;
    
    distance_between_elems distance;
};
}