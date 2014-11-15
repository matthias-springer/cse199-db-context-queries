#include "huffman_self_test.h"
#include "huffman.h"
#include <stdlib.h>
#include "output.h"

void huffman_self_test()
{
    unsigned short* terms_uncompressed = new unsigned short[1000];
    
    for (int i = 0; i < 1000; ++i)
    {
        terms_uncompressed[i] = rand() % 26000;
    }
    
    Node<unsigned short>* term_tree;
    unsigned short* term_huffman_array;
    bool* term_terminator_array;
    
    generate_array_tree_representation(terms_uncompressed, 1000, term_huffman_array, term_terminator_array, term_tree);
    encoding_dict<unsigned short> encoding_dict_terms;
    build_inverse_mapping(term_tree, encoding_dict_terms);
    
    char* terms_compressed;
    encode(terms_uncompressed, 1000, terms_compressed, encoding_dict_terms);
    
    unsigned short* terms_result;
    decode(terms_compressed, 1000, terms_result, term_huffman_array, term_terminator_array);
    
    for (int i = 0; i < 1000; ++i)
    {
        if (terms_uncompressed[i] != terms_result[i])
        {
            error("Huffman self test failed!");
        }
    }
    
    show_info("Huffman self test passed!");
}