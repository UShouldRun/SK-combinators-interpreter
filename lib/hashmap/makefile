CFLAGS = -Wall -Werror -Wpedantic
INCLUDE = -I./include/

release: libhashmap_release
	@echo "HashMap Library compiled successfully in release mode"
debug: libhashmap_debug
	@echo "HashMap Library compiled successfully in debug mode"

libhashmap_release: hashmap_out_release
	@echo "Producing hashmap library in release mode"
	@ar rcs build/libhashmap.a build/hashmap.o

libhashmap_debug: hashmap_out_debug
	@echo "Producing logger library in debug mode"
	@ar rcs build/libhashmap.a build/hashmap.o

hashmap_out_release: src/hashmap.c
	@echo "Compiling into the output file"
	@gcc -c src/hashmap.c $(CFLAGS) -o build/hashmap.o $(INCLUDE)

hashmap_out_debug: src/hashmap.c
	@echo "Compiling into the output file"
	@gcc -c src/hashmap.c $(CFLAGS) -g -o build/hashmap.o $(INCLUDE)

clean:
	@rm build/hashmap.o build/libhashmap.a
	@echo "Cleaned build directory"
