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

#define vectorized_loop(v, min,max, _i_, block, _vi_, blockVec)	\
{ 	\
	size_t _endVI;		\
	size_t _beginVI = v.loop(min, max, _endVI);		\
	for (size_t _i_ = min; _i_ < _beginVI; ++_i_) block		\
	for (size_t _vi_ = _beginVI/v.vecN(); _vi_ < _endVI; ++_vi_){		\
    blockVec		\
    }for (size_t _i_ = _endVI*v.vecN(); _i_ <= max; ++_i_) block		\
}
// Example of usage, where A,B,X are gm::varray<>s
// vectorized_loop(A, 0,size-1,
// 	i,  {
// 		X[i] += A[i] * B[i];
// 	},
// 	vi,  {
// 		X.atv(vi).v += A.atv(vi).v * B.atv(vi).v;
// 	}
// )

namespace gm
{

/**
 * @class Vec
 * @brief Vectorized type, performs operations in multiple elements at once
 * using Vector Extensions from gcc, see the compiler doc
 * Vec<double> a,b,c; c.v += a.v * b.v;
 * the number of elems in the vec is regSize(elem) */
template<typename elem>
struct alignas(regSize(elem)*sizeof(elem)) Vec
{
	elem __attribute__ ((vector_size (REG_SZ)))  v; // vectorization of elems
	// elem v[regSize(elem)];
};
/**
 * @union vecp
 * @brief Union of a Vec<elem> and elem pointers	\n
 * Used to store arrays to access Vec<elem>s or single elems as needed */
template<typename elem>
union Vecp
{
	Vec<elem>*  v;
	elem* p;

	const elem& operator[] (size_t i) const {
		return p[i];
	}
	elem& operator[] (size_t i) {
		return p[i];
	}
};
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
		X.atv(vi).v += A.atv(vi).v * B.atv(vi).v;
	}
	for (size_t i = A.endVI(max)*A.vecN(); i <= max; ++i) {
		X[i] += A[i] * B[i];
	}
 * ``` */
template<class elem>
class varray
{
protected:
	Vecp<elem> arr_; //!< pointer to elems

	size_t size_; //!< n of elems
	size_t sizeV_; //!< n of Vec<elem>s

	std::unique_ptr<char[]> pointer_; //!< only to store the pointer, no access
public:

	using iterator = elem*;
	using const_iterator = const elem*;

	/** @brief n of elems in a Vec<> */
	size_t vecN() const { return regSize(elem); }

	/** @brief allocates memory for sizeVMem Vec<elem>s */
	void memAlloc(const size_t & sizeVMem){
		size_t bytes = sizeVMem*sizeof(Vec<double>);
		arr_.v = (Vec<elem>*)al_allloc(bytes, CACHE_LINE_SIZE, pointer_);

		assert((arr_.v & (sizeof(Vec<elem>) -1)) != 0  && "varray pointer not aligned to sizeof(Vec<elem>) bytes");
	}

	/** @brief calculates how may Vec<>s should be allocated in memory */
	size_t sizeVMem() const
	{
		size_t sizeVMem = alignUp(size_, vecN()) / vecN();
		return sizeVMem = calcPadSize<Vec<elem>>(sizeVMem);
	}

	/** @brief Sets size to n elems, Allocates new memory */
	void alloc(size_t const & size){
		size_ = size;
		sizeV_ = size_/vecN();

		this->memAlloc(sizeVMem());
	}

	/** @brief Constructor @param size n of elems in the varray */
	varray(size_t size)
		: size_(size)
		, sizeV_(size_/vecN())
	{
		this->memAlloc(sizeVMem());
	}

	/** @brief Default destructor */
	~varray() = default;

	// Copy constructor
	varray(const varray & other)
		: size_(other.size_)
		, sizeV_(other.sizeV_)
	{
		this->memAlloc(sizeVMem());
		std::copy(other.begin(), other.end(), this->begin());
	}

	// Assignment
	varray & operator=(const varray & other){
		varray tmp(other);

		std::swap(*this, tmp);

		return *this;
	}

	// Move constructor
	varray(varray && other)
		: size_(other.size_)
		, sizeV_(other.sizeV_)
		, arr_(other.arr_)
		, pointer_(std::move(other.pointer_))
	{
		other.size_ = 0;
		other.sizeV_ = 0;
		other.arr_ = nullptr;
	}

	// Move assignment
	varray & operator=(varray && other)
	{
		varray tmp(std::move(other));

		std::swap(*this, tmp);

		return *this;
	}

	/** @brief n of elems */
	size_t size() const { return size_; }

	/** @brief n of vec elems */
	size_t sizeV() const { return sizeV_; }

	/** @brief Input the Index
	 * @return the VecIndex */
	size_t vecInd(size_t index) const { return index/vecN(); }

	/** @brief Vectorized access
	 * @return Vec<elem> in the vecIndex,
	 * it will contain elems (i*vecN()) to (i*vecN() + vecN() -1) */
	Vec<elem>& atv(size_t i) {
		assert(i < sizeV_ && "varray vec access out of bounds");
		return arr_.v[i];
	}
	/** @copydoc atv(size_t) */
	const Vec<elem> & atv(size_t i) const {
		assert(i < sizeV_ && "varray vec access out of bounds");
		return arr_.v[i];
	}

	/** @brief returns element at index */
	elem& at(size_t i){
		assert(i < size_ && "varray access out of bounds");
		return arr_[i];
	}
	/** @copydoc at(size_t) */
	const elem& at(size_t i) const {
		assert(i < size_ && "varray access out of bounds");
		return arr_[i];
	}

	/** @brief begin iterator */
	elem* begin() const { return &arr_[0]; }
	/** @brief end iterator */
	elem* end() const { return &arr_[size()]; }

	/** @brief begin vectorization index */
	size_t beginVI(size_t index) const {
		return alignUp(index, vecN());
	}

	/** @brief begin vectorization iterator */
	Vec<elem>* beginV() const { return &arr_.v[0]; }
	/** @brief begin vectorization iterator, the next one from index */
	size_t beginV(size_t index) const {
		return &arr_.v[beginVI(index)];
	}

	/** @brief end vecIndex, last vectorized vecIndex */
	size_t endVI() const {
		return sizeV();
	}
	/** @brief end vecIndex, last vectorized vecIndex,
	 * max element possible to vectorize being v[index] */
	size_t endVI(size_t index) const {
		return (index+1)/vecN();
	}

	/** @brief end vecIterator */
	Vec<elem>* endV() const { return &arr_.v[sizeV()]; }
	/** @brief end vecIterator
	 * max element possible to vectorize being v[index] */
	Vec<elem>* endV(size_t index) const {
		return &arr_.v[endVI(index)];
	}

	size_t loop(size_t min, size_t max, size_t &endVecI) const {
		endVecI = endVI(max);
		return beginVI(min);
	}


	/** @copydoc at(size_t) */
	elem& operator[] (size_t i) {return at(i); }
	/** @copydoc at(size_t) */
	const elem& operator[] (size_t i) const { return at(i); }
};

}
