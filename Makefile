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
bin5:
	cp -t $(shell pwd) ./casos/5.in
	cp -t $(shell pwd) ./binarios/binario-5.bin
	cp -t $(shell pwd) ./binarios/binario-5.index
	make all
case5:
	cp -t $(shell pwd) ./casos/5.in
	cp -t $(shell pwd) ./binarios/binario-5.bin
	cp -t $(shell pwd) ./binarios/binario-5.index
	make all
	make run < 5.in
diff:
	make case5 > 5meu.out
	diff 5meu.out ./casos/5.out
meld:
	hexdump -Cv binario-5.index > try.txt
	hexdump -Cv ./binarios/binario-5-depois.index > target.txt
	meld try.txt target.txt
