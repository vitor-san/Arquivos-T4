all:
	gcc -o programaTrab4 main.c escreverTela.c manipulaArquivo.c manipulaIndice.c superLista.c listaOrdenada.c -g
run:
	./programaTrab4
debug:
	gdb programaTrab4
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab4
zip:
	zip trab4.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h manipulaIndice.c manipulaIndice.h superLista.c superLista.h listaOrdenada.c listaOrdenada.h Makefile
bin8:
	cp -t $(shell pwd) ./casos/8.in
	cp -t $(shell pwd) ./binarios/binario-8.bin
	cp -t $(shell pwd) ./binarios/binario-8.index
	make all
case8:
	cp -t $(shell pwd) ./casos/8.in
	cp -t $(shell pwd) ./binarios/binario-8.bin
	cp -t $(shell pwd) ./binarios/binario-8.index
	make all
	make run < 8.in
diff:
	make case8 > 8meu.out
	diff 8meu.out ./casos/8.out
meld:
	hexdump -Cv binario-8.index > try.txt
	hexdump -Cv ./binarios/binario-8-depois.index > target.txt
	meld try.txt target.txt
