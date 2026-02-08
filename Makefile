TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
					./$(TARGET) -f ./mynewdb.db -n
					./$(TARGET) -f ./mynewdb.db -a "Michael Baker,1899 Oracle Way #1221,120"

default: 	$(TARGET)

clean:
					rm -f obj/*.ob
					rm -f bin/*
					rm -f *.db

$(TARGET): $(OBJ)
					gcc -o $@ $?

obj/%.o : src/%.c
					gcc -c $< -o $@ -Iinclude

