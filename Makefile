all:
	gcc -o programaTrab1 main.c escreverTela.c manipulaArquivo.c manipulaIndice.c superLista.c listaOrdenada.c -g
run:
	./programaTrab1
debug:
	gdb programaTrab1
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab1
zip:
	zip trab4.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h manipulaIndice.c manipulaIndice.h superLista.c superLista.h listaOrdenada.c listaOrdenada.h Makefile
case1:
	cp -t $(shell pwd) ./casos/1.in
	cp -t $(shell pwd) ./binarios/binario-1.bin
	make all
	make run < 1.in
diff:
	make case1 > 1meu.out
	diff 1meu.out ./casos/1.out
meld:
	hexdump -Cv binario-1.index > try.txt
	hexdump -Cv ./binarios/binario-1.index > target.txt
	meld try.txt target.txt
