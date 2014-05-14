#include "offset.h"
#include "context.h"
#include "input.h"

namespace offset
{
    long offset_begin(string name, DOMAIN_TYPE id)
    {
        //debug("returns " << offsets_for(name)[2 * (id + 1)]);
        return offsets_for(name)[2 * (id + 1)];
    }
    
    long offset_length(string name, DOMAIN_TYPE id)
    {
        //debug("returns " << offsets_for(name)[2 * (id + 1) + 1]);
        return offsets_for(name)[2 * (id + 1) + 1];
    }
    
    static long* offsets_for(string name)
    {
        if (offsets.find(name) == offsets.end())
        {
            open_offset_file(name);
        }
        
        return offsets[name];
    }
    
    static void open_offset_file(string name)
    {
        int fd;
        
        if ((fd = open(storage_offset_file_name(name).c_str(), O_RDWR | O_CREAT, (mode_t) 0600)) == -1)
            error("Failed to open offset file " << storage_offset_file_name(name) << ".");
        
        if (lseek(fd, OFFSETS_FILE_SIZE, SEEK_SET) == -1)
            error("Failed to stretch size for memory mapped offsets file.");
        
        if (write(fd, "", 1) == -1)
            error("Failed to write offsets file.");
        
        if ((offsets[name] = (long*) mmap(0, OFFSETS_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
            error("Failed to map offsets file to memory.");
        
        offset_files[name] = fd;
    }
    
    void close_offset_files()
    {
        for (map<string, int>::iterator iter = offset_files.begin(); iter != offset_files.end(); iter++)
        {
            show_info("Closing offset file for " << iter->first << ".");
            
            munmap(offsets[iter->first], OFFSETS_FILE_SIZE);
            close(iter->second);
        }
        
        offset_files.clear();
        offsets.clear();
        
        show_info("Closed all offset files.");
    }
    
    long next_offset_for(string name)
    {
        return offset_begin(name, -1);
    }
    
    void set_offsets_for(string name, DOMAIN_TYPE id, long begin, long length)
    {
        offsets_for(name)[2 * (id + 1)] = begin;
        offsets_for(name)[2 * (id + 1) + 1] = length;
    }
    
    void handle_bytes_written(string name, long length)
    {
        offsets_for(name)[0] += length;
    }
}
