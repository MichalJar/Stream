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
        uint32_t a;
        uint32_t b;
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
    subgraph(distance_between_elems distance): distance(distance), elem_num(0)
    {
    }

    void add_elements(std::vector<E> elements)
    {
        update_model(std::move(elements));
    }

    const std::vector<link> & get_model()
    {
        return model;
    }
    void remove_oldest_chunk()
    {
        elem_num -= (*chunks.begin())->size;
        chunks.erase(std::begin(chunks));
        for(auto chunk_ptr: chunks)
        {
            chunk_ptr->interlink_chunks.pop_back();
        }
        update_links_in_model();
    }
    int input_buffer_size()
    {
        return input_buffer.size();
    }
    int chunks_num()
    {
        return chunks.size();
    }
private:
    void update_model(std::vector<E>&& elements)
    {
        auto chunk_ptr = create_new_chunk(std::move(elements));
        elem_num += chunk_ptr->size;
        compute_chunk_interlinks(chunk_ptr);
        compute_chunk_inside_links(chunk_ptr);
        chunks.push_back(chunk_ptr);
    }
public:
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

    class disjoint_sets
    {
    public:
        disjoint_sets(unsigned start, const unsigned num): parentOf(num,0), sizeOfSet(num,1), start(start)
        {
            for(unsigned i=0; i<parentOf.size(); i++)
            {
                parentOf[i] = start+i;
            }
        }

        bool areConnected(unsigned elemIndexA, unsigned elemIndexB)
        {
            return getParentOf(elemIndexA) == getParentOf(elemIndexB);
        }

        void connect(unsigned elemIndexA, unsigned elemIndexB)
        {
            auto parentOfA = getParentOf(elemIndexA);
            auto parentOfB = getParentOf(elemIndexB);

            if(sizeOfSet[parentOfA - start] >= sizeOfSet[parentOfB - start])
            {
                setParentOf(parentOfA, parentOfB);
                sizeOfSet[parentOfA - start] += sizeOfSet[parentOfB - start];
            }
            else
            {
                sizeOfSet[parentOfB - start] += sizeOfSet[parentOfA - start];
                setParentOf(parentOfB, parentOfA);
            }
        }
    private:
        unsigned getParentOf(unsigned elemIndex)
        {
            if(parentOf[elemIndex - start] == elemIndex)
            {
                return parentOf[elemIndex - start];
            }
            
            parentOf[elemIndex - start] = getParentOf(parentOf[elemIndex-start]);

            return parentOf[elemIndex - start];
        }

        unsigned setParentOf(unsigned elemIndex, unsigned newParent)
        {
            parentOf[elemIndex - start] = newParent;
        }

        std::vector<unsigned> parentOf;
        std::vector<unsigned> sizeOfSet;
        unsigned start;
    };
    void filter_by_kruskal(std::vector<link> & links)
    {
        auto insert_iter = std::begin(links);
        auto iter = std::begin(links);
        disjoint_sets ct(chunks[0]->index, elem_num);
        unsigned last_inserted_i = 0;
        unsigned n=0;
        while(n<(elem_num-1))
        {
            if(! ct.areConnected(iter->a, iter->b))
            {
                *insert_iter = *iter;
                insert_iter++;
                ct.connect(iter->a, iter->b);
                n++;
            }
            iter++;

        }
        links.erase(std::begin(model) + elem_num-1, std::end(model));
    }

    void compute_chunk_interlinks(std::shared_ptr<chunk> chunk_ptr)
    {
        for(auto other_chunk_ptr_i = std::rbegin(chunks); other_chunk_ptr_i != std::rend(chunks); other_chunk_ptr_i++)
        {
            compute_links_between(chunk_ptr, *other_chunk_ptr_i);
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
                update_distances_by(l_elements[last_inserted_to_mst_i], last_inserted_to_mst_i, r_elements, r_out_of_mst, r_ids, r_distances);
            }
            else
            {
                update_distances_by(r_elements[last_inserted_to_mst_i], last_inserted_to_mst_i, l_elements, l_out_of_mst, l_ids, l_distances);
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
    void update_distances_by(const E & last_inserted_element, 
    const unsigned last_inserted_element_index,
    const std::vector<E> & elements, 
    const std::vector<bool> & out_of_mst,
    std::vector<unsigned> & ids,
    std::vector<double> & distances)
    {
        for(unsigned y=0; y<elements.size(); y++)
        {
            auto d = distance(last_inserted_element, elements[y]);
            if(out_of_mst[y] && d < distances[y])
            {
                ids[y] = last_inserted_element_index;
                distances[y] = d;
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
    std::shared_ptr<chunk> create_new_chunk(std::vector<E> && elements)
    {
        auto chunk_ptr = std::make_shared<chunk>(global_index, elements.size());
        chunk_ptr->elements = std::move(elements);
        global_index += chunk_ptr->size;
        return chunk_ptr;
    }
    std::vector<E> input_buffer;
    std::vector<std::shared_ptr<chunk>> chunks;
    unsigned global_index = 0;
    distance_between_elems distance;
    std::vector<link> model;
    bool model_is_actual = false;
    unsigned elem_num;
};
}
