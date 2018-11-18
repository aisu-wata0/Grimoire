#pragma once

#include <iostream>
#include <cstddef>
#include <chrono>
#include <sys/types.h>
#include <unistd.h>

#include "bytes.h"

namespace gm
{

using Clock = std::chrono::high_resolution_clock;

/**
 * @brief Stores time intervals	\n
 * Chronometer<TimerHistoryMax> timer;
 * timer.timePoints_[] is an array of TimerHistoryMax time_points
 * tick() returns the time since last tick() (or construction)	\n
 * use tick() before and tickAverage() after the code you want to measure to
 * get the averageTotal() in the end
 */
template<ssize_t size>
class Chronometer
{
public:
	Clock::time_point timePoints_[size]; // time_point history: circular array
	ssize_t c_; // current update index

	double totalTime_; // total time, for averaging
	size_t averagedNum_; // tickAverage()s taken

	void update(){
		timePoints_[c_] = Clock::now();
	}

	Chronometer()
		: c_(0)
		, totalTime_(0)
		, averagedNum_(0)
	{
		update();
	}
	/** @brief Reset chronometer state*/
	void init(){
		c_ = 0;
		update();
		initAverage();
	}
	/** @brief Reset chronometer averaging*/
	void initAverage(){
		totalTime_ = 0;
		averagedNum_ = 0;
	}
	/** @return starts counting towards tick() */
	void start(){
		c_ = (c_ + 1) % size;
		update();
	}
	/** @return the time since last tick() (or construction)  */
	double tick(){
		start();

		std::chrono::duration<double, std::milli> elapsed = timePoints_[c_] - timePoints_[mod((c_-1), (size))];
		return elapsed.count();
	}
	/** @brief Count this tick towards the average*/
	double tickAverage(){
		double lastTick = tick();

		totalTime_ += lastTick;
		++averagedNum_;

		return lastTick;
	}
	/** @return Current average from all tickAverage()s
	 * since construction or initAverage() */
	double averageTotal(){
		if(averagedNum_ == 0) return 0;

		return totalTime_/(double)averagedNum_;
	}
};

#define TimerHistoryMax 16
Chronometer<TimerHistoryMax> timer;


}
