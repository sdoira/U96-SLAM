
#ifndef RTABMAP_FLANN_RESULTSET_H
#define RTABMAP_FLANN_RESULTSET_H

#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

namespace flann
{

/* This record represents a branch point when finding neighbors in
    the tree.  It contains a record of the minimum distance to the query
    point, as well as the node at which the search resumes.
 */

template <typename T, typename DistanceType>
struct BranchStruct
{
    T node;           /* Tree node at which search resumes */
    DistanceType mindist;     /* Minimum distance to query for all nodes below. */

    BranchStruct() {
	}
    BranchStruct(const T& aNode, DistanceType dist) : node(aNode), mindist(dist) {
	}

    bool operator<(const BranchStruct<T, DistanceType>& rhs) const
    {
        return mindist<rhs.mindist;
    }
};


template <typename DistanceType>
struct DistanceIndex
{
    DistanceIndex(DistanceType dist, size_t index) :
        dist_(dist), index_(index)
    {
    }
    bool operator<(const DistanceIndex& dist_index) const
    {
        return (dist_ < dist_index.dist_) || ((dist_ == dist_index.dist_) && index_ < dist_index.index_);
    }
    DistanceType dist_;
    size_t index_;
};


template <typename DistanceType>
class ResultSet
{
public:
    virtual ~ResultSet() {}

    virtual bool full() const = 0;

    virtual void addPoint(DistanceType dist, size_t index) = 0;

    virtual DistanceType worstDist() const = 0;

};

/**
 * KNNSimpleResultSet does not ensure that the element it holds are unique.
 * Is used in those cases where the nearest neighbour algorithm used does not
 * attempt to insert the same element multiple times.
 */
template <typename DistanceType>
class KNNSimpleResultSet : public ResultSet<DistanceType>
{
public:
	typedef DistanceIndex<DistanceType> DistIndex;

	KNNSimpleResultSet(size_t capacity_) :
        capacity_(capacity_)
    {
		// reserving capacity to prevent memory re-allocations
		dist_index_.resize(capacity_, DistIndex(std::numeric_limits<DistanceType>::max(),-1));
    	clear();
    }

    ~KNNSimpleResultSet()
    {
    }

    /**
     * Clears the result set
     */
    void clear()
    {
        worst_distance_ = std::numeric_limits<DistanceType>::max();
        dist_index_[capacity_-1].dist_ = worst_distance_;
        count_ = 0;
    }

    /**
     *
     * @return Number of elements in the result set
     */
    size_t size() const
    {
        return count_;
    }

    /**
     * Radius search result set always reports full
     * @return
     */
    bool full() const
    {
        return count_==capacity_;
    }

    /**
     * Add a point to result set
     * @param dist distance to point
     * @param index index of point
     */
    void addPoint(DistanceType dist, size_t index)
    {
    	if (dist>=worst_distance_) return;

        if (count_ < capacity_) ++count_;
        size_t i;
        for (i=count_-1; i>0; --i) {
#ifdef FLANN_FIRST_MATCH
            if ( (dist_index_[i-1].dist_>dist) || ((dist==dist_index_[i-1].dist_)&&(dist_index_[i-1].index_>index)) )
#else
            if (dist_index_[i-1].dist_>dist)
#endif
            {
            	dist_index_[i] = dist_index_[i-1];
            }
            else break;
        }
        dist_index_[i].dist_ = dist;
        dist_index_[i].index_ = index;
        worst_distance_ = dist_index_[capacity_-1].dist_;
    }

    /**
     * Copy indices and distances to output buffers
     * @param indices
     * @param dists
     * @param num_elements Number of elements to copy
     * @param sorted Indicates if results should be sorted
     */
    void copy(size_t* indices, DistanceType* dists, size_t num_elements, bool sorted = true)
    {
    	size_t n = std::min(count_, num_elements);
    	for (size_t i=0; i<n; ++i) {
    		*indices++ = dist_index_[i].index_;
    		*dists++ = dist_index_[i].dist_;
    	}
    }

    DistanceType worstDist() const
    {
    	return worst_distance_;
    }

private:
    size_t capacity_;
    size_t count_;
    DistanceType worst_distance_;
    std::vector<DistIndex> dist_index_;
};

}

#endif //FLANN_RESULTSET_H

