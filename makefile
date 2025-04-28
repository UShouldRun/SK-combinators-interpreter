# Compiler and tools
CC := gcc
LEX := flex
YACC := bison

# Default build type
BUILD_TYPE ?= release

# Flags based on build type
ifeq ($(BUILD_TYPE),release)
    CFLAGS := -Wall -Wextra -O2 -std=c99 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE
else ifeq ($(BUILD_TYPE),debug)
    CFLAGS := -Wall -Wextra -std=c99 -g -O0 -DDEBUG
else
    $(error Invalid BUILD_TYPE: $(BUILD_TYPE). Use 'release' or 'debug')
endif

# Common flags - removed -Werror to allow compilation to continue despite warnings
CFLAGS += -fno-strict-aliasing -Wformat -Wformat-security -D_GNU_SOURCE
LDFLAGS := -Wl,-z,relro,-z,now
YFLAGS := -d -v -Wcounterexamples -locations

MAKEFLAGS += --no-print-directory

# Directories
SRC_DIR := src
LIB_DIR := lib
BUILD_DIR := build
ROOT_BIN_DIR := bin
DOCS_DIR := docs

# Create list of all include directories
INCLUDE_DIRS := $(shell find $(LIB_DIR) -type d -name "include")
INCLUDES := $(foreach dir,$(INCLUDE_DIRS),-I$(dir))

# Library components
ARENA_DIR := $(LIB_DIR)/arena
AST_DIR := $(LIB_DIR)/ast
HASHMAP_DIR := $(LIB_DIR)/hashmap
INTERPRETER_DIR := $(LIB_DIR)/interpreter
LEXER_DIR := $(LIB_DIR)/lexer
PARSER_DIR := $(LIB_DIR)/parser

# Build directories for each library
ARENA_BUILD_DIR := $(ARENA_DIR)/build
AST_BUILD_DIR := $(AST_DIR)/build
INTERPRETER_BUILD_DIR := $(INTERPRETER_DIR)/build
LEXER_BUILD_DIR := $(LEXER_DIR)/build
PARSER_BUILD_DIR := $(PARSER_DIR)/build

# Source files
ARENA_SRC := $(ARENA_DIR)/src/arena.c
AST_SRC := $(AST_DIR)/src/ast.c $(AST_DIR)/src/hashtable.c
INTERPRETER_SRC := $(INTERPRETER_DIR)/src/interpreter.c
MAIN_SRC := $(SRC_DIR)/main.c

# Generated source files
LEXER_SRC := $(LEXER_BUILD_DIR)/lex.yy.c
PARSER_SRC := $(PARSER_BUILD_DIR)/parser.tab.c

# Object files
ARENA_OBJ := $(patsubst $(ARENA_DIR)/src/%.c,$(ARENA_BUILD_DIR)/%.o,$(ARENA_SRC))
AST_OBJ := $(patsubst $(AST_DIR)/src/%.c,$(AST_BUILD_DIR)/%.o,$(AST_SRC))
INTERPRETER_OBJ := $(patsubst $(INTERPRETER_DIR)/src/%.c,$(INTERPRETER_BUILD_DIR)/%.o,$(INTERPRETER_SRC))
LEXER_OBJ := $(LEXER_BUILD_DIR)/lex.yy.o
PARSER_OBJ := $(PARSER_BUILD_DIR)/parser.tab.o
MAIN_OBJ := $(BUILD_DIR)/main.o

# All object files
OBJS := $(ARENA_OBJ) $(AST_OBJ) $(INTERPRETER_OBJ) $(LEXER_OBJ) $(PARSER_OBJ) $(MAIN_OBJ)

# Library files
ARENA_LIB := $(ARENA_BUILD_DIR)/libarena.a
AST_LIB := $(AST_BUILD_DIR)/libast.a
INTERPRETER_LIB := $(INTERPRETER_BUILD_DIR)/libinterpreter.a
LEXER_LIB := $(LEXER_BUILD_DIR)/liblexer.a
PARSER_LIB := $(PARSER_BUILD_DIR)/libparser.a

# External libraries
HASHMAP_LIB := $(HASHMAP_DIR)/build/libhashmap.a

# Target executable based on command
ifeq ($(MAKECMDGOALS),build)
    TARGET := $(BUILD_DIR)/interpreter
else
    TARGET := $(ROOT_BIN_DIR)/interpreter
endif

# Default rule - build with security and performance flags
all: directories $(TARGET)

# Build rule - build with default flags
build: directories $(TARGET)

# Rule to create build directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ROOT_BIN_DIR)
	@mkdir -p $(ARENA_BUILD_DIR)
	@mkdir -p $(AST_BUILD_DIR)
	@mkdir -p $(INTERPRETER_BUILD_DIR)
	@mkdir -p $(LEXER_BUILD_DIR)
	@mkdir -p $(PARSER_BUILD_DIR)

# Rules for building library components
$(ARENA_LIB): $(ARENA_OBJ)
	@mkdir -p $(ARENA_BUILD_DIR)
	@echo "Producing arena library in $(BUILD_TYPE) mode"
	@ar rcs $@ $^
	@echo "Arena Library compiled successfully in $(BUILD_TYPE) mode"

$(AST_LIB): $(AST_OBJ)
	@mkdir -p $(AST_BUILD_DIR)
	@echo "Producing AST library in $(BUILD_TYPE) mode"
	@ar rcs $@ $^
	@echo "AST Library compiled successfully in $(BUILD_TYPE) mode"

$(INTERPRETER_LIB): $(INTERPRETER_OBJ)
	@mkdir -p $(INTERPRETER_BUILD_DIR)
	@echo "Producing interpreter library in $(BUILD_TYPE) mode"
	@ar rcs $@ $^
	@echo "Interpreter Library compiled successfully in $(BUILD_TYPE) mode"

$(LEXER_LIB): $(LEXER_OBJ)
	@mkdir -p $(LEXER_BUILD_DIR)
	@echo "Producing lexer library in $(BUILD_TYPE) mode"
	@ar rcs $@ $^
	@echo "Lexer Library compiled successfully in $(BUILD_TYPE) mode"

$(PARSER_LIB): $(PARSER_OBJ)
	@mkdir -p $(PARSER_BUILD_DIR)
	@echo "Producing parser library in $(BUILD_TYPE) mode"
	@ar rcs $@ $^
	@echo "Parser Library compiled successfully in $(BUILD_TYPE) mode"

# Rules for hashmap library (external)
$(HASHMAP_LIB):
	@echo "Producing hashmap library in $(BUILD_TYPE) mode"
	@$(MAKE) -C $(HASHMAP_DIR)
	@echo "HashMap Library compiled successfully in $(BUILD_TYPE) mode"

# Rules for generating lexer and parser code
$(LEXER_SRC): $(LEXER_DIR)/src/lexer.x $(PARSER_BUILD_DIR)/parser.tab.h
	@mkdir -p $(LEXER_BUILD_DIR)
	@echo "Generating lexer source code from Flex definition"
	@$(LEX) -o $@ $<
	@echo "Lexer source code generated successfully"

$(PARSER_SRC) $(PARSER_BUILD_DIR)/parser.tab.h: $(PARSER_DIR)/src/parser.y
	@mkdir -p $(PARSER_BUILD_DIR)
	@echo "Generating parser source code from Bison grammar"
	@$(YACC) $(YFLAGS) -o parser.tab.c $<
	@mv parser.tab.c $(PARSER_BUILD_DIR)/
	@mv parser.tab.h $(PARSER_BUILD_DIR)/
	@mv parser.output docs
	@echo "Parser source code generated successfully"

# Special rules for arena compilation with needed fixes
$(ARENA_BUILD_DIR)/%.o: $(ARENA_DIR)/src/%.c
	@mkdir -p $(ARENA_BUILD_DIR)
	@echo "Compiling arena component: $(<F)"
	@$(CC) $(CFLAGS) -Wno-sign-compare $(INCLUDES) -c $< -o $@

# Rules for compiling object files (except arena which has special handling)
$(AST_BUILD_DIR)/%.o: $(AST_DIR)/src/%.c
	@mkdir -p $(AST_BUILD_DIR)
	@echo "Compiling AST component: $(<F)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(INTERPRETER_BUILD_DIR)/%.o: $(INTERPRETER_DIR)/src/interpreter.c
	@mkdir -p $(INTERPRETER_BUILD_DIR)
	@echo "Compiling interpreter component: $(<F)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LEXER_BUILD_DIR)/lex.yy.o: $(LEXER_SRC)
	@mkdir -p $(LEXER_BUILD_DIR)
	@echo "Compiling lexer component: $(<F)"
	@$(CC) $(CFLAGS) -I$(PARSER_BUILD_DIR) $(INCLUDES) -c $< -o $@

$(PARSER_BUILD_DIR)/parser.tab.o: $(PARSER_SRC) $(AST_LIB)
	@mkdir -p $(PARSER_BUILD_DIR)
	@echo "Compiling parser component: $(<F)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling main component: $(<F)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Rule for building the final executable
$(TARGET): $(ARENA_LIB) $(AST_LIB) $(INTERPRETER_LIB) $(LEXER_LIB) $(PARSER_LIB) $(HASHMAP_LIB) $(MAIN_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking final executable in $(BUILD_TYPE) mode"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(MAIN_OBJ) -L$(ARENA_BUILD_DIR) -L$(AST_BUILD_DIR) -L$(INTERPRETER_BUILD_DIR) -L$(LEXER_BUILD_DIR) -L$(PARSER_BUILD_DIR) -L$(HASHMAP_DIR)/build -linterpreter -lparser -llexer -lfl -last -larena -lhashmap
	@echo "Build completed successfully"

# Clean rule to remove build artifacts
clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(ROOT_BIN_DIR)
	@rm -rf $(ARENA_BUILD_DIR)
	@rm -rf $(AST_BUILD_DIR)
	@rm -rf $(INTERPRETER_BUILD_DIR)
	@rm -rf $(LEXER_BUILD_DIR)
	@rm -rf $(PARSER_BUILD_DIR)
	@$(MAKE) -C $(HASHMAP_DIR) clean

# Install rule for system-wide installation
install: all
	@echo "Installing interpreter to $(DESTDIR)/usr/local/bin/"
	@install -d $(DESTDIR)/usr/local/bin
	@install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/
	@echo "Installation completed successfully"

# Make dependencies
.PHONY: all build clean install directories
