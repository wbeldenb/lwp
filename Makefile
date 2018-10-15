CFLAGS = -Wall -g -pfic

liblwp.so: lwp.o lwp.h fp.h magic64.o
	gcc $(CFLAGS) -shared -o liblwp.so lwp.h fp.h magic64.o lwp.o

magic64.o: magic64.S
	gcc $(CFLAGS) -o magic64.o -c magic64.S

lwp.o: lwp.c
	gcc $(CFLAGS) -o lwp.o lwp.c