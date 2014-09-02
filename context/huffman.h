#define word_t unsigned short

class Node;

Node* generate_tree(word_t* input, long length);
long encode(word_t* input, long length, char*& output);
void decode(char* input, long length, word_t*& output, Node* tree);
void generate_inverse_mapping(Node* tree);

struct inverse_map_builder_state {
    Node* node;
    char* bits;
    int count_bits;

    inverse_map_builder_state(Node* _node, char* _bits, int _count_bits) :
        node(_node), bits(_bits), count_bits(_count_bits) {}

    inverse_map_builder_state() {}
};

