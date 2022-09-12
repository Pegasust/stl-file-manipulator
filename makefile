COMPILE_FLAGS= -g -Wall -Werror -Wpedantic -std=c11

all: generator test

3d.o: 3d.h 3d.c 3d_object_factory.c 3d_representation.c 3d_writer.c
	gcc $(COMPILE_FLAGS) -c 3d.c

generator: generator.c 3d.o
	gcc $(COMPILE_FLAGS) -o $@ $^ -lm

test: generator
	valgrind --leak-check=full ./generator

submit: 3d.h 3d.c generator.c makefile 3d_object_factory.c 3d_representation.c 3d_writer.c
	mkdir -p pa10/stl
	cp $^ pa10/stl
	zip -r pa10.zip ./pa10