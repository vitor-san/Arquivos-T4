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
<<<<<<< HEAD
case4:
	cp -t $(shell pwd) ./casos/4.in
	cp -t $(shell pwd) ./binarios/binario-4.bin
	cp -t $(shell pwd) ./binarios/binario-4.index
	make all
	make run < 4.in
diff:
	make case4 > 4meu.out
	diff 4meu.out ./casos/4.out
meld:
	hexdump -Cv binario-4.index > try.txt
	hexdump -Cv ./binarios/binario-4.index > target.txt
=======
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
>>>>>>> 2540a2c044f51dd6cddf7e2c23442d28db894eb2
	meld try.txt target.txt
