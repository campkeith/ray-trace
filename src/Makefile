OBJECTS=vector.o surface.o color.o input_file.o output_file.o ray_trace.o main.o
HEADERS=vector.h surface.h color.h input_file.h output_file.h ray_trace.h scene.h

TARGET=../bin/ray_trace

all: ${TARGET}

%.o: %.c ${HEADERS}
	gcc -g -Wall -Werror -ansi -D_ISOC99_SOURCE ${CFLAGS} -c $< -o $@

${TARGET}: ${OBJECTS}
	gcc ${OBJECTS} -lm -o $@

clean:
	rm -f ${TARGET} ${OBJECTS}
