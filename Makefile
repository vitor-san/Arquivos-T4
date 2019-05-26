all:
	gcc -o programaTrab1 main.c escreverTela.c manipulaArquivo.c -g
run:
	./programaTrab1
debug:
	gdb programaTrab1
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab1
zip:
	zip trab1.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h manipulaIndice.c manipulaIndice.h Makefile
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
