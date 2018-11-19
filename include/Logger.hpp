#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <ios>
#include <algorithm>

namespace gm
{

//!!! Subtract extra lines from LogLvlSTART_LINE if an entry takes more than one
constexpr auto LogLvlSTART_LINE = __LINE__;
enum class LogLvl
{ // Each line has to be an enum, se above
	Fatal,
	Critical,
	Error,
	Warn,
	Note,
	Info,
	Debug,
};
constexpr auto LogLvlMax = __LINE__ - LogLvlSTART_LINE - 4;

std::ostream &operator<<(std::ostream &out, const LogLvl value)
{
	std::string s;
#define CASE_VAL(p) \
	case (p):        \
		s = #p;       \
		break;
	switch (value)
	{
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
   ```cpp
    gm::LogLine<gm::LogLvl::Warn> myLine(std::cout);
	// myLine will only log messages with Lvl Warn or higher like Fatal because of the constructor

	// "msg()" sets LogLvl of this and subsequent messages
  	myLine.msg(gm::LogLvl::Info) << "Information message" << std::endl;
	// so this wont be logged, myLine is on Warn Lvl

	// "msg()" changed LogLvl subsequent messages to Warn
  	myLine.msg(gm::LogLvl::Warn);
	myLine << "this message has LogLvl::Warn" << std::endl;
	
   // The following bad practice in isolation since you might have to find
	//  where the msg Lvl was last set by msg()
	myLine << "a message which I don't know immediatly what level it is" << std::endl;
   ```
 * @tparam logLvl_ Lvl of the LogLine
 * @param out   ostream to output logs
 */
template <LogLvl logLvl_>
class LogLine
{
 public:
	LogLine(std::ostream &outStream)
		 : outStream_(outStream),
			// logLvl_(level),
			setMsgLvl_(logLvl_)
	{
	}

	void flush() { outStream_ << std::flush; }

	LogLine &msg(const LogLvl &level){
		setMsgLvl_ = level;
		return *this;
	}

	template <class T>
	LogLine &operator<<(const T &thing){
		if (setMsgLvl_ <= logLvl_){
			outStream_ << thing;
			return *this;
		}
		else{
			return *this;
		}
	}
	// logger << std::endl;
	LogLine &operator<<(std::ostream &(*f)(std::ostream &)){
		if (setMsgLvl_ <= logLvl_){
			f(outStream_);
			return *this;
		}
		else{
			return *this;
		}
	}

private:
	std::ostream &outStream_;
	// const LogLvl logLvl_;
	LogLvl setMsgLvl_;
	//static LogFilter...
};

// // example declaration
// // in a common header between all files that use the line:
// extern gm::LogLine<gm::LogLvl::Warn> logger;
// // in main.cpp:
// gm::LogLine<gm::LogLvl::Warn> logger(std::clog);

} // namespace gm