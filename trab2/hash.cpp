#include "hash.h"
#include <iostream>
#include <cstdlib>

HashTable::HashTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i] = nullptr;
    }
}

HashTable::~HashTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            HashNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
}

int HashTable::hashFunction(int category) const {
    return category % TABLE_SIZE;
}

void HashTable::insert(int category, int id, int fileIndex) {
    int index = hashFunction(category);

    HashNode* newHashNode = new HashNode;
    newHashNode->fileIndex = fileIndex;
    newHashNode->categoria = category;
    newHashNode->id = id;
    newHashNode->next = table[index];
    table[index] = newHashNode;
}

HashNode* HashTable::search(int category) const {
    int index = hashFunction(category);
    HashNode* current = table[index];

    while (current != nullptr) {
        if (current->categoria == category) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void HashTable::remove(int category, int id) {
    int index = hashFunction(category);
    HashNode* current = table[index];
    HashNode* prev = nullptr;

    while (current != nullptr) {
        if (current->id == id) {
            if (prev == nullptr) {
                table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void HashTable::printTable() const {
    for (int i = 0; i < TABLE_SIZE; i++) {
        std::cout << "Índice " << i << ": ";
        HashNode* current = table[i];
        while (current != nullptr) {
            std::cout << " -> [FileIndex: " << current->fileIndex << "]";
            current = current->next;
        }
        std::cout << " -> NULL" << std::endl;
    }
}
