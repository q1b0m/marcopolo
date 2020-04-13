
lee-lista: lee-lista.o tipos_xdr.o
	gcc -o lee-lista lee-lista.o tipos_xdr.o

lee-lista.o: lee-lista.c
	gcc -c lee-lista.c

tipos_xdr.o: tipos_xdr.c
	gcc -c tipos_xdr.c

clean:
	-rm *.o
