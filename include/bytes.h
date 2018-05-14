#ifndef BYTES_H
#define BYTES_H

#define mod(X,Y) \
    ((((X) % (Y)) + (Y)) % Y)

#define lowerMultiple(SZ, D) \
    ((SZ) - ((SZ) % D))
#define upperMultiple(SZ, D) \
    (((SZ) + ((D)-1) ) - ( ((SZ) + ((D)-1) ) % D ))

#define alignUp(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))

#define REG_SZ (32) // how many bytes in a register

#define regSize(typename) (REG_SZ/sizeof(typename))
#define cacheSize(typename) (CACHE_LINE_SIZE/sizeof(typename))

/** @return True if power of two, only positive numbers */
#define isPowerOfTwo(n)		\
	(   (n > 0) && ((n == 0) || ((n & (n - 1)) == 0))   )


#define CACHE_LINE_SIZE (64) // likwid-topology: Cache line size:	64
#define L1_LINE_DN (CACHE_LINE_SIZE/sizeof(double)) // how many doubles in a line
// likwid-topology: Size
#define L1KiB (32)
#define CACHE_L1_SIZE (L1KiB*1024/2)
#define CACHE_L2_SIZE (256*1024/2)
#define CACHE_L3_SIZE (3*1024*1024/2)
// divided by 2 because we wont be able to fill L1 completely without throwing
// useful values out

// aproximate minimum number of lines L1 cache has
// (for this capacity)(min lines mean max associativ)
#define L1LINE_N ((L1KiB*1024/8)/CACHE_LINE_SIZE)

// 2048
#define L1_DN (CACHE_L1_SIZE/sizeof(double)) // how many doubles in L1 cache
#define L2_DN (CACHE_L2_SIZE/sizeof(double)) // how many doubles in L1 cache
#define L3_DN (CACHE_L3_SIZE/sizeof(double)) // how many doubles in L1 cache

// size of the block to fit one matrix in L1
#define MAX_BL1 ((long)sqrt(L1_DN))
// align to cache line
#define BL1 (lowerMultiple(MAX_BL1, L1_LINE_DN))
// 40 % 8 == 0, (40*40 < 2048)

// to fit 3 matrices
#define MAX_B3L1 ((long)sqrt(L1_DN/3))
#define B3L1 (lowerMultiple(MAX_B3L1, L1_LINE_DN))

#define MAX_B3L2 ((long)sqrt(L2_DN/3))
#define B3L2 (lowerMultiple(MAX_B3L2, B3L1))

#define MAX_B3L3 ((long)sqrt(L3_DN/3))
#define B3L3 (lowerMultiple(MAX_B3L3, B3L2))

// to fit 2 matrices
#define MAX_B2L1 ((long)sqrt(L1_DN/2))
#define B2L1 (lowerMultiple(MAX_B2L1, L1_LINE_DN))

#define MAX_B2L2 ((long)sqrt(L2_DN/2))
#define B2L2 (lowerMultiple(MAX_B2L2, B2L1))

#define MAX_B2L3 ((long)sqrt(L3_DN/2))
#define B2L3 (lowerMultiple(MAX_B2L3, B2L2))

#endif
