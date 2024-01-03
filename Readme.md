# LZW Compression

## Overview

Welcome to the *LZW Compression* project! In this program, we focus on compressing and decompressing files using the LZW lossless compression algorithm. The main functionality revolves around running the program with flags to indicate whether compression or decompression is desired, along with specifying input and output files.

## Implementation

The LZW compression algorithm utilizes a Trie data structure for efficient encoding, while decompression reconstructs the original data. The process involves dynamically updating structures and handling edge cases, ensuring optimal compression and decompression.

## Usage

To run the program, use the following options:

- Use `./lzwencoder` followed by the `-c` or `-d` flag to specify compression or decompression.
- Include the `-i` flag followed by the input file name and the `-o` flag followed by the output file name.
- If no input or output names are provided, the program defaults to standard input.
- `-v` Enables verbose mode which will show additional metrics regarding compressed file.

You can also use `encode` or `decode` as alternatives to the `-c` or `-d` flags, respectively. Most flags are compatible with both modes, except for the `-c` or `-d` flags, which are mandatory.