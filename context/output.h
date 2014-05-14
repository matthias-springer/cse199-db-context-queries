#ifndef __context__output__
#define __context__output__

#include <iostream>
#include <map>
#include <time.h>
#include <string.h>
#include <sstream>

#define error(description) { std::cerr << "ERR  [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << "\n"; exit(1); }
#define info(description) { std::cerr << "INFO [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << "\n"; }
#define debug(description) { if (input::debug) { std::cout << "INFO [" << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "] " << description << "\n"; } }

using namespace std;

namespace output
{
    void start_timer(string name);
    void stop_timer(string name);
    string timer_statistics();
}

#endif
