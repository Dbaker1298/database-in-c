# What is going on the HeaderFile validate logic?

- We read the database (file) off of the disk.
- We bring it into  memory.
- We "unpack it" so we can work on it.
- We do operations on the file.
- We put it back to disk via our `output_file` function. `./src/parse.c`

