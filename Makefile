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
case9:
	cp -t $(shell pwd) ./casos/9.in
	cp -t $(shell pwd) ./binarios/binario-9.bin
	cp -t $(shell pwd) ./binarios/binario-9.index
	make all
	make run < 9.in
diff:
	make case9 > 9meu.out
	diff 9meu.out ./casos/9.out
meld:
	hexdump -Cv binario-9.index > try.txt
	hexdump -Cv ./binarios/binario-9-depois.index > target.txt
	meld try.txt target.txt
