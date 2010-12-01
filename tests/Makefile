HEADERS=../src/vector.h ../src/surface.h ../src/color.h ../src/scene.h

TARGETS=test_input_file test_ray_trace

all: ${TARGETS}

%.o: %.c ${HEADERS}
	gcc -g -Wall -Werror -ansi -D_ISOC99_SOURCE -I../src ${CFLAGS} -c $< -o $@

test_input_file: ../src/input_file.o test_input_file.o
	gcc $^ -lm -o $@
	- ./$@

test_ray_trace: ../src/ray_trace.o ../src/color.o ../src/surface.o test_ray_trace.o
	gcc $^ -lm -o $@
	- ./$@

clean:
	rm -f ${TARGETS} *.o
