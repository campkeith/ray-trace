all:
	make -C src clean
	make -C src CFLAGS="-O3"

debug:
	make -C src clean
	make -C src CFLAGS="-O0"

test:
	make -C tests clean
	make -C tests

clean:
	make -C src clean
	make -C tests clean
