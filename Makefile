all:
	gcc -o programaTrab14 main.c escreverTela.c manipulaArquivo.c manipulaIndice.c superLista.c listaOrdenada.c -g
run:
	./programaTrab14
debug:
	gdb programaTrab14
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab14
zip:
	zip trab14.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h manipulaIndice.c manipulaIndice.h superLista.c superLista.h listaOrdenada.c listaOrdenada.h Makefile
bin14:
	cp -t $(shell pwd) ./casos/14.in
	cp -t $(shell pwd) ./binarios/binario-14.bin
	cp -t $(shell pwd) ./binarios/binario-14.index
	make all
case14:
	cp -t $(shell pwd) ./casos/14.in
	cp -t $(shell pwd) ./binarios/binario-14.bin
	cp -t $(shell pwd) ./binarios/binario-14.index
	make all
	make run < 14.in
diff:
	make case14 > 14meu.out
	diff 14meu.out ./casos/14.out
meld:
	hexdump -Cv binario-14.index > try.txt
	hexdump -Cv ./binarios/binario-14-depois.index > target.txt
	meld try.txt target.txt
