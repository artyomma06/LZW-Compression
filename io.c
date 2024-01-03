#include "io.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//I would like to credit most of the logical design inside of this function to
//Eugene chow as well as Max L. as they both gave us alot of help with several
//fucntions such as making the buffers as well as read header and get header
static uint8_t bit_buffer[BLOCK_SIZE];
static uint8_t char_buffer[BLOCK_SIZE];
static uint32_t bit_index = 0;
static uint32_t char_index = 0;

extern uint64_t compressed_len;
extern uint64_t long_word;

void read_header(int infile, FileHeader *header) {
  read(infile, header, sizeof(FileHeader));
  if (header->magic != MAGIC) {
    printf("invalid magic\n");
    exit(-1);
  }
  return;
}
void write_header(int outfile, FileHeader *header) {
  write(outfile, header, sizeof(FileHeader));
  return;
}

uint8_t next_char(int infile) {
  if (char_index == 0 || char_index == BLOCK_SIZE) {
    read(infile, &char_buffer, BLOCK_SIZE);
    char_index = 0;
  }
  char_index += 1;
  return (char_buffer[char_index - 1]);
}

void buffer_code(int outfile, uint16_t code, uint8_t bit_len) {
  for (uint32_t i = 0; i < bit_len; i++) {
    if ((bit_index) == (BLOCK_SIZE * 8)) {
      flush_code(outfile);
    }
    if ((code & (0x1 << (i % 16)))) { // is this how id check the bit
      bit_buffer[bit_index / 8] |= (0x1 << (bit_index % 8));
    } else {
      bit_buffer[bit_index / 8] &= ~(0x1 << (bit_index % 8));
    }
    bit_index += 1;
  }
  return;
}

void flush_code(int outfile) {
  uint32_t byte_length = 0;
  if (bit_index % 8 != 0) {
    byte_length = (bit_index / 8) + 1;
  } else {
    byte_length = (bit_index / 8);
  }
  if (bit_index != 0) {
    write(outfile, &bit_buffer, byte_length);
    bit_index = 0;
  }
  compressed_len = compressed_len + byte_length;
  return;
}

uint16_t next_code(int infile, uint8_t bit_len) {
  uint16_t sum = 0;
  uint16_t binary = 1;
  compressed_len = compressed_len + bit_len;
  for (uint8_t i = 0; i < bit_len; i++) {
    if ((bit_index) == 0) {
      read(infile, &bit_buffer, BLOCK_SIZE);
    }

    if (((bit_buffer[bit_index / 8] & (0x1 << (bit_index % 8)))
            >> (bit_index % 8))
        == 1) {
      sum = sum + binary;
    }
    binary = binary * 2;
    bit_index += 1;
    if ((bit_index) == (8 * BLOCK_SIZE)) {
      bit_index = 0;
    }
  }
  return sum;
}

void buffer_word(int outfile, Word *w) {
  for (uint64_t i = 0; i < w->word_len; i++) {
    if (char_index == BLOCK_SIZE) {
      flush_word(outfile);
    }
    char_buffer[char_index] = w->word[i];
    char_index += 1;
  }
  if (long_word < w->word_len) {
    long_word = w->word_len;
  }
  return;
}

void flush_word(int outfile) {
  if (char_index != 0) {
    write(outfile, &char_buffer, char_index);
    char_index = 0;
  }
  return;
}
