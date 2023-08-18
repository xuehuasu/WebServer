all:
	mkdir -p bin
	mkdir -p log
	mkdir -p objs
	cd code && make
clean:
	cd code && make clean
	rm -rf bin
	rm -rf log
	rm -rf objs
