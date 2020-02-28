#include <stdexcept>

#include "Trie.hh"

Trie::Branch::Branch () { children.fill (nullptr); }

Trie::Branch *Trie::make_branch (string key) {
    Branch *next = root;
    for (char c : key) {
        char kc;
        if (c == '_') {
            kc = 26;
        } else {
            kc = c - 'a';
        }
        if (kc < 0 || 26 < kc)
            throw logic_error ("key must consists of a-z or _");
        if (next->children[kc] == nullptr) {
            next->children[kc] = new Branch;
        }
        next = next->children[kc];
    }

    return next;
}

Trie::Trie () : root (new Branch) {}

Trie::~Trie () { delete_branch (root); }

void Trie::delete_branch (Branch *branch) {
    for (Branch *b : branch->children) {
        if (b != nullptr) {
            delete_branch (b);
        }
    }
    delete branch;
}

array<unsigned char, 4> &Trie::operator[] (string key) {
    Branch *item = make_branch (key);
    return item->item;
}

array<unsigned char, 4> *Trie::find (string key) {
    Branch *next = root;
    for (char c : key) {
        char kc;
        if (c == '_') {
            kc = 26;
        } else {
            kc = c - 'a';
        }
        if (kc < 0 || 26 < kc)
            throw logic_error ("key must consists of a-z or _");
        next = next->children[kc];
        if (next == nullptr) {
            return nullptr;
        }
    }

    return &next->item;
}
