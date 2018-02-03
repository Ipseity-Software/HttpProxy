mysql_resolve.vx: mysql_resolve.o
	gcc -shared -o mysql_resolve.vx mysql_resolve.o `mysql_config --cflags --libs`
mysql_resolve.o: mysql_resolve.c
	gcc -o mysql_resolve.o -c -Wall -Werror -fPIC mysql_resolve.c `mysql_config --cflags --libs`
install: mysql_resolve.vx
	cp mysql_resolve.vx ../../bin/
	chmod +x ../../bin/mysql_resolve.vx
clean:
	rm -f mysql_resolve.o
