#ifndef RBT_H
#define RBT_H
#include <utility>
struct Node
{
    bool red;
    unsigned long long data;
    Node* parent;
    Node* major;
    Node* minor;
};

Node* rotateLeft(Node* n, Node** head);
Node* rotateRight(Node* n, Node** head);
Node* nodeAlloc(unsigned long long value);
void nodeFree(Node* n);
Node* nodeFind(Node* head, unsigned long long value);
void nodeAdd_RBT(Node* node, Node** head, unsigned long long value);
bool nodeDelete_RBT(Node** head, unsigned long long value);
std::pair<Node*, Node*> nodeDelete_BST(Node** head, unsigned long long value, Node** replacement);
void nodePrint(Node* n);

#endif