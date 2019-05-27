#ifndef MANIPULA_INDICE_H
#define MANIPULA_INDICE_H
#include <stdio.h>

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

typedef unsigned char byte; //define o tipo de dados "byte"

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    int nroRegistros;   //numero de registros presentes no arquivo de indice
} regCabecI;

typedef struct {  //define o tipo de dados "registro de dados"
    char chaveBusca[120];
    long long byteOffset;   //byte offset do registro de dados principal referenciado por ele
} regDadosI;

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
    void leCabecalhoIndice(FILE *file, regCabecI *cabecalho);
//le do arquivo um registro de dados e o coloca na estrutura passada por referencia
    void leRegistroIndice(FILE *file, regDadosI *registro);  //DEPRECATED
//insere o cabecalho no arquivo binario
    void insereCabecalhoIndice(FILE *file, regCabecI *cabecalho);
//insere o registro no arquivo binario (CUIDADO: anda com o seek)
    void insereRegistroIndice(FILE *file, regDadosI *registro);
//verifica o espaco disponivel na pagina de disco atual
    void checaFimPaginaIndice(FILE *file, regDadosI *registro, int tamAntigo);

//imprime na tela o registro de dados passado como parametro.
    void printRegistroIndice(regDadosI *registro);

#endif
