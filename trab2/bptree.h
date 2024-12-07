#ifndef BPTREE_H
#define BPTREE_H

#include <vector>
#include <iostream>

struct Node {
    std::vector<int> keys;           // Chaves no nó
    std::vector<Node*> children;     // Ponteiros para filhos (apenas para nós internos)
    std::vector<long> addresses;     // Endereços (apenas para folhas)
    bool isLeaf;                     // Indica se é um nó folha
    Node(bool leaf) : isLeaf(leaf) {}
};

class BPlusTree {
private:
    Node* root;       // Raiz da árvore
    int order;        // Ordem da árvore

    void insertInternal(int key, Node* cursor, Node* child, long address = -1);
    void splitInternal(Node* cursor);
    Node* findParent(Node* cursor, Node* child);
    void insertLeaf(int key, long address, Node* leaf);
    void splitLeaf(Node* leaf);

    void adjustAfterDelete(Node* cursor, Node* parent);

public:
    BPlusTree(int order);
    ~BPlusTree();
    Node* getRoot() const;
    void insert(int key, long address);
    void remove(int key);
    void print(Node* cursor = nullptr, int level = 0) const;
};

#endif // BPTREE_H
