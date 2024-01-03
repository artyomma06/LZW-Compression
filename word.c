#include "word.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Word *word_create(uint8_t *word, uint64_t word_len) {
  Word *w = (Word *)malloc(sizeof(Word)); //
  assert(w);
  w->word_len = word_len;
  w->word = (uint8_t *)malloc(word_len * sizeof(uint8_t));
  assert(w->word);
  for (uint64_t i = 0; i < word_len; i++) {
    w->word[i] = word[i];
  }
  return w;
}

void word_delete(Word *w) {
  free(w);
  w = NULL;
  return;
}

WordTable *wt_create(void) {
  WordTable *wt = (WordTable *)malloc(sizeof(WordTable));
  memset(wt->entries, 0x00, UINT16_MAX);
  for (uint16_t i = 0; i <= 255; i++) {
    uint8_t temp = (uint8_t)i;
    wt->entries[temp] = word_create(&temp, 1);
  }
  return wt;
}

void wt_reset(WordTable *wt) {
  for (uint32_t i = 256; i < UINT16_MAX; i++) {
    wt->entries[i] = NULL;
  }
  return;
}

void wt_delete(WordTable *wt) {
  for (uint32_t i = 0; i < UINT16_MAX; i++) {
    if (wt->entries[i] != 0) {
      word_delete(wt->entries[i]);
    }
  }
  free(wt);
  wt = NULL;
  return;
}

void word_print(Word *w) {
  for (uint64_t i = 0; i < w->word_len; i++) {
    printf("%c", w->word[i]);
  }
  return;
}

void wt_print(WordTable *wt) {
  for (uint32_t i = 0; i < UINT16_MAX; i++) {
    if (wt->entries[i] != 0) {
      word_print(wt->entries[i]);
    }
  }
  return;
}
