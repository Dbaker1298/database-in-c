# GitHub Copilot Instructions for database-in-c

This repository contains a lightweight database implementation in C. Follow these guidelines when working with the codebase.

## Code Style

### General C Conventions
- Use 2 spaces for indentation (no tabs)
- Place opening braces on the same line for functions and control structures
- Use descriptive variable names (e.g., `dbfd` for database file descriptor, `dbhdr` for database header)
- Include blank lines between function definitions for readability

### Naming Conventions
- Use lowercase with underscores for functions: `create_db_file()`, `validate_db_header()`
- Use lowercase with underscores for variable names: `newfile`, `filepath`
- Use `_t` suffix for struct tags/types: `struct dbheader_t`, ``
- Use ALL_CAPS for preprocessor macros: `STATUS_ERROR`, `STATUS_SUCCESS`

### Header Files
- Always use include guards in the format: `#ifndef FILENAME_H` / `#define FILENAME_H` / `#endif`
- Organize includes in this order:
  1. Standard library headers (`<stdio.h>`, `<stdlib.h>`, etc.)
  2. System headers (`<sys/types.h>`, `<fcntl.h>`, etc.)
  3. Project headers (`"common.h"`, `"file.h"`, etc.)

### Error Handling
- Use `STATUS_ERROR` (-1) for error conditions and `STATUS_SUCCESS` (0) for success
- Return error codes from functions and check them in calling code
- Use `perror()` for system call failures to provide context
- Use `printf()` for application-level error messages
- Implement cleanup with `goto cleanup` pattern for resource management
- Always close file descriptors and free allocated memory in cleanup sections

## Build System

### Makefile
- The project uses a Makefile for building
- Build the project with: `make`
- Clean build artifacts with: `make clean` (note: the current Makefile removes `bin/` artifacts and `obj/*.ob` files; `.o` object files in `obj/` are **not** removed due to a typo and must be deleted manually or after fixing the Makefile)
- Build and run with: `make run`
- Compiled binaries go in `bin/` directory
- Object files go in `obj/` directory
- Use `-Iinclude` flag when compiling to include project headers

### Directory Structure
- `src/` - Source files (.c)
- `include/` - Header files (.h)
- `bin/` - Compiled binaries (not committed)
- `obj/` - Object files (not committed)

## Database Architecture

### Core Components
- **main.c**: Entry point, command-line argument parsing, and main program flow
- **file.c/file.h**: File operations for creating and opening database files
- **parse.c/parse.h**: Database header validation and output operations
- **common.h**: Shared constants and definitions

### Database Operations
- Database files use `.db` extension
- Headers are validated before use with `validate_db_header()`
- Database state is maintained in memory during operations
- Changes are written back to disk using `output_file()`

## Command-Line Interface
- Use `getopt()` for parsing command-line arguments
- Required argument: `-f <database_file>` for file path
- Optional flag: `-n` to create a new database file
- Always provide usage information via `print_usage()` function

## Memory and Resource Management
- Always initialize file descriptors to -1
- Always initialize pointers to NULL
- Close file descriptors when done
- Free all allocated memory before exiting
- Use cleanup sections with goto for consistent resource cleanup

## Security Best Practices
- Never hard-code credentials or sensitive information
- Validate all user inputs (especially file paths)
- Check return values from all system calls
- Use safe file permissions (0644) when creating files
- Handle file existence errors appropriately

## Comments
- The project compiles with modern C standards (C99 or later)
- Both `//` single-line and `/* */` multi-line comment styles are acceptable
- Use comments sparingly - code should be self-documenting
- Avoid stating the obvious in comments
- Only add comments for complex logic or non-obvious behavior
- Keep comments concise and up-to-date with code changes
