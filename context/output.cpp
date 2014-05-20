#include "output.h"
#include "input.h"
#include <sys/time.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace output
{
    map<string, long> timer_start;
    map<string, long> timer_runtime;
    
    map<string, struct timespec> timer_start_timespec;
    
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
    
    inline struct timespec current_time_timespec()
    {
        struct timespec ts;

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        ts.tv_sec = mts.tv_sec;
        ts.tv_nsec = mts.tv_nsec;
        
#else
        clock_gettime(CLOCK_REALTIME, &ts);
#endif
        
        return ts;
    }
    
    struct timespec diff(timespec start, timespec end)
    {
        timespec temp;
        
        if ((end.tv_nsec-start.tv_nsec)<0)
        {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        }
        else
        {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
    }
    
    void start_timer(string name)
    {
        timer_start_timespec[name] = current_time_timespec();
    }
    
    void stop_timer(string name)
    {
        struct timespec diff_val = diff(timer_start_timespec[name], current_time_timespec());
        timer_runtime[name] += diff_val.tv_sec*1000000000 + diff_val.tv_nsec;
    }
  
    string timer_statistics()
    {
        stringstream s;
        
        for (auto iter = timer_runtime.begin(); iter != timer_runtime.end(); ++iter)
        {
            s << "  (" << iter->first << ")\t" << iter->second/1000000000.0 << "\n";
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