
#ifndef RTABMAP_FLANN_NNINDEX_H
#define RTABMAP_FLANN_NNINDEX_H

#include <vector>

#include "flann/general.h"
#include "flann/matrix.h"
#include "flann/params.h"
#include "flann/result_set.h"
#include "flann/dynamic_bitset.h"

namespace flann
{

#define KNN_HEAP_THRESHOLD 250


class IndexBase
{
public:
    virtual ~IndexBase() {};

    virtual size_t veclen() const = 0;

    virtual size_t size() const = 0;

    virtual flann_algorithm_t getType() const = 0;

    virtual int usedMemory() const = 0;

    virtual IndexParams getParameters() const = 0;

    virtual void loadIndex(FILE* stream) = 0;

    virtual void saveIndex(FILE* stream) = 0;
};

/**
 * Nearest-neighbour index base class
 */
template <typename Distance>
class NNIndex : public IndexBase
{
public:
    typedef typename Distance::ElementType ElementType;
    typedef typename Distance::ResultType DistanceType;

	NNIndex(const IndexParams& params, Distance d) : distance_(d), last_id_(0), size_(0), size_at_build_(0), veclen_(0),
			index_params_(params), removed_(false), removed_count_(0), data_ptr_(NULL)
	{
	}

	virtual ~NNIndex()
	{
		if (data_ptr_) {
			delete[] data_ptr_;
		}
	}


	virtual NNIndex* clone() const = 0;

	virtual void buildIndex()
	{
    	freeIndex();
    	cleanRemovedPoints();

    	// building index
		buildIndexImpl();

        size_at_build_ = size_;

	}

    virtual void addPoints(const Matrix<ElementType>& points, float rebuild_threshold = 2)
    {
    }

    virtual ElementType* getPoint(size_t id)
    {
		return NULL;
    }

    /**
     * @return number of features in this index.
     */
    inline size_t size() const
    {
    	return size_ - removed_count_;
    }

    inline size_t removedCount() const
	{
		return removed_count_;
	}

    inline size_t sizeAtBuild() const
	{
		return size_at_build_;
	}

    inline size_t veclen() const
    {
        return veclen_;
    }

    IndexParams getParameters() const
    {
        return index_params_;
    }

    template<typename Archive>
    void serialize(Archive& ar)
    {
    }

    virtual int knnSearch(const Matrix<ElementType>& queries,
    		Matrix<size_t>& indices,
    		Matrix<DistanceType>& dists,
    		size_t knn,
    		const SearchParams& params) const
    {
 		KNNSimpleResultSet<DistanceType> resultSet(knn);
		int count = 0;
    	for (int i = 0; i < (int)queries.rows; i++) {
    		resultSet.clear();
    		findNeighbors(resultSet, queries[i], params);
    		size_t n = std::min(resultSet.size(), knn);
    		resultSet.copy(indices[i], dists[i], n, params.sorted);
    		indices_to_ids(indices[i], indices[i], n);
    		count += (int)n;
    	}

    	return count;
    }

    virtual void findNeighbors(ResultSet<DistanceType>& result, const ElementType* vec, const SearchParams& searchParams) const = 0;

protected:

    virtual void freeIndex() = 0;

    virtual void buildIndexImpl() = 0;

    void indices_to_ids(const size_t* in, size_t* out, size_t size) const
    {
		if (removed_) {
			for (size_t i=0;i<size;++i) {
				out[i] = ids_[in[i]];
			}
		}
    }

    void setDataset(const Matrix<ElementType>& dataset)
    {
    	size_ = dataset.rows;
    	veclen_ = dataset.cols;
    	last_id_ = 0;

    	ids_.clear();
    	removed_points_.clear();
    	removed_ = false;
    	removed_count_ = 0;

    	points_.resize(size_);
    	for (size_t i=0;i<size_;++i) {
    		points_[i] = dataset[i];
    	}
    }

    void extendDataset(const Matrix<ElementType>& new_points)
    {
    	size_t new_size = size_ + new_points.rows;
    	if (removed_) {
    		removed_points_.resize(new_size);
    		ids_.resize(new_size);
    	}
    	points_.resize(new_size);
    	for (size_t i=size_;i<new_size;++i) {
    		points_[i] = new_points[i-size_];
    		if (removed_) {
    			ids_[i] = last_id_++;
    			removed_points_.reset(i);
    		}
    	}
    	size_ = new_size;
    }

    void cleanRemovedPoints()
    {
    	if (!removed_) return;

    	size_t last_idx = 0;
    	for (size_t i=0;i<size_;++i) {
    		if (!removed_points_.test(i)) {
    			points_[last_idx] = points_[i];
    			ids_[last_idx] = ids_[i];
    			removed_points_.reset(last_idx);
    			++last_idx;
    		}
    	}
    	points_.resize(last_idx);
    	ids_.resize(last_idx);
    	removed_points_.resize(last_idx);
    	size_ = last_idx;
    	removed_count_ = 0;
    }

protected:

    /**
     * The distance functor
     */
    Distance distance_;

    /**
     * Each index point has an associated ID. IDs are assigned sequentially in
     * increasing order. This indicates the ID assigned to the last point added to the
     * index.
     */
    size_t last_id_;

    /**
     * Number of points in the index (and database)
     */
    size_t size_;

    /**
     * Number of features in the dataset when the index was last built.
     */
    size_t size_at_build_;

    /**
     * Size of one point in the index (and database)
     */
    size_t veclen_;

    /**
     * Parameters of the index.
     */
    IndexParams index_params_;

    /**
     * Flag indicating if at least a point was removed from the index
     */
    bool removed_;

    /**
     * Array used to mark points removed from the index
     */
    DynamicBitset removed_points_;

    /**
     * Number of points removed from the index
     */
    size_t removed_count_;

    /**
     * Array of point IDs, returned by nearest-neighbour operations
     */
    std::vector<size_t> ids_;

    /**
     * Point data
     */
    std::vector<ElementType*> points_;

    /**
     * Pointer to dataset memory if allocated by this index, otherwise NULL
     */
    ElementType* data_ptr_;

};

#define USING_BASECLASS_SYMBOLS \
		using NNIndex<Distance>::distance_;\
		using NNIndex<Distance>::size_;\
		using NNIndex<Distance>::size_at_build_;\
		using NNIndex<Distance>::veclen_;\
		using NNIndex<Distance>::index_params_;\
		using NNIndex<Distance>::removed_points_;\
		using NNIndex<Distance>::ids_;\
		using NNIndex<Distance>::removed_;\
		using NNIndex<Distance>::points_;\
		using NNIndex<Distance>::extendDataset;\
		using NNIndex<Distance>::setDataset;\
		using NNIndex<Distance>::cleanRemovedPoints;\
		using NNIndex<Distance>::indices_to_ids;
}

#endif //FLANN_NNINDEX_H
