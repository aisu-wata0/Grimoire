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

#define LogM(LogLvl_, Message_) \
gm::logger.logLvlMsg(LogLvl_); \
gm::logger << Message_; gm::logger.flush();

#ifdef NDEBUG
	#define LogDEBUG(_) {}; // do nothing
#else
	#define LogDEBUG(Message_) LogM(LogLvl::Debug, Message_);
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

template<std::ostream &outStream_>
class LogLine {
public:
	LogLine(std::ostream& out = std::clog, LogLvl level = LogLvl	::Warn)
		: logLvl_(level)
		, logLvlMsg_(level)
	{
	}

	void logLvlMsg(LogLvl level){ logLvlMsg_ = level; }

	void logLvlSet(LogLvl level){ logLvl_ = level; }

	void flush() { outStream_ << std::flush; }

	template<class T>
	std::ostream& operator<<(const T& thing) {
		if(logLvlMsg_ <= logLvl_){
			outStream_ << thing;
			return outStream_;
		} else {
			return nullStream_;
		}
	}
	// logger << std::endl;
	std::ostream& operator<<(std::ostream& (*f)(std::ostream&)) {
		if(logLvlMsg_ <= logLvl_) {
			f(outStream_);
			return outStream_;
		} else {
			return nullStream_;
		}
	}


//private:
	std::ofstream nullStream_;
	LogLvl logLvl_;
	LogLvl logLvlMsg_;
	//static LogFilter...
};

extern LogLine<std::clog> logger;
LogLine<std::clog> logger(std::clog, LogLvl::Warn);


}
#endif
