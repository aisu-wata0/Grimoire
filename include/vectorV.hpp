#pragma once

#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <assert.h>
#include <exception>

#include "bytes.h"

#define unroll(v,n) for(size_t v = 0; v < n; ++v)
#define unroll2D(vi,ni, vj,nj) unroll(vi,ni)unroll(vj,nj)
#define vec(varr, _i_) for(size_t _i_ = 0; _i_ < varr.vecN(); ++_i_)

#define gm_vectorized_loop_(v, min,max, _i_, block, _vi_, blockVec)	\
{ 	\
	size_t _endVI;		\
	size_t _beginVI = v.loop(min, max, _endVI);		\
	for (size_t _i_ = min; _i_ < _beginVI; ++_i_) block		\
	for (size_t _vi_ = _beginVI/v.vecN(); _vi_ < _endVI; ++_vi_){		\
    blockVec		\
    }for (size_t _i_ = _endVI*v.vecN(); _i_ <= max; ++_i_) block		\
}
// Example of usage, where A,B,X are gm::vectorV<>s
// gm_vectorized_loop_(A, 0,size-1,
// 	i,  {
// 		X[i] += A[i] * B[i];
// 	},
// 	vi,  {
// 		X.atV(vi).v += A.atV(vi).v * B.atV(vi).v;
// 	}
// )

namespace gm
{

/**
 * @class Vec
 * @brief Vectorized type, performs operations in multiple elements at once
 * using Vector Extensions from gcc, see the compiler doc
 *
 * ```cpp
	Vec<double> a,b,c;
	c += a * b; // 4 doubles will be multiplied in one instruction
	// can be individualy be accessed like Vec<> is an array
	for(int i = 0; i < VecN(double); ++i)
		std::cout << c[i] << " ";
	std::cout << std::endl;
 * ```
 * the number of elems in the Vec is regSize(elem) */
template <typename T>
using Vec __attribute__ ((vector_size (REG_SZ))) = T;

/**
 * @brief Allocates size*sizeof(elem) and aligns it into a boundary-bit boundary
 * @param ppElement pointer to array (pointer)
 * @param size number of elems in your resulting pointer
 * @param boundary : Power of two, else the behavior is undefined
 * @return The pointer you should free, don't lose it
 */
void* al_allloc(size_t size, size_t boundary, std::unique_ptr<char[]> & pMem){
	size_t bytes = size + boundary-1;
	// pMem = std::move(std::make_unique<char[]>(bytes)); // Very slow
	pMem.reset(new char[bytes]);

	auto tmpPtr = (void*)pMem.get();
	return std::align(boundary, size, tmpPtr, bytes);
}
/**
 * @brief Calculated padded size
 * to align the end to a cache line and avoid cache trashing
 * will be a multiple of cacheSize(elem) and not a power of two
 */
template<typename elem>
size_t calcPadSize(size_t size){
	// add to make it multiple of cache line (padding)
	size = alignUp(size, cacheSize(elem));
	// make sure size is not a power of two, avoid cache trashing
	size_t cacheLinesMem = size/cacheSize(elem);
	if(cacheLinesMem >= L1LINE_N && isPowerOfTwo(cacheLinesMem))
		size += cacheSize(elem);
	return size;
}

/**
 * @brief Vectorized array, use this to use Vector Extensions easily	\n
 * Uses dynamic allocated aligned memory. The start of the array is 64 bytes aligned	\n
 * so the vectorization can be used in a loop from 0 to size()/vecN()
 * (== vecInd(size()) == sizeV()) \n
 * the remaining non vectorized elements are looped
 * from sizeV()*vecN() (== remStart() == remInd(sizeV())).
 * If you must loop from a index that is not multiple of vecN()
 * you have to loop through the first indexes until it reaches a multiple of 4.	\n
 * Example for a generic loop from start to end:	\n
 * ```cpp
	for (size_t i = min; i < A.beginVI(min); ++i) {
		X[i] += A[i] * B[i];
	}
	for (size_t vi = A.beginVI(min)/A.vecN();
	vi < A.endVI(max); ++vi) {
		X.atV(vi).v += A.atV(vi).v * B.atV(vi).v;
	}
	for (size_t i = A.endVI(max)*A.vecN(); i <= max; ++i) {
		X[i] += A[i] * B[i];
	}
 * ``` */
template<class elem>
class vectorV : std::vector<elem>
{
public:

	using value_typeV 				= Vec<elem>;
	using referenceV					= Vec<elem> &;
	using const_referenceV			= const Vec<elem> &;
	using pointerV						= Vec<elem> *;
	using const_pointerV				= const Vec<elem> *;
	using iteratorV					= Vec<elem> *;
	using const_iteratorV			= const Vec<elem> *;
	using reverse_iteratorV			= std::reverse_iterator<iteratorV>;
	using const_reverse_iteratorV	= std::reverse_iterator<const_iteratorV>;

	/** @brief n of elems in a Vec<> */
	size_type vecN() const { return regSize(elem); }

	/** @brief n of vec elems */
	// size_type sizeV() const { return sizeV_; }

	/** @brief Input the Index
	 * @return the VecIndex */
	size_type vecInd(size_type index) const { return index/vecN(); }

	/** @brief Vectorized access
	 * @return Vec<elem> in the vecIndex,
	 * it will contain elems (i*vecN()) to (i*vecN() + vecN() -1) */
	referenceV atV(size_type  i) {
		assert(i < sizeV_ && "vectorV vec access out of bounds");
		return arr_.v[i];
	}
	/** @copydoc atV(size_type) */
	const_referenceV atV(size_type  i) const {
		assert(i < sizeV_ && "vectorV vec access out of bounds");
		return arr_.v[i];
	}


	/** @brief begin vectorization index */
	size_type firstVI(size_type index) const {
		return alignUp(index, vecN());
	}
	/** @brief last vecIndex, last vectorized vecIndex,
	 * max element possible to vectorize being v[index] */
	size_type lastVI(size_type index) const {
		return (index+1)/vecN();
	}

	/** @brief begin vectorization index */
	size_type firstVI() const {
		return firstVI(0);
	}
	/** @brief end vecIndex, last vectorized vecIndex */
	size_type lastVI() const {
		return lastVI(size_);
	}


	referenceV frontV();
	const_referenceV frontV() const;
	referenceV backV();
	const_referenceV backV() const;

	/** @brief begin iterator */
	iteratorV beginV() { return &arr_.v[firstVI(0)]; }
	/** @brief end iterator */
	iteratorV endV() { return &arr_.v[lastVI(size())]; }

	/** @brief begin iterator */
	const_iteratorV cbeginV() const { return &arr_[0]; }
	/** @brief end iterator */
	const_iteratorV cendV() const { return &arr_.v[sizeV()]; }


	/** @brief begin vectorization iterator, the next one from index */
	iteratorV beginV(size_type index) {
		return &arr_.v[firstVI(index)];
	}
	iteratorV endV(size_type index) {
		return &arr_.v[lastVI(index)];
	}

	/** @copydoc beginV(size_type)  */
	const_iteratorV cbeginV(size_type index) const {
		return &arr_.v[firstVI(index)];
	}
	const_iteratorV cendV(size_type index) const {
		return &arr_.v[lastVI(index)];
	}


	size_type loop(size_type min, size_type max, size_type &endVecI) const {
		endVecI = endVI(max);
		return beginVI(min);
	}
};

}
