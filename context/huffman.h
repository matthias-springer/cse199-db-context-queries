#define word_t unsigned short

class Node;

Node* generate_tree(word_t* input, long length);
long encode(word_t* input, long length, char*& output, Node* tree);
void decode(char* input, long length, word_t*& output, Node* tree);

