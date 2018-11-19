#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <ios>
#include <algorithm>

#define LogM(logLine_, LogLvl_, Message_) \
logLine_.setMsgLvl(LogLvl_); \
logLine_ << Message_; logLine_.flush();

#ifdef NDEBUG
	#define LogDEBUG(logLine_, Message_) {}; // do nothing
#else
	#define LogDEBUG(logLine_, Message_) LogM(logLine_, LogLvl::Debug, Message_);
#endif

namespace gm
{

//!!! Subtract extra lines from LogLvlSTART_LINE if an entry takes more than one
constexpr auto LogLvlSTART_LINE = __LINE__;
enum class LogLvl { // Each line has to be an enum, se above
	Fatal,
	Critical,
	Error,
	Warn,
	Note,
	Info,
	Debug,
};
constexpr auto LogLvlMax = __LINE__ - LogLvlSTART_LINE - 4;

std::ostream& operator<<(std::ostream& out, const LogLvl value){
	std::string s;
#define CASE_VAL(p) case(p): s = #p; break;
	switch(value){
		CASE_VAL(LogLvl::Fatal);
		CASE_VAL(LogLvl::Critical);
		CASE_VAL(LogLvl::Error);
		CASE_VAL(LogLvl::Warn);
		CASE_VAL(LogLvl::Note);
		CASE_VAL(LogLvl::Info);
		CASE_VAL(LogLvl::Debug);
	}
#undef CASE_VAL

	return out << s;
}

/**
 * LogLine is used to filter messages by levels, using gm::LogLvl enum
 *
 * example
 * ```cpp
   gm::LogLine<std::cout> myLine(gm::LogLvl::Info);
	// myLine will only log messages with LogLvl of tier Info or higher like Warn

	// "setMsgLvl()" changes LogLvl of this and subsequent messages
  	myLine.setMsgLvl(gm::LogLvl::Info)
  	<< "Information message" << std::endl;

	myLine << "this message also has LogLvl::Info" << std::endl;

	// "setMsgLvl()" changed LogLvl subsequent messages to Warn
  	myLine.setMsgLvl(gm::LogLvl::Warn);
	myLine << "this message has LogLvl::Warn" << std::endl;

	// "msg()" sends a message of LogLvl given
  	myLine.msg(gm::LogLvl::Note) << "one time Note without changing Line Msg LogLvl" << std::endl;

	// myLine will only log messages with LogLvl of tier Critical or higher like Fatal
	myLine.setLvl(LogLvl::Critical);

	//  setMsgLvl was set to Warn beforehand by setMsgLvl()
	myLine << "another message that has LogLvl::Warn" << std::endl;
	// so this wont be logged

 * ```
 * @param out   [description]
 * @param level [description]
 */
class LogLine {
public:
  LogLine(std::ostream &outStream, LogLvl level = LogLvl::Warn)
		: outStream_(outStream), logLvl_(level), setMsgLvl_(level)
	{
	}

	void setLvl(LogLvl level){ logLvl_ = level; }

	void flush() { outStream_ << std::flush; }

	LogLine& setMsgLvl(LogLvl level){
		setMsgLvl_ = level;
		return *this;
	}

	std::ostream& msg(LogLvl level){
		if(level <= logLvl_){
			return outStream_;
		} else {
			return nullStream_;
		}
	}

	template<class T>
	std::ostream& operator<<(const T& thing) {
		if(setMsgLvl_ <= logLvl_){
			outStream_ << thing;
			return outStream_;
		} else {
			return nullStream_;
		}
	}
	// logger << std::endl;
	std::ostream& operator<<(std::ostream& (*f)(std::ostream&)) {
		if(setMsgLvl_ <= logLvl_) {
			f(outStream_);
			return outStream_;
		} else {
			return nullStream_;
		}
	}


//private:
	std::ostream &outStream_;
	std::ofstream nullStream_;
	LogLvl logLvl_;
	LogLvl setMsgLvl_;
	//static LogFilter...
};

extern LogLine logger;
LogLine logger(std::clog, LogLvl::Warn);

}
