#include <memory>
#include <functional>
#include <algorithm>
#include <tuple>
#include <cmath>
#include <vector>
#include <iterator>

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
        node_num = 0;
    }

    void add_element(const CF & cf_created_from_element)
    {
        node_num++;
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

    int get_node_num() const
    {
        return node_num;
    }

    std::vector<CF> get_cfs()
    {
        std::vector<CF> cfs;
        std::vector<node_ptr> stack;
        stack.reserve(node_num);

        stack.push_back(root);

        while(! stack.empty())
        {
            auto last = stack.back();

            for(const auto& cf: last->cluster_features)
            {
                cfs.push_back(cf);
            }

            stack.pop_back();

            auto riter = std::rbegin(last->childs);
            
            while(riter != std::rend(last->childs))
            {
                if(*riter != nullptr)
                {
                    stack.push_back(*riter);
                }
                riter++;
            }
        }
        return cfs;
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
            split(n_ptr, cf_created_from_element);
        }
    }

    void split(node_ptr & n_ptr, const CF & cf_created_from_element)
    {
        auto & cfs = n_ptr->cluster_features;
        cfs.push_back(cf_created_from_element);

        unsigned int cf_i=0, cf_y=0;

        std::tie(cf_i, cf_y) = get_nearest_cfs_in(cfs);

        node_ptr new_n_ptr = std::make_shared<node>();
        new_n_ptr->cluster_features.push_back(cfs[cf_i]);
        new_n_ptr->cluster_features.push_back(cfs[cf_y]);
        new_n_ptr->childs.push_back(nullptr);
        new_n_ptr->childs.push_back(nullptr);

        n_ptr->childs[cf_i] = new_n_ptr;
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
                if(d >= nd)
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
    unsigned int node_num;
};

struct StandardCF
{
    double ls[2];
    double ss[2];
    unsigned n;
};

void update_cf(StandardCF & cluster_feature, const StandardCF& new_element)
{
    cluster_feature.n +=  new_element.n;
    cluster_feature.ss[0] += new_element.ss[0];
    cluster_feature.ss[1] += new_element.ss[1];

    cluster_feature.ls[0] += new_element.ls[0];
    cluster_feature.ls[1] += new_element.ls[1];
}

double distance_between_cf(const StandardCF & cf_a, const StandardCF & cf_b)
{
    double a[2] = {0,0};
    a[0] = cf_a.ls[0] / cf_a.n;
    a[1] = cf_a.ls[1] / cf_a.n;
    double b[2] = {0,0};
    b[0] = cf_b.ls[0] / cf_b.n;
    b[1] = cf_b.ls[1] / cf_b.n;
    double d = (a[0] - b[0])*(a[0] - b[0]) + (a[1] - b[1])*(a[1] - b[1]);
    return std::sqrt(d);
}
}