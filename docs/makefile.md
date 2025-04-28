# Makefile Documentation

## Overview
This Makefile orchestrates the compilation and linking process for the interpreter project. It manages the building of several independent libraries, the generation of source code for the lexer and parser, and the creation of the final executable program. The Makefile supports different build configurations (release and debug) and provides targets for building, cleaning, and installing the application.

## Build Targets
These are the primary commands you can run with `make`:

- `make` (or `make all`): This is the default target. It compiles the entire project using release settings (optimized and secure) and places the final executable named `interpreter` in the `bin/` directory.
- `make build`: This target explicitly builds the project using the defined build type (default is release) and places the final executable `interpreter` in the `build/` directory. This is useful for keeping the final executable separate from the source and library build artifacts.
- `make clean`: This command removes all generated build directories (`build/` at the root level and the `build/` directories within each library), object files, generated source files (lexer and parser), and the `bin/` directory. It ensures a clean state for a fresh build. It also recursively calls `make clean` in the external `hashmap` library.
- `make install`: This target first builds the entire project using the `all` target and then installs the resulting `interpreter` executable to `/usr/local/bin/` on your system. The `$(DESTDIR)` variable allows for staged installations.

## Build Types
You can control the build configuration by setting the `BUILD_TYPE` variable when running `make`:

- `BUILD_TYPE=release`: This is the default build type. It compiles the code with optimizations (`-O2`) for performance and includes security-enhancing compiler and linker flags.
- `BUILD_TYPE=debug`: This build type compiles the code with debugging symbols (`-g`) and disables optimizations (`-O0`). This makes it easier to debug the application with tools like GDB.

If an invalid `BUILD_TYPE` is specified, `make` will output an error and stop.

## Directory Structure
This section describes the organization of the project directories:

- `src/`: This directory contains the main source code files for the interpreter, including `main.c`.
- `lib/`: This directory houses the source code and header files for the various library components of the project. Each subdirectory within `lib/` represents a distinct library (e.g., `arena`, `ast`, `interpreter`, `lexer`, `parser`, `hashmap`).
    - `lib/<library>/include/`: Contains the public header files for the respective library, which are intended for use by other parts of the project.
    - `lib/<library>/src/`: Contains the implementation (source code files `.c`) for the respective library.
    - `lib/<library>/build/`: This directory is created during the build process to store the object files (`.o`) and static library (`.a`) for that specific library.
- `build/`: This is the central build directory at the root level. It's used to store the final `interpreter` executable when the `make build` target is used and also temporarily holds the `main.o` object file.
- `bin/`: This directory is where the final `interpreter` executable is placed when the default `make` target (or `make all`) is used. This is often intended as the primary location for the user-facing executable.
- `docs/`: This directory contains project documentation, such as this Makefile documentation (`makefile.md`).

## Implementation Details

### Library Structure and Building
The project is structured as a set of independent libraries. Each library (arena, ast, interpreter, lexer, parser) resides in its own subdirectory within `lib/`. The Makefile compiles each library's source files into a separate static library (`.a` file) within its own `lib/<library>/build/` directory. This modular design promotes code reusability and better organization.

### Build Process Breakdown
The `make` and `make build` targets follow these steps:

1.  **Create Build Directories:** The `directories` rule ensures that the necessary build directories (`build/` and the `build/` directories within each library) are created if they don't already exist.
2.  **Generate Lexer and Parser Code:**
    - The `lexer.x` file in `lib/lexer/src/` is processed by `flex` to generate the C source code for the lexer (`lex.yy.c`) in `lib/lexer/build/`. This lexer is responsible for tokenizing the input language.
    - The `parser.y` file in `lib/parser/src/` is processed by `bison` to generate the C source code for the parser (`parser.tab.c`) and its header file (`parser.tab.h`) in `lib/parser/build/`. The parser analyzes the token stream from the lexer and builds the abstract syntax tree (AST).
3.  **Compile Source Files to Object Files:** Each `.c` source file in the project (including the main application code and the library implementations) is compiled into an object file (`.o`). These object files are placed in the corresponding library's `build/` directory (e.g., `lib/arena/build/arena.o`) or the root-level `build/` directory for `main.c`. Special handling exists for `arena.c` to suppress specific warnings.
4.  **Archive Object Files into Component Libraries:** The object files for each library are then archived together using `ar` to create a static library (`.a` file) in the library's `build/` directory (e.g., `lib/arena/build/libarena.a`).
5.  **Link Libraries and Main Object File:** Finally, the object file for `main.c` (`build/main.o`) is linked together with all the generated static libraries (`lib/arena/build/libarena.a`, `lib/ast/build/libast.a`, etc.) and the external `hashmap` library to create the final `interpreter` executable. The `-L` flags specify the directories where the linker should look for the libraries, and the `-l` flags specify the names of the libraries to link against. The executable is placed in either `bin/` (for `make`) or `build/` (for `make build`).

### Security Features in Release Builds
When `BUILD_TYPE` is set to `release` (or by default), the compiler and linker use the following flags to enhance the security of the compiled executable:

- `-D_FORTIFY_SOURCE=2`: This compiler flag enables aggressive checking of buffer overflows at runtime, potentially preventing security vulnerabilities.
- `-fstack-protector-strong`: This flag adds stack canaries to functions, which are used to detect stack buffer overflows (stack smashing attacks) at runtime.
- `-fPIE`: This flag compiles the executable as a Position Independent Executable (PIE). PIEs can be loaded at a random memory address each time they are run, making it harder for attackers to exploit memory-related vulnerabilities.
- `-Wl,-z,relro`: This linker flag enables "Relocation Read-Only" (RELRO), which marks the relocation sections of the executable as read-only after linking, preventing attackers from overwriting them.
- `-Wl,-z,now`: This linker flag enables "lazy binding" to be disabled, forcing all dynamic symbol lookups to be resolved at program startup. This can improve security by preventing attackers from injecting malicious code through symbol resolution.

### Automatic Header Inclusion
The Makefile automatically finds all directories named "include" within the `lib/` directory structure using the `find` command. It then creates a list of `-I` flags, ensuring that the compiler can find the header files for all the libraries without requiring manual specification of include paths. This simplifies the build process and makes it more robust to changes in the directory structure.

### Key Improvements Summarized
- **Organized Build Output**: Each library's build artifacts are now contained within its own `build/` subdirectory, leading to a cleaner project structure.
- **Robust Security Measures**: Release builds benefit from a comprehensive set of compiler and linker flags designed to mitigate various security risks.
- **Optimized Performance**: Release builds are compiled with `-O2` optimization for better runtime performance.
- **Debugging Convenience**: Debug builds retain necessary debugging symbols and disable optimizations for easier debugging.
- **Automated Documentation**: While not explicitly a build target in this snippet, the comment hints at a potential `make docs` target for generating documentation.
- **Flexible Executable Placement**: The Makefile provides flexibility in where the final executable is placed based on the chosen build target.
- **Thorough Cleanup**: The `make clean` command effectively removes all generated files and directories, ensuring a pristine build environment.
