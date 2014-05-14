#include "input.h"
#include "bit_storage.h"
#include "offset.h"

namespace input
{
    FILE *input_file;
    int storage_type;
    int debug;
    bool stats_visible;
    
    void store(string name)
    {
        show_info("Input number id and number of values.");
        store(name, read_value());
    }
    
    void store(string name, DOMAIN_TYPE id)
    {
        show_info("Input data for " << name << ":" << id << ". Waiting for count.");
        
        DOMAIN_TYPE count = read_value();
        show_info("Waiting for " << count << " values.");
        
        storage *s = storage::new_instance();
        
        for (int i = 0; i < count; ++i)
        {
            s->add(read_value());
        }
        
        pair<long, long> position;
        s->save_to_file(name, position);
        debug("Saved storage to file.");
        
        offset::set_offsets_for(name, id, position.first, position.second);
        debug("Saved offsets to offset file.");
    }
    
    DOMAIN_TYPE read_value()
    {
        DOMAIN_TYPE value;
        
        if (sizeof(DOMAIN_TYPE) == 1)
        {
            fscanf(input_file, "%c", &value);
        }
        else if (sizeof(DOMAIN_TYPE) == 4)
        {
            fscanf(input_file, "%i", &value);
        }
        else if (sizeof(DOMAIN_TYPE) == 8)
        {
            fscanf(input_file, "%ld", &value);
        }
        
        return value;
    }
}