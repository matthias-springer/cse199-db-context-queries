#ifndef __context__map_column_storage__
#define __context__map_column_storage__

#include <iostream>
#include "storage.h"
#include <unordered_map>

template<class TAttribute>
class map_column_storage : public storage
{
public:
    unordered_map<DOMAIN_TYPE, TAttribute> *data;
    
    map_column_storage()
    {
        type = STORAGE_TYPE_MAP_COLUMN;
        data = new unordered_map<DOMAIN_TYPE, TAttribute>();
    }
    
    virtual ~map_column_storage()
    {
        delete data;
    }
    
    virtual void intersect(storage& another_storage)
    {
        error("Intersect not implemented.");
    }
    
    virtual void load_from_file(string name, long offset, long length)
    {
        output::start_timer("io/load_map_column");
        
        delete data;
        data = new unordered_map<DOMAIN_TYPE, TAttribute>();
        
        long cnt_elements = length / (sizeof(DOMAIN_TYPE) + sizeof(TAttribute));
        
        FILE *data_file = fopen(storage_data_file_name(name).c_str(), "r");
        fseek(data_file, offset, 0);
        
        DOMAIN_TYPE key_buffer;
        TAttribute value_buffer;
        
        for (int i = 0; i < cnt_elements; ++i)
        {
            fread(&key_buffer, sizeof(DOMAIN_TYPE), 1, data_file);
            fread(&value_buffer, sizeof(TAttribute), 1, data_file);
            
            (*data)[key_buffer] = value_buffer;
        }
        
        fclose(data_file);
        
        output::increment_stat("io-bytes/map_column-read", length);
        output::stop_timer("io/load_map_column");
    }
    
    virtual long save_to_file(string name, pair<long, long> &position)
    {
        output::start_timer("io/save_map_column");
        
        long write_pos = offset::next_offset_for(name);
        FILE *file = fopen(storage_data_file_name(name).c_str(), "a");
        
        fseek(file, write_pos, 0);
        for (auto &p : *data)
        {
            fwrite(&p.first, sizeof(DOMAIN_TYPE), 1, file);
            fwrite(&p.second, sizeof(TAttribute), 1, file);
        }
        
        long bytes_written = ftell(file) - write_pos;
        offset::handle_bytes_written(name, bytes_written);
        
        fclose(file);
        
        position = make_pair(write_pos, bytes_written);
        
        output::increment_stat("io-bytes/map_column-written", (int) bytes_written);
        output::stop_timer("io/save_map_column");
        
        return bytes_written;
    }
    
    virtual long count()
    {
        return data->size();
    }
    
    virtual vector<DOMAIN_TYPE> *elements()
    {
        vector<DOMAIN_TYPE> *result = new vector<DOMAIN_TYPE>();
        
        for (auto iter = data->begin(); iter != data->end(); ++iter)
        {
            result->push_back(iter->first);
        }
        
        return result;
    }
    
    unordered_map<DOMAIN_TYPE, TAttribute> *elements_with_columns()
    {
        return data;
    }
    
    virtual void add(DOMAIN_TYPE key, TAttribute attribute)
    {
        (*data)[key] = attribute;
    }
    
    virtual void add(DOMAIN_TYPE item)
    {
        error("Attempt to use map_column_storage without column.");
    }
    
    static map_column_storage<TAttribute> *load(string name, DOMAIN_TYPE id)
    {
        map_column_storage<TAttribute> *result = new map_column_storage<TAttribute>();
        result->load_from_file(name, offset::offset_begin(name, id), offset::offset_length(name, id));
        return result;
    }
};

#endif
