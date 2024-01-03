#include "trie.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

TrieNode *trie_node_create(uint16_t code) {
  TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
  assert(node);
  node->code = code;
  return node;
}

void trie_node_delete(TrieNode *n) {
  free(n);
  return;
}

TrieNode *trie_create(void) {
  TrieNode *n = trie_node_create(1);
  for (uint16_t i = 0; i < 256; i++) {
    n->children[i] = trie_node_create(i); //ask about this
  }
  return n;
}

void trie_reset(TrieNode *root) {
  for (uint16_t i = 0; i < 256; i++) {
    trie_delete(root->children[i]);
    root->children[i] = trie_node_create(i);
  }
  return;
}

//using recurstion you can snuf the tree from left most branch to right
void trie_delete(TrieNode *n) {
  for (uint32_t i = 0; i < 256; i++) {
    if (n->children[i]) {
      trie_delete(n->children[i]);
    }
  }
  trie_node_delete(n);
  return;
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
  if (n->children[sym]) {
    return n->children[sym];
  } else {
    return NULL;
  }
}
