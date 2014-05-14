#include "output.h"
#include "input.h"
#include <sys/time.h>

namespace output
{
    map<string, long> timer_start;
    map<string, long> timer_runtime;
    
    // random number generator: Marsaglia
    static unsigned long x=123456789, y=362436069, z=521288629;
    
    unsigned long xorshf96(void)
    {
        // period 2^96-1
        
        unsigned long t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;
        
        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        
        return z;
    }
    
    inline long current_time_millis()
    {
        //struct timeval t;
        //gettimeofday(&t, NULL);
        return time(NULL);
    }
    
    void start_timer(string name)
    {
        timer_start[name] = current_time_millis();
    }
    
    void stop_timer(string name)
    {
        timer_runtime[name] += current_time_millis() - timer_start[name];
    }
  
    string timer_statistics()
    {
        stringstream s;
        
        for (auto iter = timer_runtime.begin(); iter != timer_runtime.end(); ++iter)
        {
            s << "  (" << iter->first << ")\t" << iter->second << "\n";
        }
        
        if (timer_runtime["io-bytes/bitvector-written-uncompressed"] != 0)
        {
            s << "  (io-bytes/bitvector-written-cu-ratio)\t" << 100.0*timer_runtime["io-bytes/bitvector-written-compressed"] / timer_runtime["io-bytes/bitvector-written-uncompressed"] << " %\n";
        }
        
        return s.str();
    }
    
    void increment_stat(string name, int increment)
    {
        timer_runtime[name] += increment;
    }
    
    void clear_stats()
    {
        timer_runtime.clear();
        timer_start.clear();
    }
    
    void show_stats()
    {
        if (input::stats_visible)
        {
            show_info("Runtime statistics:\n" << output::timer_statistics());
        }
    }
}