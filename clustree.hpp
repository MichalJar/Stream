#include <memory>
#include <functional>
#include <algorithm>

namespace stream{

template<typename CF>
class clustree
{
    using update_cf = std::function<void (CF & cluster_feature, const CF & new_element)>;
    using distance_between_cf  = std::function<void (const CF & cf_a, const CF & cf_b)>;

    class node
    {
    public:
        std::vector<cf> cluster_features;
        std::vector<std::shared_ptr<node>> childs;
    }

    using node_ptr = std::shared_ptr<node>;

public:
    clustree(int node_max_width, int cf_max_diameter, update_cf update, distance_between_cf distance):
        node_max_width(node_max_width), 
        cf_max_diameter(cf_max_diameter),
        update(update),
        distance(distance)
    {
        root = std::make_shared<Node>();
    }

    void add_element(const CF & cf_created_from_element)
    {
        if(root->cluster_features.size() == 0)
        {
            root->cluster_features.push_back( cf_created_from_element );
            root->childs.push_back(nullptr)
        }
        else
        {
            add_element_to_no_empty_tree(cf_created_from_element);
        }
    }
private:
    void add_element_to_no_empty_tree(const CF & cf_created_from_element)
    {
        auto n_ptr = traverse_and_update(element);

        const int i = get_nearest_cf_index_from(cf_created_from_element, n_ptr);
        if(distance(cf_created_from_element, n_ptr->cluster_features[i]) <= cf_max_diameter)
        {
            update_cf(n_ptr->cluster_features[i], cf_created_from_element);
        }
        else if( n_ptr->cluster_features.size() >= node_max_width)
        {
            update_cf(n_ptr->cluster_features[i], cf_created_from_element);
            n_ptr->childs[i] = create_new_leaf(cf_created_from_element);
        }
        else 
        {
            n_ptr->cluster_features.push_back(cf_created_from_element);
            n_ptr->childs.push_back(nullptr);
        }
    }

    node_ptr create_new_leaf(const & cf_created_from_element)
    {
        node_ptr new_n_ptr = std::make_shared<Node>();
        new_n_ptr->cluster_features.push_back(cf_created_from_element);
        new_n_ptr->childs.push_back(nullptr);
        return new_n_ptr;
    }

    node_ptr traverse_and_update(const CF & cf_created_from_element)
    {
        node_ptr n_ptr = root;
        while(true)
        {
            const int i = get_nearest_cf_index_from(cf_created_from_element, n_ptr);
            if(n_ptr->childs[i] == nullptr)
            {
                break;
            }
            else
            {
                update_cf(n_ptr->cluster_features[i], cf_created_from_element);
                n_ptr = n_ptr->childs[i];
            }
        }
        return n_ptr;
    }

    const int get_nearest_cf_index_from(const cf<e> & new_cf, const node_ptr n_ptr)
    {
        auto const & cfs = n_ptr->cluster_features;
        int best_index = 0;
        int d = dist(new_cf, cfs[0]);

        for(auto i=1; i< cfs.size(); i++)
        {
            if(d > dist(new_cf, cfs[i]))
            {
                d = dist(new_cf, cfs[i]);
                best_index = i;
            }
        }
        return best_index;
    }

private:
    const int node_max_width;
    const double cf_max_diameter;

    node_ptr root;

    update_cf update;
    distance_between_cf distance;
};
}
