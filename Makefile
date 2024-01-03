all:
	clang -Wall -Wpedantic -Wextra -Werror -c main.c trie.c word.c io.c
	clang -Wall -Wpedantic -Wextra -Werror -o lzwcoder main.c trie.c word.c io.c -lm
	ln lzwcoder encode
	ln lzwcoder decode
word:
	clang -Wall -Wpedantic -Wextra -Werror -c word.c
	clang -Wall -Wpedantic -Wextra -Werror -o word word.c
trie:
	clang -Wall -Wpedantic -Wextra -Werror -c trie.c
	clang -Wall -Wpedantic -Wextra -Werror -o trie trie.c
io:
	clang -Wall -Wpedantic -Wextra -Werror -c io.c word.c
	clang -Wall -Wpedantic -Wextra -Werror -o io io.c word.c
valgrind: clean all
	valgrind --leak-check=full --show-leak-kinds=all ./lzwencoder -d -i bible.lzw -o done.txt
infer:
	infer-capture -- make; infer-analyze -- make
clean:
	rm -rf word.o
	rm -rf trie.o
	rm -rf io.o
	rm -rf lzwcoder
	rm -rf main.o
	rm -rf infer-out
	rm -rf encode
	rm -rf decode
