static.vx: static.o
	gcc -shared -o static.vx static.o
static.o: static.c
	gcc -o static.o -c -Wall -Werror -fPIC static.c
install: static.vx
	cp static.vx ../../bin/
	chmod +x ../../bin/static.vx
clean:
	rm -f static.o
