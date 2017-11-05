#ifndef CHRONOMETER_HPP
#define CHRONOMETER_HPP

#include <sys/time.h>
#include <iostream>
#include <cstddef>

namespace gm
{


/**
 * @return returns current clock time in seconds 
 */
double tv_sec(timeval* tp){
	return ((tp->tv_sec + tp->tv_usec*1.0e-6));
}
/**
 * @class Chronometer
 * @file Chronometer.hpp
 * @brief Stores time intervals
 * tick() returns the time since last tick() (or construction)
 * use tick() before and tickAverage() after the code you want to measure to
 * get the averageTotal() in the end
 */
template<ptrdiff_t size>
class Chronometer
{
public:
	timeval clock[size];
	//clock_t n_clock[size];
	ptrdiff_t c;
	double mTotalTime;
	size_t mAveragedNum;
	
	Chronometer(){
		init();
	}
	/** @brief Reset chronometer state*/
	void init(){
		gettimeofday(&clock[0], NULL);
		//n_clock[0] = std::clock();
		c = 0;
		initAverage();
	}
	/** @return the time since last tick() (or construction)  */
	double tick(){
		c = (c + 1) % size;
		gettimeofday(&clock[c], NULL);
		//n_clock[c] = std::clock();
		
		timeval elapsed_tv;
		elapsed_tv.tv_sec = clock[c].tv_sec - clock[mod(c-1, size)].tv_sec;
		elapsed_tv.tv_usec = clock[c].tv_usec - clock[mod(c-1, size)].tv_usec;
		
		//*cl = ((double)(n_clock[c] - n_clock[c-1]) / CLOCKS_PER_SEC);
		return tv_sec(&elapsed_tv);
	}
	/** @return starts counting towards tick() */
	void start(){
		c = (c + 1) % size;
		gettimeofday(&clock[c], NULL);
		//n_clock[c] = std::clock();
	}
	/** @brief Reset chronometer averaging*/
	void initAverage(){
		mTotalTime = 0;
		mAveragedNum = 0;
	}
	/** @brief Count this tick towards the average*/
	double tickAverage(){
		double lastTick = tick();
		mTotalTime += lastTick;
		mAveragedNum++;
		return lastTick;
	}
	/** @return Current average from all tickAverage()s
	 * since construction or initAverage() */
	double averageTotal(){
		if(mAveragedNum == 0) return 0;
		
		return mTotalTime/(double)mAveragedNum;
	}
};

#define TimerHistoryMax 16
Chronometer<TimerHistoryMax> timer;


}
#endif // CHRONOMETER_H
