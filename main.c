#include "io.h"
#include "trie.h"
#include "word.h"
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Global variables for tracking compressed length and longest word
uint64_t compressed_len = 0;
uint64_t long_word = 0;

// Main function
int main(int argc, char **argv) {
  // Flags and variables initialization
  bool verbose = false;
  bool compression = false;
  bool decompression = false;
  char *inputfile = NULL;
  char *outputfile = NULL;
  int c = 0;
  int input;
  int output;

  // Check command-line arguments to determine compression or decompression
  if (strcmp(argv[0], "encode") == 0 || strcmp(argv[0], "./encode") == 0) {
    compression = true;
  }
  if (strcmp(argv[0], "decode") == 0 || strcmp(argv[0], "./decode") == 0) {
    decompression = true;
  }

  // Process command-line options using getopt
  while ((c = getopt(argc, argv, "-vcdi:o:")) != -1) {
    switch (c) {
      case 'v':
        verbose = true;
        break;
      case 'c':
        compression = true;
        break;
      case 'd':
        decompression = true;
        break;
      case 'i':
        inputfile = strdup(optarg);
        break;
      case 'o':
        outputfile = strdup(optarg);
        break;
    }
  }

  // Open input file or set to standard input if not provided or invalid
  if (inputfile) {
    input = open(inputfile, O_RDONLY);
  }
  if (!inputfile || input < 0) {
    input = 0;
  }
  if (inputfile) {
    free(inputfile);
  }

  // Open output file or set to standard output if not provided or invalid
  if (outputfile) {
    output = open(outputfile, O_CREAT | O_RDWR | O_TRUNC, 0600);
  }
  if (!outputfile || output < 0) {
    output = 1;
  }
  if (outputfile) {
    free(outputfile);
  }

  // FileHeader structure for storing file information
  FileHeader header;

  // Compression logic
  if (compression && !decompression) {
    // File statistics for input file
    struct stat stats;
    fstat(input, &stats);
    header.file_size = stats.st_size;
    header.protection = stats.st_mode;
    header.magic = MAGIC;
    write_header(output, &header);

    // Trie setup for compression
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    uint16_t next_avail_code = 256;
    uint64_t encoded_chars = 0;
    uint8_t curr_char;
    uint8_t bit_len;

    // Main compression loop
    while (encoded_chars != header.file_size) {
      curr_char = next_char(input);
      TrieNode *next_node = trie_step(curr_node, curr_char);

      if (encoded_chars == 0 || next_node) {
        curr_node = next_node;
      } else {
        bit_len = (uint8_t)log2((double)next_avail_code) + 1;
        buffer_code(output, curr_node->code, bit_len);
        curr_node->children[curr_char] = trie_node_create(next_avail_code);
        curr_node = root->children[curr_char];
        next_avail_code += 1;
      }

      encoded_chars += 1;

      if (next_avail_code == UINT16_MAX) {
        trie_reset(root);
        curr_node = root->children[curr_char];
        next_avail_code = 256;
      }
    }

    bit_len = (uint8_t)log2((double)next_avail_code) + 1;
    buffer_code(output, curr_node->code, bit_len);
    flush_code(output);
    trie_delete(root);
  }

  // Decompression logic
  if (decompression && !compression) {
    // Read header information for decompression
    read_header(input, &header);
    WordTable *table = wt_create();
    uint16_t next_avail_code = 256;
    bool reset = false;
    uint64_t decoded_chars = 0;
    uint8_t bit_len;
    uint16_t curr_code = 0;
    uint64_t prev_size = 4096;
    uint64_t curr_size = 4096;
    uint64_t new_size = 4096;

    uint8_t *prev_word = (uint8_t *)calloc(prev_size, sizeof(uint8_t));
    uint8_t *curr_word = (uint8_t *)calloc(curr_size, sizeof(uint8_t));
    uint8_t *new_word = (uint8_t *)calloc(new_size, sizeof(uint8_t));
    Word *curr_entry = NULL;
    Word *prev_entry = NULL;
    Word *missing_entry;

    uint16_t prev_code = 0;

    // Main decompression loop
    while (decoded_chars != header.file_size) {
      bit_len = (uint8_t)log2((double)(next_avail_code + 1)) + 1;
      curr_code = next_code(input, bit_len);
      curr_entry = table->entries[curr_code];

      if (decoded_chars == 0 || reset) {
        buffer_word(output, curr_entry);
        prev_word = curr_entry->word;
        reset = false;
        decoded_chars += 1;
      } else if (curr_entry != NULL) {
        curr_word = curr_entry->word;
        prev_entry = table->entries[prev_code];
        prev_word = prev_entry->word;

        if ((prev_entry->word_len + 1) > new_size) {
          new_size = new_size * 2;
          new_word = (uint8_t *)realloc(new_word, new_size);
        }

        for (uint64_t i = 0; i < prev_entry->word_len; i++) {
          new_word[i] = prev_word[i];
        }

        new_word[prev_entry->word_len] = curr_word[0];

        table->entries[next_avail_code] =
            word_create(new_word, (prev_entry->word_len + 1));
        next_avail_code += 1;
        buffer_word(output, curr_entry);
        decoded_chars += curr_entry->word_len;
      } else {
        prev_entry = table->entries[prev_code];
        prev_word = prev_entry->word;

        if ((prev_entry->word_len + 1) > curr_size) {
          curr_size = curr_size * 2;
          curr_word = (uint8_t *)realloc(curr_word, curr_size);
        }

        for (uint64_t i = 0; i < prev_entry->word_len; i++) {
          curr_word[i] = prev_word[i];
        }

        curr_word[prev_entry->word_len] = prev_word[0];

        missing_entry =
            word_create(curr_word, prev_entry->word_len + 1);
        table->entries[next_avail_code] = missing_entry;
        next_avail_code += 1;
        buffer_word(output, missing_entry);
        decoded_chars += missing_entry->word_len;
      }

      prev_code = curr_code;

      if (next_avail_code == UINT16_MAX - 1) {
        wt_reset(table);
        next_avail_code = 256;
        reset = true;
      }
    }

    // Flush remaining data
    flush_word(output);

    // Free allocated memory
    free(prev_word);
    free(curr_word);
    free(new_word);
    wt_delete(table);
  }

  // Check for conflicting options
  if ((compression && decompression) || (!compression && !decompression)) {
    printf("You must choose compression or decompression, not both or neither.\n");
  }

  // Display verbose information
  if (verbose) {
    printf("\nOriginal file size: %" PRIu64 "\n", header.file_size);
    printf("Compressed file size: %" PRIu64 "\n", compressed_len);
    printf("Compression ratio: %2.4f%% \n",
           (100 * (1 - ((double)compressed_len / (double)header.file_size))));
    printf("Longest word length: %" PRIu64 "\n", long_word);
  }

  // Close file descriptors if they were opened
  if (input == 0) {
    close(input);
  }
  if (output == 1) {
    close(output);
  }

  return 0;
}
