#include "bptree.h"
#include <algorithm>
#include <iostream>

BPlusTree::BPlusTree(int order) : root(new Node(true)), order(order) {}

BPlusTree::~BPlusTree() {
}

Node* BPlusTree::getRoot() const {
    return root;
}

void BPlusTree::insert(int key, long address) {
    Node* cursor = root;
    while (!cursor->isLeaf) {
        auto it = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key);
        int pos = it - cursor->keys.begin();
        cursor = cursor->children[pos];
    }
    insertLeaf(key, address, cursor);
}

void BPlusTree::insertLeaf(int key, long address, Node* leaf) {
    auto it = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int pos = it - leaf->keys.begin();

    leaf->keys.insert(leaf->keys.begin() + pos, key);
    leaf->addresses.insert(leaf->addresses.begin() + pos, address);

    if (leaf->keys.size() >= order) {
        splitLeaf(leaf);
    }
}

void BPlusTree::splitLeaf(Node* leaf) {
    int midIndex = leaf->keys.size() / 2;

    Node* newLeaf = new Node(true);
    newLeaf->keys.assign(leaf->keys.begin() + midIndex, leaf->keys.end());
    newLeaf->addresses.assign(leaf->addresses.begin() + midIndex, leaf->addresses.end());

    leaf->keys.resize(midIndex);
    leaf->addresses.resize(midIndex);

    newLeaf->children.push_back(leaf->children.empty() ? nullptr : leaf->children[0]);
    leaf->children.clear();
    leaf->children.push_back(newLeaf);

    if (leaf == root) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(newLeaf->keys[0]);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
    } else {
        insertInternal(newLeaf->keys[0], findParent(root, leaf), newLeaf);
    }
}

void BPlusTree::insertInternal(int key, Node* cursor, Node* child, long address) {
    auto it = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key);
    int pos = it - cursor->keys.begin();

    cursor->keys.insert(cursor->keys.begin() + pos, key);
    cursor->children.insert(cursor->children.begin() + pos + 1, child);

    if (cursor->keys.size() >= order) {
        splitInternal(cursor);
    }
}

void BPlusTree::splitInternal(Node* cursor) {
    int midIndex = cursor->keys.size() / 2;
    int midKey = cursor->keys[midIndex];

    Node* newNode = new Node(false);
    newNode->keys.assign(cursor->keys.begin() + midIndex + 1, cursor->keys.end());
    newNode->children.assign(cursor->children.begin() + midIndex + 1, cursor->children.end());

    cursor->keys.resize(midIndex);
    cursor->children.resize(midIndex + 1);

    if (cursor == root) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(midKey);
        newRoot->children.push_back(cursor);
        newRoot->children.push_back(newNode);
        root = newRoot;
    } else {
        insertInternal(midKey, findParent(root, cursor), newNode);
    }
}

Node* BPlusTree::findParent(Node* cursor, Node* child) {
    if (!cursor->isLeaf) {
        for (auto c : cursor->children) {
            if (c == child) return cursor;
            Node* parent = findParent(c, child);
            if (parent) return parent;
        }
    }
    return nullptr;
}

void BPlusTree::print(Node* cursor, int level) const {
    if (!cursor) cursor = root;
    std::cout << std::string(level * 2, '-') << "Keys: ";
    for (int key : cursor->keys) std::cout << key << " ";
    std::cout << "\n";
    if (!cursor->isLeaf) {
        for (Node* child : cursor->children) {
            print(child, level + 1);
        }
    }
}

void BPlusTree::remove(int key) {
    Node* cursor = root;
    Node* parent = nullptr;

    while (!cursor->isLeaf) {
        auto it = std::lower_bound(cursor->keys.begin(), cursor->keys.end(), key);
        int pos = it - cursor->keys.begin();
        parent = cursor;
        cursor = cursor->children[pos];
    }

    auto it = std::find(cursor->keys.begin(), cursor->keys.end(), key);
    if (it == cursor->keys.end()) {
        std::cout << "Chave não encontrada\n";
        return;
    }
    int pos = it - cursor->keys.begin();
    cursor->keys.erase(it);
    cursor->addresses.erase(cursor->addresses.begin() + pos);

    if (cursor->keys.size() < (order - 1) / 2 && cursor != root) {
        adjustAfterDelete(cursor, parent);
    }

    if (cursor == root && cursor->keys.empty()) {
        if (!cursor->isLeaf) {
            root = cursor->children[0];
        } else {
            root = nullptr;
        }
        delete cursor;
    }
}

void BPlusTree::adjustAfterDelete(Node* cursor, Node* parent) {
    int idx = 0;
    while (idx < parent->children.size() && parent->children[idx] != cursor) {
        idx++;
    }

    Node* leftSibling = (idx > 0) ? parent->children[idx - 1] : nullptr;
    Node* rightSibling = (idx < parent->children.size() - 1) ? parent->children[idx + 1] : nullptr;

    if (leftSibling && leftSibling->keys.size() > (order - 1) / 2) {
        cursor->keys.insert(cursor->keys.begin(), parent->keys[idx - 1]);
        parent->keys[idx - 1] = leftSibling->keys.back();
        leftSibling->keys.pop_back();
        return;
    }

    if (rightSibling && rightSibling->keys.size() > (order - 1) / 2) {
        cursor->keys.push_back(parent->keys[idx]);
        parent->keys[idx] = rightSibling->keys.front();
        rightSibling->keys.erase(rightSibling->keys.begin());
        return;
    }

    if (leftSibling) {
        leftSibling->keys.push_back(parent->keys[idx - 1]);
        leftSibling->keys.insert(leftSibling->keys.end(), cursor->keys.begin(), cursor->keys.end());
        parent->keys.erase(parent->keys.begin() + idx - 1);
        parent->children.erase(parent->children.begin() + idx);
        delete cursor;
    } else if (rightSibling) {
        cursor->keys.push_back(parent->keys[idx]);
        cursor->keys.insert(cursor->keys.end(), rightSibling->keys.begin(), rightSibling->keys.end());
        parent->keys.erase(parent->keys.begin() + idx);
        parent->children.erase(parent->children.begin() + idx + 1);
        delete rightSibling;
    }

    if (parent->keys.size() < (order - 1) / 2) {
        Node* grandparent = findParent(root, parent);
        adjustAfterDelete(parent, grandparent);
    }
}
