CFLAGS = -Wall -g -fPIC

liblwp.so: lwp.o lwp.h fp.h magic64.o
	gcc $(CFLAGS) -shared -o liblwp.so lwp.h fp.h magic64.o lwp.o

magic64.o: magic64.S
	gcc $(CFLAGS) -o magic64.o -c magic64.S

lwp.o: lwp.c
	gcc $(CFLAGS) -o lwp.o -c lwp.c

clean: 
	rm *.o
	rm liblwp.so
	make

testfile: tests.o Random.o liblwp.so
	gcc $(CFLAGS) -L . -l:liblwp.so -o tester Random.o tests.o

tests.o: tests.c
	gcc $(CFLAGS) -o tests.o -c tests.c 

Random.o: Random.c
	gcc $(CFLAGS) -o Random.o -c Random.c
