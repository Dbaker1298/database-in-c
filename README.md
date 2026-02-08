# LLA database-in-c

## Overview

This repository contains a lightweight employee database implementation in C that demonstrates fundamental database concepts, including file I/O, data serialization, and CRUD operations.

## Current Implementation

The current codebase implements a fully functional binary database system with the following features:

### Core Components

- **Binary File Format**: Custom database file format with magic number validation (0x4c4c4144) for file integrity
- **Database Header**: Stores metadata including version, employee count, and file size
- **Employee Records**: Structured data storage for employee information (name, address, hours worked)

### Functionality

1. **File Operations** (`file.c`, `file.h`)
   - Create new database files
   - Open existing databases in read-write or read-only modes
   - Proper file descriptor management and error handling

2. **Data Parsing** (`parse.c`, `parse.h`)
   - Serialize and deserialize employee records
   - Network byte order conversion for cross-platform compatibility
   - Database header validation and verification

3. **Employee Management**
   - Add new employees with name, address, and hours
   - List all employees with formatted output
   - Input validation and error handling

4. **Command-Line Interface** (`main.c`)
   - `-n`: Create a new database file
   - `-f <file>`: Specify database file path (required)
   - `-a <name, address, hours>`: Add an employee
   - `-l`: List all employees
   - Automatic read-only mode optimization when no mutations occur

### Key Features

- **Memory Safety**: Proper malloc/free management, buffer overflow protection
- **Data Integrity**: Magic number validation, file size verification, corruption detection
- **Error Handling**: Comprehensive error checking with cleanup paths for resource management
- **Portability**: Network byte order handling for cross-platform compatibility
- **Security**: Input validation, integer overflow prevention using SIZE_MAX checks

### Building and Running

```bash
# Build the project
make

# Create a new database
./bin/dbview -n -f mydb.db

# Add an employee
./bin/dbview -f mydb.db -a "John Doe,123 Main St,40"

# List all employees
./bin/dbview -f mydb.db -l

# Clean build artifacts
make clean
```

This implementation demonstrates production-quality C programming practices with a focus on low-level file I/O, memory safety, and robust error handling.
