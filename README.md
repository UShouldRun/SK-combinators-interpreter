# SK Combinators Interpreter

A minimalist interpreter for SK combinator calculus.

## Overview

This interpreter implements the SK combinator calculus, a variant of combinatory logic that uses only two primitive combinators - S and K - to express all computable functions. It serves as both a theoretical model of computation and a practical tool for exploring functional programming concepts.

## Features

- Parsing and interpreting of SK expressions
- Step-by-step reduction visualization
- Support for custom combinators and extensions
- Command line interface for interactive evaluation
- File-based execution for batch processing
- Debug mode for tracing expression evaluation

## Installation

```
make
make install
```

## Usage

```
interpreter [options] [file]
```

### Options

- `-v, --verbose`: Show detailed reduction steps
- `-h, --help`: Display help information
- `-d, --debug`: Run in debug mode
- `-i, --interactive`: Start interactive REPL mode

### Examples

```
# Evaluate a file
interpreter example.ld

# Interactive mode
interpreter -i

# Show reduction steps
interpreter -v example.ld
```

## Syntax

The interpreter accepts expressions in the following format:

```
S       # S combinator
K       # K combinator
(S K)   # Application
((S K) K)  # Nested application
```

## Author

Henrique Teixeira

## License

MIT License
