# How the HeaderFile validation logic works

- We read the database (file) off of the disk.
- We bring it into memory.
- We unpack it so we can work with it.
- We do operations on the file.
- We put it back to disk via our `output_file` function. `./src/parse.c`
