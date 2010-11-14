SOURCE=vector.c surface.c color.c input_file.c output_file.c ray_trace.c main.c
HEADERS=vector.h surface.h color.h input_file.h output_file.h ray_trace.h scene.h

TARGET=ray_trace
SCENE=../scenes/basic

all: ${TARGET}

${TARGET}: ${SOURCE} ${HEADERS}
	gcc -g -Wall -Werror -ansi -D_ISOC99_SOURCE ${SOURCE} -O3 -funroll-all-loops -lm -o $@

image.ppm: ${TARGET} ${SCENE}
	./${TARGET} < ${SCENE} > $@ 

clean:
	rm -f ${TARGET} image.ppm
