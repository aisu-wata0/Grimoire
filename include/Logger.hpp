#ifndef LOG_HPP
#define LOG_HPP

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <ios>
#include <algorithm>

#define LogM(Level_, Message_) \
logger.msgLevel(Level_); \
logger << Message_; logger.flush();

#ifdef NDEBUG
	#define LogDEBUG(_) do {} while(false);
#else
	#define LogDEBUG(Message_) LogM(Level::Debug, Message_);
#endif

namespace gm
{

//!!! Subtract extra lines from LevelSTART_LINE if an entry takes more than one
constexpr auto LevelSTART_LINE = __LINE__;
enum class Level { // Each line has to be an enum, se above
	Fatal,
	Critical,
	Error,
	Warning,
	Notification,
	Information,
	Debug,
};
constexpr auto LevelMax = __LINE__ - LevelSTART_LINE - 4;

std::ostream& operator<<(std::ostream& out, const Level value){
	string s;
#define CASE_VAL(p) case(p): s = #p; break;
	switch(value){
		CASE_VAL(Level::Fatal);
		CASE_VAL(Level::Critical);
		CASE_VAL(Level::Error);
		CASE_VAL(Level::Warning);
		CASE_VAL(Level::Notification);
		CASE_VAL(Level::Information);
		CASE_VAL(Level::Debug);
	}
#undef CASE_VAL

	return out << s;
}

class LogLine {
public:
	LogLine(std::ostream& out = clog, Level level = Level::Warning)
	:	mOut(out),
		mLogLevel(level),
		mCurrentLevel(level){
	}
	
	template <class T>
	LogLine& operator<<(const T& thing) {
		if(mCurrentLevel <= mLogLevel)
			mOut << thing;
		return *this;
	}
	
	void msgLevel(Level level){ mCurrentLevel = level; }
	
	void setLevel(Level level){ mLogLevel = level; }
	
	void flush() { mOut << std::flush; }
	
private:
	std::stringstream mStream;
	std::ostream& mOut;
	Level mLogLevel;
	Level mCurrentLevel;
	//static LogFilter...
};

extern LogLine logger;
LogLine logger(clog, Level::Warning);


}
#endif
