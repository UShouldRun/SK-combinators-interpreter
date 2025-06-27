# SK Combinators Interpreter

A minimalist interpreter for SK combinator calculus.

## Overview

This interpreter implements the SK combinator calculus, a variant of combinatory logic that uses only two primitive combinators - S and K - to express all computable functions. It serves as both a theoretical model of computation and a practical tool for exploring functional programming concepts.

## Features

- Parsing and interpreting of SK expressions
- Step-by-step reduction visualization (needs to be modified in the beta reduction function to be seen)
- Support for custom combinators and extensions

## Installation

```
make
make install
```

## Usage

```
interpreter [file]
```

### Example

```
interpreter example.ld
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
