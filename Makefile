all:
	gcc -o programaTrab4 main.c escreverTela.c manipulaArquivo.c manipulaIndice.c -g
run:
	./programaTrab4
debug:
	gdb programaTrab4
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab4
zip:
	zip trab4.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h manipulaIndice.c manipulaIndice.h Makefile
case1:
	cp -t $(shell pwd) ./casos/1.in
	cp -t $(shell pwd) ./binarios/binario-1.bin
	make all
	make run < 1.in
diff:
	make case1 > 1meu.out
	diff 1meu.out ./casos/1.out
meld:
	hexdump -Cv binario-1-index.bin > try.txt
	hexdump -Cv ./binarios/binario-1-index.bin > target.txt
	meld try.txt target.txt
