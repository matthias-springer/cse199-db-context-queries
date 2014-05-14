#include "output.h"

namespace output
{
    map<string, long> timer_start;
    map<string, long> timer_runtime;
    
    
    void start_timer(string name)
    {
        timer_start[name] = time(NULL);
    }
    
    void stop_timer(string name)
    {
        timer_runtime[name] += time(NULL) - timer_start[name];
    }
  
    string timer_statistics()
    {
        stringstream s;
        
        for (auto iter = timer_runtime.begin(); iter != timer_runtime.end(); ++iter)
        {
            s << "  (" << iter->first << ")\t" << iter->second << "\n";
        }
        
        return s.str();
    }
}