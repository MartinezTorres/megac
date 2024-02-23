////////////////////////////////////////////////////////////////////////
// 
// Extended logging file (based on uSnippets::Log)
//
// Manuel Martinez (salutte@gmail.com)
//
// license: LGPLv3

#pragma once

#include <thread>
#include <mutex>
 
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

#define ASSERT_INFO "Assert in " << __func__ << " (" << __FILE__ << ":" << __LINE__ << ") "


enum LogLevel {
	EXTRA = -3,
	DEBUG = -2,
	INFO = -1,
	WARNING = 0,
	ERROR_NOTHROW = 1,
	ERROR = 2
};

class Log {
public:

	constexpr static auto ANSI_RESET = "\x1b[0m";
	constexpr static auto ANSI_BLUE  = "\x1b[34;1m";
	constexpr static auto ANSI_YELLOW = "\x1b[33;1m";
	constexpr static auto ANSI_RED   = "\x1b[31;1m";

	std::string msg() const { return sstr?sstr->str():""; }
	static LogLevel &report_level() { static LogLevel rl = WARNING; return rl; };
	static LogLevel &throw_level() { static LogLevel tl = ERROR; return tl; };

	Log (LogLevel level_) : level(level_), sstr(level_>=report_level()?new std::stringstream():nullptr) {}
	~Log() noexcept(false) {
		if (sstr) {
			static std::mutex mtx;
			static auto start = std::chrono::steady_clock::now();
			static auto last = std::chrono::steady_clock::now();

			std::lock_guard<std::mutex> lock(mtx);
			auto now = std::chrono::steady_clock::now();
			
			if ( level == INFO )    std::cerr << ANSI_BLUE;
			if ( level == WARNING ) std::cerr << ANSI_YELLOW;
			if ( level == ERROR_NOTHROW )   std::cerr << ANSI_RED;
			if ( level == ERROR )   std::cerr << ANSI_RED;
			
			std::boolalpha(std::cerr);
			std::cerr << "L" << std::setw( 2 ) << level << " ";				
			std::cerr << "[" << std::fixed << std::setw( 8 ) << std::setprecision( 4 )  << std::chrono::duration<double>(now-start).count() << "] ";
			last = now;

			int t = threadid();
			std::cerr << std::string( ((t-1)%6)*25,' ') ;
			std::cerr << "#" << t << "[" << std::fixed << std::setw( 8 ) << std::setprecision( 4 )  << threadTime(t) << "] ";
			
			std::cerr << sstr->str() << std::endl;
			
			std::cerr << ANSI_RESET;
			
			delete sstr;
		}
		
		if ( level >= throw_level()) throw std::runtime_error("Stopping due to previous errors.");
	}
	template<typename T> Log &operator<<(const T &v) { if (sstr) *sstr << v; return *this; }
	void operator<<(std::nullptr_t) {}
	
private:
	
	LogLevel level;
	std::stringstream * const sstr;
	int threadid() { 
		static std::map<std::thread::id,int> dict; 
		int ret = dict[std::this_thread::get_id()];
		if (not ret) ret = dict[std::this_thread::get_id()] = dict.size();
		return ret;
	}

	double threadTime(uint id) { 
		static std::vector<std::chrono::time_point<std::chrono::steady_clock>> last;
		if (id>=last.size()) last.resize(id+1, std::chrono::steady_clock::now());
		auto now = std::chrono::steady_clock::now();
		double ret = std::chrono::duration<double>(now-last[id]).count();
		last[id]=now;
		return ret;
	}		
};

/*struct Assert : Log { Assert(bool condition) : Log(condition ? DEBUG : ERROR) {} };

struct Str {

	std::ostringstream oss;

	template<typename T>
	Str &operator<<(const T &data) { oss << data;  return *this; }

	operator const std::string() const { return oss.str(); }
};*/

	

