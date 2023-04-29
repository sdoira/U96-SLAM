
#ifndef RTABMAP_FLANN_PARAMS_H_
#define RTABMAP_FLANN_PARAMS_H_

#include "flann/any.h"
#include "flann/general.h"
#include <iostream>
#include <map>

namespace flann
{

namespace anyimpl
{
SMALL_POLICY(flann_algorithm_t);
SMALL_POLICY(flann_centers_init_t);
SMALL_POLICY(flann_log_level_t);
SMALL_POLICY(flann_datatype_t);
}


typedef std::map<std::string, any> IndexParams;


typedef enum {
	FLANN_False = 0,
	FLANN_True = 1,
	FLANN_Undefined
} tri_type;


struct SearchParams
{
    SearchParams(int checks_ = 32, float eps_ = 0.0, bool sorted_ = true ) :
    	checks(checks_), eps(eps_), sorted(sorted_)
    {
    	max_neighbors = -1;
    	use_heap = FLANN_Undefined;
    	cores = 1;
    	matrices_in_gpu_ram = false;
    }

    // how many leafs to visit when searching for neighbours (-1 for unlimited)
    int checks;
    // search for eps-approximate neighbours (default: 0)
    float eps;
    // only for radius search, require neighbours sorted by distance (default: true)
    bool sorted;
    // maximum number of neighbors radius search should return (-1 for unlimited)
    int max_neighbors;
    // use a heap to manage the result set (default: FLANN_Undefined)
    tri_type use_heap;
    // how many cores to assign to the search (used only if compiled with OpenMP capable compiler) (0 for auto)
    int cores;
    // for GPU search indicates if matrices are already in GPU ram
    bool matrices_in_gpu_ram;
};

/*
inline bool has_param(const IndexParams& params, std::string name)
{
	return params.find(name)!=params.end();
}
*/

template<typename T>
T get_param(const IndexParams& params, std::string name, const T& default_value)
{
    IndexParams::const_iterator it = params.find(name);
    if (it != params.end()) {
        return it->second.cast<T>();
    }
    else {
        return default_value;
    }
}

template<typename T>
T get_param(const IndexParams& params, std::string name)
{
    IndexParams::const_iterator it = params.find(name);
    if (it != params.end()) {
        return it->second.cast<T>();
    }
    else {
        throw FLANNException(std::string("Missing parameter '")+name+std::string("' in the parameters given"));
    }
}
/*
inline void print_params(const IndexParams& params)
{
    IndexParams::const_iterator it;

    for(it=params.begin(); it!=params.end(); ++it) {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}

inline void print_params(const SearchParams& params)
{
	std::cout << "checks : " << params.checks << std::endl;
	std::cout << "eps : " << params.eps << std::endl;
	std::cout << "sorted : " << params.sorted << std::endl;
	std::cout << "max_neighbors : " << params.max_neighbors << std::endl;
}
*/

}


#endif /* FLANN_PARAMS_H_ */
