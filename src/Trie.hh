#ifndef TRIE_HH
#define TRIE_HH

#include <array>
#include <string>
#include <utility>

using namespace std;

class Trie {
    struct Branch {
        array<Branch *, 27> children; /* for `a' to `z' and `_' */
        array<unsigned char, 4> item;

        Branch ();
    };

    /* virtual leaf of trie with no character asssciated with. */
    Branch *root;

    /* make branch recursively to last char of KEY can be inserted nad returns
       the last branch. */
    Branch *make_branch (string key);
    void delete_branch (Branch *branch);

public:
    Trie ();
    ~Trie ();

    /* returns reference to item, for read and write access.
       if no item associated with the KEY found, new item will be created. */
    array<unsigned char, 4> &operator[] (string key);
    /* returns pointer to item associated with KEY. if no item associated,
       nullptr will be returned. */
    array<unsigned char, 4> *find (string key);
};

#endif
