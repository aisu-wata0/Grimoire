#ifndef VARRAY_HPP
#define VARRAY_HPP

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "bytes.h"

namespace gm
{
using namespace std;

/** @return True if power of two, only positive numbers */
template <typename num>
bool isPowerOfTwo (num n) {
	return (n > 0) && ((n == 0) || ((n & (n - 1)) == 0));
}

/** @return the smallest number >= x, multiple of arg 2 */
template <typename num>
num roundUpMultiple(num x, num multiple) {
    assert(multiple);
    return ((x + multiple - 1) / multiple) * multiple;
}

/** @brief How many of elem are there in a register of size REG_SZ */
template<typename elem>
inline size_t regSize() { return (REG_SZ/sizeof(elem)); }

/**
 * @class vec
 * @brief Vectorized type, performs operations in multiple elements at once
 * using Vector Extensions from gcc, see the compiler doc
 * vec<double> a,b,c; c.v += a.v * b.v;
 * the number of elems in the vec is regSize<elem>() */
template<typename elem>
struct vec
{
	elem __attribute__ ((vector_size (REG_SZ)))  v; // vectorization of elems
	
	inline const elem& operator[] (size_t i) const {
		assert(i < regSize<elem>() && "Vectorized elem out of register access");
		return v[i];
	}
	inline elem& operator[] (size_t i) {
		assert(i < regSize<elem>() && "Vectorized elem out of register access");
		return v[i];
	}
};
/**
 * @union vecp
 * @brief Union of a vec<elem> and elem pointers	\n
 * Used to store arrays to access vec<elem>s or single elems as needed */
template<typename elem>
union vecp
{
	vec<elem>*  v;
	elem* p;
};

/**
 * @brief Allocates size*sizeof(elem) and aligns it into a boundary-bit boundary
 * @param ppElement pointer to array (pointer)
 * @param size number of elems in your resulting pointer
 * @param boundary : Power of two, else the behavior is undefined
 * @return The pointer you should free, don't lose it
 */
template<typename elem>
void* al_allloc(elem** ppElement, size_t size, size_t boundary){
	*ppElement = (elem*)malloc((size)*sizeof(elem) + boundary-1);
	if(*ppElement == NULL){
		cerr <<"failed to malloc "<< (size)*sizeof(elem)/1024 <<" KiB"<< endl;
		exit(0);
	}
	void* pMem = *ppElement;
	*ppElement = (elem*)(((uintptr_t)pMem + (boundary-1)) & (uintptr_t)( ~ (boundary-1)));
	return pMem;
}
/**
 * @brief Calculated padded size
 * to align the end to a cache line and avoid cache trashing
 */
size_t calcPadSize(size_t size){
	// add to make it multiple of L1_LINE_DN (padding)
	size = roundUpMultiple(size, L1_LINE_DN);
	// make sure size is not a power of two, avoid cache trashing
	if(size > L1LINE_N-1 && isPowerOfTwo(size))
		size += L1_LINE_DN;
	return size;
}
/**
 * @brief Vectorized array, use this to use Vector Extensions easily	\n
 * Uses dynamic allocated aligned memory. The start of the array is 64 bytes aligned	\n
 * so the vectorization can be used in a loop from 0 to size()/vecN()
 * (== vecInd(size()) == sizeVec()) \n
 * the remaining non vectorized elements are looped
 * from sizeVec()*vecN() (== remStart() == remInd(sizeVec())).
 * If you must loop from a index that is not multiple of vecN()
 * you have to loop through the first indexes until it reaches a multiple of 4.	\n
 * Example for a generic loop from start to end:	\n
 * ```cpp
 * fstEnd = roundUpMultiple(start, vecN());
 * for(i = start; i < fstEnd; ++i) // fsrt loop // if start%vecN == 0, no loop
 * 		something(V[i]);
 * for(iv = V.vecInd(i); iv < V.vecInd(end); ++iv) // vec loop
 * 		something(V.atv(vi));
 * for(i = V.remInd(vi); i < end; ++i) // remaining loop // if end%vecN == 0, no loop
 * 		something(V.at(i));
 * ``` */
template<class elem>
class varray
{
protected:
	vecp<elem> arr; //!< pointer to elems
	size_t mSize; //!< n of elems
	size_t mSizeVec; //!< n of vec<elem>s
	
	size_t mSizeMem; //!< n of elem in memory
	size_t mSizeVecMem; //!< n of vec<elem>s in memory
	size_t mPad; //!< n of vec<elem>s in memory
	
	size_t mEndVec; //!< index where vectorization ends
	void* mpMem; //!< pointer to be freed
	
	/** @brief Allocates n elems (if existed: frees old varray pointer) */
	void memAlloc(size_t mSizeMem){
		if(mpMem != NULL) { free(mpMem); }
		mpMem = al_allloc(&arr.p, mSizeMem, CACHE_LINE_SIZE);
	}
	
public:
	/** @brief n of elems in a vec */
	inline size_t vecN() const { return regSize<elem>(); }
	
	/** @brief Sets size to n elems (if existed: frees old varray pointer) */
	void alloc(size_t size){
		mSize = size;
		mSizeVec = mSize/vecN();
		mSizeMem = calcPadSize(mSize);
		mSizeVecMem = mSizeMem/vecN();
		mEndVec = Lower_Multiple(mSize, vecN());
		mPad = mSizeMem - mSize;
		memAlloc(mSizeMem);
	}
	
	/** @brief Constructor @param size n of elems in the varray */
	varray(size_t size) {
		mpMem = NULL;
		alloc(size);
	}
	/** @brief empty constructor, call alloc before using */
	varray() {
		mpMem = NULL;
	}
	/** @brief destructor, frees array memory */
	~varray(){
		if(mpMem != NULL) { free(mpMem); }
	}
	
	/** @brief n of elems */
	size_t size() const { return mSize; }
	
	/** @brief n of elems in memory */
	size_t sizeMem() const { return mSizeMem; }
	
	/** @brief n of vec elems */
	size_t sizeVec() const { return mSizeVec; }
	
	/** @brief n of vec elems in memory*/
	size_t sizeVecMem() const { return mSizeVecMem; }
	
	/** @brief Input the index
	 * @return the vec index */
	size_t vecInd(size_t index) const { return index/vecN(); }
	
	/** @brief remaining loop start index */
	size_t remStart() const { return mEndVec; }
	
	/** @brief Input vec index
	 * @return index */
	size_t remInd(size_t index) const { return index*vecN(); }
	
	/** @brief Input the start of a loop
	 * @return the end of the loop before varray is vectorized */
	size_t fstEnd(size_t start, size_t end) const {
		size_t vecStart = roundUpMultiple(start, vecN());
		if(end < vecStart)
			return end;
		return vecStart;
	}
	
	/** @brief size of the padding */
	size_t pad() const { return mPad; }
	
	/** @brief Vectorized access
	 * @return vec<elem> in the vec index,
	 * it will contain elems (i*vecN()) to (i*vecN() + vecN() -1) */
	inline vec<elem>& atv(size_t i) {
		assert(i < mSizeVec && "varray vec access out of bounds");
		return arr.v[i];
	}
	/** @copydoc atv(size_t) */
	inline const vec<elem> & atv(size_t i) const {
		assert(i < mSizeVec && "varray vec access out of bounds");
		return arr.v[i];
	}
	
	/** @brief returns element at index*/
	inline elem& at(size_t i){
		assert(i < mSize && "varray access out of bounds");
		return arr.p[i];
	}
	/** @copydoc at(size_t) */
	inline const elem& at(size_t i) const {
		assert(i < mSize && "varray access out of bounds");
		return arr.p[i];
	}
	
	/** @brief begin iterator (pointer) */
	elem* begin() const { return &arr.p[0]; }
	/** @brief end iterator (pointer) */
	elem* end() const { return &arr.p[size()]; }
	
	/** @brief begin vec iterator (pointer) */
	vec<elem>* beginVec() const { return &arr.v[0]; }
	/** @brief end vec iterator (pointer) */
	vec<elem>* endVec() const { return &arr.v[sizeVec()]; }
	
	/** @copydoc at(size_t) */
	elem& operator[] (size_t i) {return at(i); }
	/** @copydoc at(size_t) */
	const elem& operator[] (size_t i) const { return at(i); }
};

/** @brief prints vector to cout; elems divided by spaces; no endl or flush */
template<class Cont>
void printv(Cont& C){
	for(size_t i = 0; i < C.size(); ++i)
		cout << C.at(i) <<" ";
}


}
#endif