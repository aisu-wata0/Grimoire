#pragma once

#include <stdint.h>
#include <cstddef>
#include <cstring>
#include <utility>
#include <iterator>
#include <stdexcept>
#include <cstdlib>
#include <stdlib.h>
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
// Example of usage, where A,B,X are gm::varray<>s
// gm_vectorized_loop_(A, 0,size-1,
// 	i,  {
// 		X[i] += A[i] * B[i];
// 	},
// 	vi,  {
// 		X.atv(vi).v += A.atv(vi).v * B.atv(vi).v;
// 	}
// )

namespace gm {

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
			X.atv(vi).v += A.atv(vi).v * B.atv(vi).v;
		}
		for (size_t i = A.endVI(max)*A.vecN(); i <= max; ++i) {
			X[i] += A[i] * B[i];
		}
	 * ``` */
	template <typename T>
	class vector {

		public:

			using value_type 					= T;
			using reference					= T &;
			using const_reference			= const T &;
			using pointer						= T *;
			using const_pointer				= const T *;
			using iterator						= T *;
			using const_iterator				= const T *;
			using reverse_iterator			= std::reverse_iterator<iterator>;
			using const_reverse_iterator	= std::reverse_iterator<const_iterator>;
			using difference_type			= ptrdiff_t;
			using size_type					= size_t;

			// Vec<> support
			using value_typeV 				= Vec<T>;
			using referenceV					= Vec<T> &;
			using const_referenceV			= const Vec<T> &;
			using pointerV						= Vec<T> *;
			using const_pointerV				= const Vec<T> *;
			using iteratorV					= Vec<T> *;
			using const_iteratorV			= const Vec<T> *;
			using reverse_iteratorV			= std::reverse_iterator<iteratorV>;
			using const_reverse_iteratorV	= std::reverse_iterator<const_iteratorV>;

			// 23.3.11.2, construct/copy/destroy:
			vector() noexcept;
			explicit vector(size_type n);
			vector(size_type n, const T &val);
			vector(iterator first, iterator last);
			vector(std::initializer_list<T>);
			vector(const vector<T> &);
			vector(vector<T> &&) noexcept;
			~vector() = default;;
			vector<T> & operator = (const vector<T> &);
			vector<T> & operator = (vector<T> &&);
			vector<T> & operator = (std::initializer_list<T>);
			void assign(size_type, const T &value);
			void assign(iterator, iterator);
			void assign(std::initializer_list<T>);

			// iterators:
			iterator begin() noexcept;
			const_iterator cbegin() const noexcept;
			iterator end() noexcept;
			const_iterator cend() const noexcept;
			reverse_iterator rbegin() noexcept;
			const_reverse_iterator crbegin() const noexcept;
			reverse_iterator rend() noexcept;
			const_reverse_iterator crend() const noexcept;

			// 23.3.11.3, capacity:
			bool empty() const noexcept;
			size_type size() const noexcept;
			size_type max_size() const noexcept;
			size_type capacity() const noexcept;
			void resize(size_type);
			void resize(size_type, const T &);
			void reserve(size_type);
			void shrink_to_fit();

			// element access
			reference operator [](size_type);
			const_reference operator [](size_type) const;
			reference at(size_type);
			const_reference at(size_type) const;
			reference front();
			const_reference front() const;
			reference back();
			const_reference back() const;

			// 23.3.11.4, data access:
			T * data() noexcept;
			const T * data() const noexcept;

			// 23.3.11.5, modifiers:
			template <class ... Args> void emplace_back(Args && ... args);
			void push_back(const T &);
			void push_back(T &&);
			void pop_back();

			template <class ... Args> iterator emplace(const_iterator, Args && ...);
			iterator insert(const_iterator, const T &);
			iterator insert(const_iterator, T &&);
			iterator insert(const_iterator, size_type, const T&);
			template <class InputIt> iterator insert(const_iterator, InputIt, InputIt);
			iterator insert(const_iterator, std::initializer_list<T>);
			iterator erase(const_iterator);
			iterator erase(const_iterator, const_iterator);
			void swap(vector<T> &);
			void clear() noexcept;

			bool operator == (const vector<T> &) const;
			bool operator != (const vector<T> &) const;
			bool operator < (const vector<T> &) const;
			bool operator <= (const vector<T> &) const;
			bool operator > (const vector<T> &) const;
			bool operator >= (const vector<T> &) const;

			/** @brief n of elems in a Vec<> */
			size_t vecN() const { return regSize(T); }

			size_type sizeV(){
				return size_/vecN();
			}
			/** @brief calculates how may Vec<>s should be allocated in memory */
			size_t rsrv_szV() const
			{
				size_t rsrv_szV = alignUp(rsrv_sz_, vecN()) / vecN();
				return rsrv_szV = calcPadSize<Vec<T>>(rsrv_szV);
			}

			// Memory management
			/** @brief allocates memory for sizeVMem Vec<T>s */
			T* memAlloc(std::unique_ptr<char[]> & ptr){
				T* arr;
				size_t bytes = rsrv_szV()*sizeof(Vec<T>);
				arr = (T*)al_allloc(bytes, CACHE_LINE_SIZE, ptr);

				assert((arr & (sizeof(Vec<T>) -1)) != 0  && "varray pointer not aligned to sizeof(Vec<elem>) bytes");
				return arr;
			}
			/** @brief allocates memory for sizeVMem Vec<T>s */
			void memAlloc(){
				arr_ = memAlloc(ptr_);
			}

			/** @brief Sets size to n elems, Allocates new memory no constructor call */
			void alloc(size_t const & size){
				size_ = size;

				memAlloc();
			}

		protected:

			size_type rsrv_sz_ = STARTING_SIZE;
			size_type size_ = 0;
			T *arr_;
			std::unique_ptr<char[]> ptr_; //!< only to store the pointer, no access

			// Constants

			static const size_type STARTING_SIZE = 4;
			static const size_type GROWTH_FACTOR = 2;
			static const size_type MAX_SZ = 1000000000;

			// Memory manipulation

			/** @brief Uses GROWTH_FACTOR to multiply current rsrv_sz_ */
			void grow();
			/** @brief Reallocates data into an array of size rsrv_sz_ */
			inline void reallocate();
	};



	template <typename T>
	vector<T>::vector() noexcept {
		memAlloc();
	}

	template <typename T>
	vector<T>::vector(typename vector<T>::size_type n) {
		size_type i;
		rsrv_sz_ = n;
		grow();
		memAlloc();
		for (i = 0; i < n; ++i)
			arr_[i] = T();
		size_ = n;
	}

	template <typename T>
	vector<T>::vector(typename vector<T>::size_type n, const T &value) {
		size_type i;
		rsrv_sz_ = n;
		grow();
		memAlloc();
		for (i = 0; i < n; ++i)
			arr_[i] = value;
		size_ = n;
	}

	template <typename T>
	vector<T>::vector(typename vector<T>::iterator first, typename vector<T>::iterator last) {
		size_type i, count = last - first;
		rsrv_sz_ = count;
		grow();
		memAlloc();
		for (i = 0; i < count; ++i, ++first)
			arr_[i] = *first;
		size_ = count;
	}

	template <typename T>
	vector<T>::vector(std::initializer_list<T> lst) {
		rsrv_sz_ = lst.size();
		grow();
		memAlloc();
		for (auto &item: lst)
			arr_[size_++] = item;
	}

	template <typename T>
	vector<T>::vector(const vector<T> &other) {
		size_type i;
		rsrv_sz_ = other.rsrv_sz_;
		memAlloc();
		for (i = 0; i < other.size_; ++i)
			arr_[i] = other.arr_[i];
		size_ = other.size_;
	}

	template <typename T>
	vector<T>::vector(vector<T> &&other) noexcept {
		size_type i;
		rsrv_sz_ = other.rsrv_sz_;
		memAlloc();
		for (i = 0; i < other.size_; ++i)
			arr_[i] = std::move(other.arr_[i]);
		size_ = other.size_;
	}

	template <typename T>
	vector<T> & vector<T>::operator = (const vector<T> &other) {
		size_type i;
		if (rsrv_sz_ < other.size_) {
			rsrv_sz_ = other.size_;
			grow();
			reallocate();
		}
		for (i = 0; i < other.size_; ++i)
			arr_[i] = other.arr_[i];
		size_ = other.size_;
	}

	template <typename T>
	vector<T> & vector<T>::operator = (vector<T> &&other) {
		size_type i;
		if (rsrv_sz_ < other.size_) {
			rsrv_sz_ = other.size_;
			grow();
			reallocate();
		}
		for (i = 0; i < other.size_; ++i)
			arr_[i] = std::move(other.arr_[i]);
		size_ = other.size_;
	}

	template <typename T>
	vector<T> & vector<T>::operator = (std::initializer_list<T> lst) {
		if (rsrv_sz_ < lst.size()) {
			rsrv_sz_ = lst.size();
			grow();
			reallocate();
		}
		size_ = 0;
		for (auto &item: lst)
			arr_[size_++] = item;
	}

	template <typename T>
	void vector<T>::assign(typename vector<T>::size_type count, const T &value) {
		size_type i;
		if (count > rsrv_sz_) {
			rsrv_sz_ = count;
			grow();
			reallocate();
		}
		for (i = 0; i < count; ++i)
			arr_[i] = value;
		size_ = count;
	}

	template <typename T>
	void vector<T>::assign(typename vector<T>::iterator first, typename vector<T>::iterator last) {
		size_type i, count = last - first;
		if (count > rsrv_sz_) {
			rsrv_sz_ = count;
			grow();
			reallocate();
		}
		for (i = 0; i < count; ++i, ++first)
			arr_[i] = *first;
		size_ = count;
	}

	template <typename T>
	void vector<T>::assign(std::initializer_list<T> lst) {
		size_type i, count = lst.size();
		if (count > rsrv_sz_) {
			rsrv_sz_ = count;
			grow();
			reallocate();
		}
		i = 0;
		for (auto &item: lst)
			arr_[i++] = item;
	}


	template <typename T>
	typename vector<T>::iterator vector<T>::begin() noexcept {
		return arr_;
	}

	template <typename T>
	typename vector<T>::const_iterator vector<T>::cbegin() const noexcept {
		return arr_;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::end() noexcept {
		return arr_ + size_;
	}

	template <typename T>
	typename vector<T>::const_iterator vector<T>::cend() const noexcept {
		return arr_ + size_;
	}

	template <typename T>
	typename vector<T>::reverse_iterator vector<T>::rbegin() noexcept {
		return reverse_iterator(arr_ + size_);
	}

	template <typename T>
	typename vector<T>::const_reverse_iterator vector<T>::crbegin() const noexcept {
		return reverse_iterator(arr_ + size_);
	}

	template <typename T>
	typename vector<T>::reverse_iterator vector<T>::rend() noexcept {
		return reverse_iterator(arr_);
	}

	template <typename T>
	typename vector<T>::const_reverse_iterator vector<T>::crend() const noexcept {
		return reverse_iterator(arr_);
	}


	template <typename T>
	inline void vector<T>::reallocate() {
		std::unique_ptr<char[]> tptr_;
		T* tarr_ = memAlloc(tptr_);
		memcpy(tarr_, arr_, size_ * sizeof(T));

		ptr_.swap(tptr_);
		arr_ = tarr_;
	}


	template <typename T>
	bool vector<T>::empty() const noexcept {
		return size_ == 0;
	}

	template <typename T>
	typename vector<T>::size_type vector<T>::size() const noexcept{
		return size_;
	}

	template <typename T>
	typename vector<T>::size_type vector<T>::max_size() const noexcept {
		return MAX_SZ;
	}

	template <typename T>
	typename vector<T>::size_type vector<T>::capacity() const noexcept {
		return rsrv_sz_;
	}

	template <typename T>
	void vector<T>::resize(typename vector<T>::size_type sz) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
		} else {
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i].~T();
		}
		size_ = sz;
	}

	template <typename T>
	void vector<T>::resize(typename vector<T>::size_type sz, const T &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		} else {
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i].~T();
		}
		size_ = sz;
	}

	template <typename T>
	void vector<T>::reserve(typename vector<T>::size_type _sz) {
		if (_sz > rsrv_sz_) {
			rsrv_sz_ = _sz;
			reallocate();
		}
	}

	template <typename T>
	void vector<T>::shrink_to_fit() {
		rsrv_sz_ = size_;
		reallocate();
	}


	template <typename T>
	typename vector<T>::reference vector<T>::operator [](typename vector<T>::size_type idx) {
		return arr_[idx];
	}

	template <typename T>
	typename vector<T>::const_reference vector<T>::operator [](typename vector<T>::size_type idx) const {
		return arr_[idx];
	}

	template <typename T>
	typename vector<T>::reference vector<T>::at(size_type pos) {
		if (pos < size_)
			return arr_[pos];
		else
			throw std::out_of_range("accessed position is out of range");
	}

	template <typename T>
	typename vector<T>::const_reference vector<T>::at(size_type pos) const {
		if (pos < size_)
			return arr_[pos];
		else
			throw std::out_of_range("accessed position is out of range");
	}

	template <typename T>
	typename vector<T>::reference vector<T>::front() {
		return arr_[0];
	}

	template <typename T>
	typename vector<T>::const_reference vector<T>::front() const {
		return arr_[0];
	}

	template <typename T>
	typename vector<T>::reference vector<T>::back() {
		return arr_[size_ - 1];
	}

	template <typename T>
	typename vector<T>::const_reference vector<T>::back() const {
		return arr_[size_ - 1];
	}


	template <typename T>
	T * vector<T>::data() noexcept {
		return arr_;
	}

	template <typename T>
	const T * vector<T>::data() const noexcept {
		return arr_;
	}


	template <typename T>
	template <class ... Args>
	void vector<T>::emplace_back(Args && ... args) {
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		arr_[size_] = std::move( T( std::forward<Args>(args) ... ) );
		++size_;
	}

	template <typename T>
	void vector<T>::push_back(const T &val) {
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		arr_[size_] = val;
		++size_;
	}

	template <typename T>
	void vector<T>::grow() {
		rsrv_sz_ <<= GROWTH_FACTOR;
	}

	template <typename T>
	void vector<T>::push_back(T &&val) {
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		arr_[size_] = std::move(val);
		++size_;
	}

	template <typename T>
	void vector<T>::pop_back() {
		--size_;
		arr_[size_].~T();
	}


	template <typename T>
	template <class ... Args>
	typename vector<T>::iterator vector<T>::emplace(typename vector<T>::const_iterator it, Args && ... args) {
		iterator iit = &arr_[it - arr_];
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		memmove(iit + 1, iit, (size_ - (it - arr_)) * sizeof(T));
		(*iit) = std::move( T( std::forward<Args>(args) ... ) );
		++size_;
		return iit;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::insert(typename vector<T>::const_iterator it, const T &val) {
		iterator iit = &arr_[it - arr_];
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		memmove(iit + 1, iit, (size_ - (it - arr_)) * sizeof(T));
		(*iit) = val;
		++size_;
		return iit;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::insert(typename vector<T>::const_iterator it, T &&val) {
		iterator iit = &arr_[it - arr_];
		if (size_ == rsrv_sz_) {
			grow();
			reallocate();
		}
		memmove(iit + 1, iit, (size_ - (it - arr_)) * sizeof(T));
		(*iit) = std::move(val);
		++size_;
		return iit;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::insert(typename vector<T>::const_iterator it, typename vector<T>::size_type cnt, const T &val) {
		iterator f = &arr_[it - arr_];
		if (!cnt) return f;
		if (size_ + cnt > rsrv_sz_) {
			rsrv_sz_ = (size_ + cnt);
			grow();
			reallocate();
		}
		memmove(f + cnt, f, (size_ - (it - arr_)) * sizeof(T));
		size_ += cnt;
		for (iterator it = f; cnt--; ++it)
			(*it) = val;
		return f;
	}

	template <typename T>
	template <class InputIt>
	typename vector<T>::iterator vector<T>::insert(typename vector<T>::const_iterator it, InputIt first, InputIt last) {
		iterator f = &arr_[it - arr_];
		size_type cnt = last - first;
		if (!cnt) return f;
		if (size_ + cnt > rsrv_sz_) {
			rsrv_sz_ = (size_ + cnt);
			grow();
			reallocate();
		}
		memmove(f + cnt, f, (size_ - (it - arr_)) * sizeof(T));
		for (iterator it = f; first != last; ++it, ++first)
			(*it) = *first;
		size_ += cnt;
		return f;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::insert(typename vector<T>::const_iterator it, std::initializer_list<T> lst) {
		size_type cnt = lst.size();
		iterator f = &arr_[it - arr_];
		if (!cnt) return f;
		if (size_ + cnt > rsrv_sz_) {
			rsrv_sz_ = (size_ + cnt);
			grow();
			reallocate();
		}
		memmove(f + cnt, f, (size_ - (it - arr_)) * sizeof(T));
		iterator iit = f;
		for (auto &item: lst) {
			(*iit) = item;
			++iit;
		}
		size_ += cnt;
		return f;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::erase(typename vector<T>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		(*iit).~T();
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(T));
		--size_;
		return iit;
	}

	template <typename T>
	typename vector<T>::iterator vector<T>::erase(typename vector<T>::const_iterator first, typename vector<T>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		for ( ; first != last; ++first)
			(*first).~T();
		memmove(f, last, (size_ - (last - arr_)) * sizeof(T));
		size_ -= last - first;
		return f;
	}

	template <typename T>
	void vector<T>::swap(vector<T> &rhs) {
		size_t tsize_ = size_,
			   trsrv_sz_ = rsrv_sz_;
		T *tarr_ = arr_;

		size_ = rhs.size_;
		rsrv_sz_ = rhs.rsrv_sz_;
		arr_ = rhs.arr_;

		rhs.size_ = tsize_;
		rhs.rsrv_sz_ = trsrv_sz_;
		rhs.arr_ = tarr_;
	}

	template <typename T>
	void vector<T>::clear() noexcept {
		size_type i;
		for (i = 0; i < size_; ++i)
			arr_[i].~T();
		size_ = 0;
	}


	template <typename T>
	bool vector<T>::operator == (const vector<T> &rhs) const {
		if (size_ != rhs.size_) return false;
		size_type i;
		for (i = 0; i < size_; ++i)
			if (arr_[i] != rhs.arr_[i])
				return false;
		return true;
	}

	template <typename T>
	bool vector<T>::operator != (const vector<T> &rhs) const {
		if (size_ != rhs.size_) return true;
		size_type i;
		for (i = 0; i < size_; ++i)
			if (arr_[i] != rhs.arr_[i])
				return true;
		return false;
	}

	template <typename T>
	bool vector<T>::operator < (const vector<T> &rhs) const {
		size_type i, j, ub = size_ < rhs.size_ ? size_ : rhs.size_;
		for (i = 0; i < ub; ++i)
			if (arr_[i] != rhs.arr_[i])
				return arr_[i] < rhs.arr_[i];
		return size_ < rhs.size_;
	}

	template <typename T>
	bool vector<T>::operator <= (const vector<T> &rhs) const {
		size_type i, j, ub = size_ < rhs.size_ ? size_ : rhs.size_;
		for (i = 0; i < ub; ++i)
			if (arr_[i] != rhs.arr_[i])
				return arr_[i] < rhs.arr_[i];
		return size_ <= rhs.size_;
	}

	template <typename T>
	bool vector<T>::operator > (const vector<T> &rhs) const {
		size_type i, j, ub = size_ < rhs.size_ ? size_ : rhs.size_;
		for (i = 0; i < ub; ++i)
			if (arr_[i] != rhs.arr_[i])
				return arr_[i] > rhs.arr_[i];
		return size_ > rhs.size_;
	}

	template <typename T>
	bool vector<T>::operator >= (const vector<T> &rhs) const {
		size_type i, j, ub = size_ < rhs.size_ ? size_ : rhs.size_;
		for (i = 0; i < ub; ++i)
			if (arr_[i] != rhs.arr_[i])
				return arr_[i] > rhs.arr_[i];
		return size_ >= rhs.size_;
	}

	template <>
	void vector<signed char>::resize(typename vector<signed char>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned char>::resize(typename vector<unsigned char>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<char>::resize(typename vector<char>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<short int>::resize(typename vector<short int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned short int>::resize(typename vector<unsigned short int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<int>::resize(typename vector<int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned int>::resize(typename vector<unsigned int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<long int>::resize(typename vector<long int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned long int>::resize(typename vector<unsigned long int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<long long int>::resize(typename vector<long long int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned long long int>::resize(typename vector<unsigned long long int>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<float>::resize(typename vector<float>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<double>::resize(typename vector<double>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}

	template <>
	void vector<long double>::resize(typename vector<long double>::size_type sz) {
		if (sz > rsrv_sz_) {
			rsrv_sz_ = sz;
			reallocate();
		}
		size_ = sz;
	}


	template <>
	void vector<signed char>::resize(typename vector<signed char>::size_type sz, const signed char &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned char>::resize(typename vector<unsigned char>::size_type sz, const unsigned char &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<char>::resize(typename vector<char>::size_type sz, const char &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<short int>::resize(typename vector<short int>::size_type sz, const short int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned short int>::resize(typename vector<unsigned short int>::size_type sz, const unsigned short int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<int>::resize(typename vector<int>::size_type sz, const int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned int>::resize(typename vector<unsigned int>::size_type sz, const unsigned int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<long int>::resize(typename vector<long int>::size_type sz, const long int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned long int>::resize(typename vector<unsigned long int>::size_type sz, const unsigned long int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<long long int>::resize(typename vector<long long int>::size_type sz, const long long int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<unsigned long long int>::resize(typename vector<unsigned long long int>::size_type sz, const unsigned long long int &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<float>::resize(typename vector<float>::size_type sz, const float &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<double>::resize(typename vector<double>::size_type sz, const double &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<long double>::resize(typename vector<long double>::size_type sz, const long double &c) {
		if (sz > size_) {
			if (sz > rsrv_sz_) {
				rsrv_sz_ = sz;
				reallocate();
			}
			size_type i;
			for (i = size_; i < sz; ++i)
				arr_[i] = c;
		}
		size_ = sz;
	}

	template <>
	void vector<signed char>::pop_back() {
		--size_;
	}

	template <>
	void vector<unsigned char>::pop_back() {
		--size_;
	}

	template <>
	void vector<char>::pop_back() {
		--size_;
	}

	template <>
	void vector<short int>::pop_back() {
		--size_;
	}

	template <>
	void vector<unsigned short int>::pop_back() {
		--size_;
	}

	template <>
	void vector<int>::pop_back() {
		--size_;
	}

	template <>
	void vector<unsigned int>::pop_back() {
		--size_;
	}

	template <>
	void vector<long int>::pop_back() {
		--size_;
	}

	template <>
	void vector<unsigned long int>::pop_back() {
		--size_;
	}

	template <>
	void vector<long long int>::pop_back() {
		--size_;
	}

	template <>
	void vector<unsigned long long int>::pop_back() {
		--size_;
	}

	template <>
	void vector<float>::pop_back() {
		--size_;
	}

	template <>
	void vector<double>::pop_back() {
		--size_;
	}

	template <>
	void vector<long double>::pop_back() {
		--size_;
	}


	template <>
	typename vector<signed char>::iterator vector<signed char>::erase(typename vector<signed char>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(signed char));
		--size_;
		return iit;
	}

	template <>
	typename vector<unsigned char>::iterator vector<unsigned char>::erase(typename vector<unsigned char>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(unsigned char));
		--size_;
		return iit;
	}

	template <>
	typename vector<char>::iterator vector<char>::erase(typename vector<char>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(char));
		--size_;
		return iit;
	}

	template <>
	typename vector<short int>::iterator vector<short int>::erase(typename vector<short int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(short int));
		--size_;
		return iit;
	}

	template <>
	typename vector<unsigned short int>::iterator vector<unsigned short int>::erase(typename vector<unsigned short int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(unsigned short int));
		--size_;
		return iit;
	}

	template <>
	typename vector<int>::iterator vector<int>::erase(typename vector<int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(int));
		--size_;
		return iit;
	}

	template <>
	typename vector<unsigned int>::iterator vector<unsigned int>::erase(typename vector<unsigned int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(unsigned int));
		--size_;
		return iit;
	}

	template <>
	typename vector<long int>::iterator vector<long int>::erase(typename vector<long int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(long int));
		--size_;
		return iit;
	}

	template <>
	typename vector<unsigned long int>::iterator vector<unsigned long int>::erase(typename vector<unsigned long int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(unsigned long int));
		--size_;
		return iit;
	}

	template <>
	typename vector<long long int>::iterator vector<long long int>::erase(typename vector<long long int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(long long int));
		--size_;
		return iit;
	}

	template <>
	typename vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(typename vector<unsigned long long int>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(unsigned long long int));
		--size_;
		return iit;
	}

	template <>
	typename vector<float>::iterator vector<float>::erase(typename vector<float>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(float));
		--size_;
		return iit;
	}

	template <>
	typename vector<double>::iterator vector<double>::erase(typename vector<double>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(double));
		--size_;
		return iit;
	}

	template <>
	typename vector<long double>::iterator vector<long double>::erase(typename vector<long double>::const_iterator it) {
		iterator iit = &arr_[it - arr_];
		memmove(iit, iit + 1, (size_ - (it - arr_) - 1) * sizeof(long double));
		--size_;
		return iit;
	}


	template <>
	typename vector<signed char>::iterator vector<signed char>::erase(typename vector<signed char>::const_iterator first, typename vector<signed char>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(signed char));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<unsigned char>::iterator vector<unsigned char>::erase(typename vector<unsigned char>::const_iterator first, typename vector<unsigned char>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(unsigned char));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<char>::iterator vector<char>::erase(typename vector<char>::const_iterator first, typename vector<char>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(char));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<short int>::iterator vector<short int>::erase(typename vector<short int>::const_iterator first, typename vector<short int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(short int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<unsigned short int>::iterator vector<unsigned short int>::erase(typename vector<unsigned short int>::const_iterator first, typename vector<unsigned short int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(unsigned short int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<int>::iterator vector<int>::erase(typename vector<int>::const_iterator first, typename vector<int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<unsigned int>::iterator vector<unsigned int>::erase(typename vector<unsigned int>::const_iterator first, typename vector<unsigned int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(unsigned int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<long long int>::iterator vector<long long int>::erase(typename vector<long long int>::const_iterator first, typename vector<long long int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(long long int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<unsigned long long int>::iterator vector<unsigned long long int>::erase(typename vector<unsigned long long int>::const_iterator first, typename vector<unsigned long long int>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(unsigned long long int));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<float>::iterator vector<float>::erase(typename vector<float>::const_iterator first, typename vector<float>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(float));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<double>::iterator vector<double>::erase(typename vector<double>::const_iterator first, typename vector<double>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(double));
		size_ -= last - first;
		return f;
	}

	template <>
	typename vector<long double>::iterator vector<long double>::erase(typename vector<long double>::const_iterator first, typename vector<long double>::const_iterator last) {
		iterator f = &arr_[first - arr_];
		if (first == last) return f;
		memmove(f, last, (size_ - (last - arr_)) * sizeof(long double));
		size_ -= last - first;
		return f;
	}


	template <>
	void vector<signed char>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<unsigned char>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<char>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<short int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<unsigned short int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<unsigned int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<long int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<unsigned long int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<long long int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<unsigned long long int>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<float>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<double>::clear() noexcept {
		size_ = 0;
	}

	template <>
	void vector<long double>::clear() noexcept {
		size_ = 0;
	}


}
