#include <memory>
#include <functional>
#include <algorithm>
#include <tuple>

namespace stream{

template<typename CF>
class clustree
{
    using update_cf = std::function<void (CF & cluster_feature, const CF & new_element)>;
    using distance_between_cf  = std::function<double (const CF & cf_a, const CF & cf_b)>;

    class node
    {
    public:
        std::vector<CF> cluster_features;
        std::vector<std::shared_ptr<node>> childs;
    };

    using node_ptr = std::shared_ptr<node>;

public:
    clustree(unsigned int node_max_width, double cf_max_diameter, update_cf update, distance_between_cf distance):
        node_max_width(node_max_width), 
        cf_max_diameter(cf_max_diameter),
        update(update),
        distance(distance)
    {
        root = std::make_shared<node>();
    }

    void add_element(const CF & cf_created_from_element)
    {
        if(root->cluster_features.size() == 0)
        {
            root->cluster_features.push_back( cf_created_from_element );
            root->childs.push_back(nullptr);
        }
        else
        {
            add_element_to_no_empty_tree(cf_created_from_element);
        }
    }

    node_ptr root;
private:
    void add_element_to_no_empty_tree(const CF & cf_created_from_element)
    {
        auto n_ptr = traverse_and_update(cf_created_from_element);

        const int i = get_nearest_cf_index_from(cf_created_from_element, n_ptr);
        if(distance(cf_created_from_element, n_ptr->cluster_features[i]) <= cf_max_diameter)
        {
            update(n_ptr->cluster_features[i], cf_created_from_element);
        }
        else if( n_ptr->cluster_features.size() < node_max_width)
        {
            n_ptr->cluster_features.push_back(cf_created_from_element);
            n_ptr->childs.push_back(nullptr);
        }
        else 
        {
            n_ptr->childs[i] = create_new_leaf(cf_created_from_element, n_ptr->cluster_features[i]);
            update(n_ptr->cluster_features[i], cf_created_from_element);
        }
    }

    void split(node_ptr & n_ptr, const CF & cf_created_from_element)
    {
        auto & cfs = n_ptr->cluster_features;
        cfs.push_back(cf_created_from_element);

        unsigned int cf_i=0, cf_y=0;

        std::tie(cf_i, cf_y) = get_nearest_cfs_in(cfs);

        node_ptr new_n_ptr = std::make_shared<node>();
        new_n_ptr->cluster_features.push_back(cfs[cf_i], cfs[cf_y]);
        new_n_ptr->childs.push_back(nullptr);
        new_n_ptr->childs.push_back(nullptr);

        n_ptr->childs = new_n_ptr;
        update(cfs[cf_i],cfs[cf_y]);
        cfs.erase(std::begin(cfs) + cf_y);
    }

    auto get_nearest_cfs_in(const std::vector<CF> & cfs)
    {
        unsigned int cf_i = 0;
        unsigned int cf_y = 1;

        double d = distance(cfs[0],cfs[1]);

        for(int i=0;i<cfs.size()-1;i++)
        {
            for(int y=i+1;y<cfs.size();y++)
            {
                auto nd = distance(cfs[i],cfs[y]);
                if(d > nd)
                {
                    d = nd;
                    cf_i = i;
                    cf_y = y;
                }
            }
        }
        return std::make_tuple(cf_i, cf_y);
    }

    node_ptr create_new_leaf(const CF & cf_created_from_element, const CF & parent_cf)
    {
        node_ptr new_n_ptr = std::make_shared<node>();

        new_n_ptr->cluster_features.push_back(parent_cf);
        new_n_ptr->cluster_features.push_back(cf_created_from_element);
        
        new_n_ptr->childs.push_back(nullptr);
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
                update(n_ptr->cluster_features[i], cf_created_from_element);
                n_ptr = n_ptr->childs[i];
            }
        }
        return n_ptr;
    }

    const int get_nearest_cf_index_from(const CF & new_cf, const node_ptr n_ptr)
    {
        auto const & cfs = n_ptr->cluster_features;
        int best_index = 0;
        int d = distance(new_cf, cfs[0]);

        for(auto i=1; i< cfs.size(); i++)
        {
            if(d > distance(new_cf, cfs[i]))
            {
                d = distance(new_cf, cfs[i]);
                best_index = i;
            }
        }
        return best_index;
    }
    const unsigned int node_max_width;
    const double cf_max_diameter;
    update_cf update;
    distance_between_cf distance;
};
}
