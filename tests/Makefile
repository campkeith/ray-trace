HEADERS=../src/vector.h ../src/surface.h ../src/color.h ../src/scene.h

TARGET=test_input_file

all: ${TARGET}

%.o: %.c ${HEADERS}
	gcc -g -Wall -Werror -ansi -D_ISOC99_SOURCE -I../src ${CFLAGS} -c $< -o $@

test_input_file: ../src/input_file.o test_input_file.o
	gcc $^ -lm -o $@
	./$@

clean:
	rm -f ${TARGET} test_input_file.o
