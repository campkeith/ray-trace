TARGET=../bin/ppm_compare

all: ${TARGET}

${TARGET}: ppm_compare.c
	gcc -std=c99 -Wall $^ -o $@

clean:
	rm -f ${TARGET}
