all:
	make -C src clean
	make -C src CFLAGS="-O3"

debug:
	make -C src clean
	make -C src CFLAGS="-O0"

clean:
	make -C src clean
