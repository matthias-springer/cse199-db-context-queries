#ifndef __context__output__
#define __context__output__

#include <iostream>
#include <map>
#include <time.h>
#include <string.h>
#include <sstream>

#define error(description) { std::cerr << "ERR  [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << endl; exit(1); }
#define show_info(description) { std::cerr << "INFO [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << endl; }
#define debug(description) { if (input::debug) { std::cout << "DBG  [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << endl; } }

#define error_n(description) { std::cerr << "\rERR  [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description; fflush(stdout); exit(1); }
#define show_info_n(description) { std::cerr << "\rINFO [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description; fflush(stdout); }
#define debug_n(description) { if (input::debug) { std::cout << "\rDBG   [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description; fflush(stdout); } }

using namespace std;

namespace output
{
    void start_timer(string name);
    void stop_timer(string name);
    string timer_statistics();
    void increment_stat(string name, int increment);
    void clear_stats();
    void show_stats();
    unsigned long xorshf96();
}

#endif
