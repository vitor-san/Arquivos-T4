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
bin10:
	cp -t $(shell pwd) ./casos/10.in
	cp -t $(shell pwd) ./binarios/binario-10.bin
	cp -t $(shell pwd) ./binarios/binario-10.index
	make all
case10:
	cp -t $(shell pwd) ./casos/10.in
	cp -t $(shell pwd) ./binarios/binario-10.bin
	cp -t $(shell pwd) ./binarios/binario-10.index
	make all
	make run < 10.in
diff:
	make case10 > 10meu.out
	diff 10meu.out ./casos/10.out
meld:
	hexdump -Cv binario-10.index > try.txt
	hexdump -Cv ./binarios/binario-10-depois.index > target.txt
	meld try.txt target.txt
