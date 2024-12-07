#ifndef HASH_H
#define HASH_H

#include <iostream>

struct HashNode {
    int fileIndex;
    int categoria;
    int id;
    bool memoria;
    HashNode* next = nullptr;
};

class HashTable {
private:
    static const int TABLE_SIZE = 101;
    HashNode* table[TABLE_SIZE];

    int hashFunction(int category) const;

public:
    HashTable();
    ~HashTable();

    void insert(int category, int id, int fileIndex);
    HashNode* search(int category) const;
    void remove(int category, int id);

    void printTable() const;
};

#endif
