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

        std::vector<std::vector<link>> interlink_chunks;
    };
public:    
    subgraph(unsigned chunk_size, distance_between_elems distance): chunk_size(chunk_size), distance(distance), elem_num(0)
    {
    }
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
        elem_num += chunk_ptr->size;
        compute_chunk_interlinks(chunk_ptr);
        compute_chunk_inside_links(chunk_ptr);

        chunks.push_back(chunk_ptr);
        

        update_links_in_model();
    }
    
    void update_links_in_model()
    {
        model.clear();
        auto inserter = std::back_insert_iterator<decltype(model)>(model);
        for(const auto & chunk_ptr: chunks)
        {
            for(const auto & inter_links: chunk_ptr->interlink_chunks)
            {
                std::copy(std::begin(inter_links), std::end(inter_links), inserter);
            }
            std::copy(std::begin(chunk_ptr->links), std::end(chunk_ptr->links), inserter);
        }
        std::sort(std::begin(model), std::end(model), [](const auto & l, const auto & r){return l.lenght < r.lenght; });
        filter_by_kruskal(model);
    }
    class color_table
    {
    public:
        color_table(unsigned start, const unsigned num): table(num,0), index_start(start)
        {
            for(unsigned i=0; i < table.size(); i++)
            {
                table[i] = i;
            }
        }
        void connect(unsigned a, unsigned b)
        {
            if(a == b) return;
            if(a > b) connect(b, a);
            if(table[a-index_start] == a)
            {
                table[b-index_start] = a;
            }
            else
            {
                table[b-index_start] = find_parent(a);
            }
        }
        unsigned find_parent(unsigned i)
        {
            if(i == table[i-index_start])
            {
                return i;
            }
            else
            {
                table[i-index_start] = find_parent(table[i-index_start]);
                return table[i-index_start];
            }
        }

        bool are_connected(unsigned a, unsigned b)
        {
            return find_parent(a) == find_parent(b);
        }
    private:
        const unsigned index_start;
        std::vector<unsigned> table;
    };

    void filter_by_kruskal(std::vector<link> & links)
    {
        auto insert_iter = std::begin(links);
        auto iter = std::begin(links);
        
        color_table ct(chunks[0]->index, elem_num);
        unsigned last_inserted_i = 0;

        unsigned n=0;
        while(n<(elem_num-1))
        {
            if(! ct.are_connected(iter->a, iter->b))
            {
                *insert_iter = *iter;
                insert_iter++;
                ct.connect(iter->a, iter->b);
                n++;
            }
            iter++;

        }
        model.erase(std::begin(model) + elem_num-1, std::end(model));
    }

    void compute_chunk_interlinks(std::shared_ptr<chunk> chunk_ptr)
    {
        for(const auto & other_chunk_ptr: chunks)
        {
            compute_links_between(chunk_ptr, other_chunk_ptr);
        }
    }
    void compute_links_between(std::shared_ptr<chunk> new_chunk_ptr, std::shared_ptr<chunk> other_chunk_ptr)
    {
        new_chunk_ptr->interlink_chunks.push_back({});

        const auto & l_elements = new_chunk_ptr->elements;

        std::vector<double> l_distances(l_elements.size(),std::numeric_limits<double>::max());
        std::vector<bool> l_out_of_mst(l_elements.size(),true);
        std::vector<unsigned> l_ids(l_elements.size(),0);

        const auto & r_elements = other_chunk_ptr->elements;

        std::vector<double> r_distances(r_elements.size(),std::numeric_limits<double>::max());
        std::vector<bool> r_out_of_mst(r_elements.size(),true);
        std::vector<unsigned> r_ids(r_elements.size(),0);

        l_out_of_mst[0] = false;
        bool last_elem_from_left = true;
        unsigned last_inserted_to_mst_i = 0;
        for(unsigned i=0; i<(l_elements.size() + r_elements.size() -1); i++)
        {
            if(last_elem_from_left)
            {
                for(unsigned y=0; y<r_elements.size(); y++)
                {
                    auto d = distance(l_elements[last_inserted_to_mst_i], r_elements[y]);
                    if(r_out_of_mst[y] && d < r_distances[y])
                    {
                        r_distances[y] = d;
                        r_ids[y] = last_inserted_to_mst_i;
                    }
                }
            }
            else
            {
                for(unsigned y=0; y<l_elements.size(); y++)
                {
                    auto d = distance(r_elements[last_inserted_to_mst_i], l_elements[y]);
                    if(l_out_of_mst[y] && d < l_distances[y])
                    {
                        l_distances[y] = d;
                        l_ids[y] = last_inserted_to_mst_i;
                    }
                }
            }
            auto l_best_i = find_min_distance_index(l_distances, l_out_of_mst);
            auto r_best_i = find_min_distance_index(r_distances, r_out_of_mst);

            if(l_distances[l_best_i] < r_distances[r_best_i])
            {
                new_chunk_ptr->interlink_chunks.back().push_back( link(l_best_i+new_chunk_ptr->index, l_ids[l_best_i]+other_chunk_ptr->index, l_distances[l_best_i]) );
                l_out_of_mst[l_best_i] = false;
                last_inserted_to_mst_i = l_best_i;
                last_elem_from_left = true;
            }
            else
            {
                new_chunk_ptr->interlink_chunks.back().push_back( link(r_best_i+other_chunk_ptr->index, r_ids[r_best_i]+new_chunk_ptr->index, r_distances[r_best_i]) );
                r_out_of_mst[r_best_i] = false;
                last_inserted_to_mst_i = r_best_i;
                last_elem_from_left = false;
            }
        }
    }
    void compute_chunk_inside_links(std::shared_ptr<chunk> chunk_ptr)
    {
        const auto & elements = chunk_ptr->elements;
        auto & links = chunk_ptr->links;

        std::vector<double> distances(elements.size(),std::numeric_limits<double>::max());
        std::vector<bool> out_of_mst(elements.size(),true);
        std::vector<unsigned> ids(elements.size(),0);

        out_of_mst[0] = false;

        unsigned last_inserted_to_mst_i = 0;
        
        for(unsigned i=1; i<elements.size(); i++)
        {
            for(unsigned y=0; y<elements.size(); y++)
            {
                auto d = distance(elements[last_inserted_to_mst_i], elements[y]);
                if(out_of_mst[y] && d < distances[y])
                {
                    distances[y] = d;
                    ids[y] = last_inserted_to_mst_i;
                }
            }

            unsigned best = find_min_distance_index(distances, out_of_mst);

            links.push_back(link(ids[best] + chunk_ptr->index, best + chunk_ptr->index, distances[best]));

            out_of_mst[best] = false;
            last_inserted_to_mst_i = best;
        }
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
        global_index += chunk_ptr->size;
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
        return model;
    }
private:
    std::vector<E> input_buffer;
    std::vector<std::shared_ptr<chunk>> chunks;

    int chunk_size;
    unsigned global_index = 0;
    
    distance_between_elems distance;

    std::vector<link> model;
    bool model_is_actual = false;

    unsigned elem_num;
};
}