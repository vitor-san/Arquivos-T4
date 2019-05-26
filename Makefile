all:
	gcc -o programaTrab3 main.c escreverTela.c manipulaArquivo.c -g
run:
	./programaTrab3
debug:
	gdb programaTrab3
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab3
zip:
	zip trab3.zip main.c escreverTela.c escreverTela.h manipulaArquivo.c manipulaArquivo.h Makefile
case1:
	cp -t /home/vitorsan/Desktop/Trabalhos-USP/Files/Trabalho3 ./casos/1.in
	cp -t /home/vitorsan/Desktop/Trabalhos-USP/Files/Trabalho3 ./binarios/binario-1.bin
	make all
	make run < 1.in
diff:
	make case1 > 1meu.out
	diff 1meu.out ./casos/1.out
meld:
	hexdump -Cv binario-1-index.bin > try.txt
	hexdump -Cv ./binarios/binario-1-index.bin > target.txt
	meld try.txt target.txt
