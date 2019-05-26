#ifndef MANIPULA_INDICE_H
#define MANIPULA_INDICE_H
#include <stdio.h>

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

typedef unsigned char byte; //define o tipo de dados "byte"

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    int nroRegistros;   //numero de registros presentes no arquivo de indice
} regCabec;

typedef struct {  //define o tipo de dados "registro de dados"
    char chaveBusca[120];
    long long byteOffset;   //byte offset do registro de dados principal referenciado por ele
} regDados;

/*
* NOTA: Por simplicidade, adotou-se que todas
* as funcoes que manipularem registros de dados
* conterao em seu nome apenas "Registro".
* Quando a funcao manipular o registro de cabecalho,
* isso estara explicito em seu nome.
*/

//inicializa um novo registro de cabecalho
    regCabec *criaCabecalhoIndice();
//inicializa um novo registro de dados
    regDados *criaRegistroIndice();
//le o registro de cabecalho e o coloca na estrutura passada por referencia
    void leCabecalhoIndice(FILE *file, regCabec *cabecalho);
//le do arquivo um registro de dados e o coloca na estrutura passada por referencia
    void leRegistroIndice(FILE *file, regDados *registro);
//insere o cabecalho no arquivo binario
    void insereCabecalhoIndice(FILE *file, regCabec *cabecalho);
//insere o registro no arquivo binario (CUIDADO: anda com o seek)
    void insereRegistroIndice(FILE *file, regDados *registro);
//verifica o espaco disponivel na pagina de disco atual
    void checaFimPaginaIndice(FILE *file, regDados *registro, int tamAntigo);
//adiciona o registro (cujo byte offset eh newBO) na lista de removidos
    void adicionaLista(FILE *file, long long newBO, int tamRegistro);
//sobreescreve os campos do registro com lixo
    void completaLixo(FILE *file);
//insere o registro na primeira posicao disponivel, de acordo com a lista de removidos
    long long achaPosicaoInsere(FILE *file, regDados *registro, long long ultimoBO);
//imprime na tela o registro de dados passado como parametro.
    void printRegistro(regDados *registro);
//imprime na tela a lista de removidos do arquivo.
    void printLista(FILE *file);
//libera a memoria alocada para o registro
    void freeRegistro(regDados *registro);

#endif
