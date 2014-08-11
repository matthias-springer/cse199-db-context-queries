#include "huffman_benchmark.h"
#include "huffman.h"
#include "pubmed.h"
#include <map>
#include <vector>
#include <queue>
#include <deque>
#include "output.h"

class Node
{
public:
    vector<int>* terms;
    long counter;
};

struct node_comparator
{
    bool operator()(const Node* a, const Node* b) const
    {
        return a->counter > b->counter;
    }
};

void calculate_huffman_encoded_size()
{
    map<int, int> term_bit_counter;
    priority_queue<Node*, deque<Node*>, node_comparator> nodes;
    
    for (int i = 0; i < T_PM; ++i)
    {
        Node* node = new Node();
        vector<int>* terms = new vector<int>();
        terms->push_back(i);
        node->terms = terms;
        node->counter = pubmed::get_group_by_term(i);
        
        nodes.push(node);
    }
    
    while (nodes.size() > 1)
    {
        Node* left = nodes.top();
        nodes.pop();
        Node* right = nodes.top();
        nodes.pop();
        
        Node* result = new Node();
        vector<int>* terms = new vector<int>();
        terms->insert(terms->end(), left->terms->begin(), left->terms->end());
        terms->insert(terms->end(), right->terms->begin(), right->terms->end());
        result->terms = terms;
        result->counter = left->counter + right->counter;
        nodes.push(result);
        
        delete left;
        delete right;
        
        for (auto it = terms->begin(); terms->end() != it; ++it)
        {
            term_bit_counter[*it]++;
        }
    }
    
    show_info(term_bit_counter.size());
    
    map<int, int> bit_len_counter;
    
    // calculate new size
    long compressed_bits = 0;
    long uncompressed_bits = 0;
    
    for (int i = 0; i < T_PM; ++i)
    {
        int freq = pubmed::get_group_by_term(i);
        uncompressed_bits += freq * 16;     // assuming 16 bit shorts
        compressed_bits += freq * term_bit_counter[i];
        
        bit_len_counter[term_bit_counter[i]]++;
    }
    
    for (auto it = bit_len_counter.begin(); bit_len_counter.end() != it; ++it)
    {
        show_info(it->first << " bits: " << it->second << " times.");
    }
    
    show_info("Uncompressed size: " << uncompressed_bits << " bits.");
    show_info("Compressed size: " << compressed_bits << " bits.");
}