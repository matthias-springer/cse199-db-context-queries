#ifndef __context__compression__
#define __context__compression__

#include <stdio.h>

namespace benchmark
{
    void cb_generate_tuples();
    void cb_uncompressed();
    void cb_huffman();
    void cb_rle();
}

#endif