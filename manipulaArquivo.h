#ifndef MANIPULA_ARQUIVO_H
#define MANIPULA_ARQUIVO_H
#include <stdio.h>

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

typedef unsigned char byte; //define o tipo de dados "byte"

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    long long topoLista;
    char tagCampo[5];
    char desCampo[5][40];
} regCabec;

typedef struct {  //define o tipo de dados "registro de dados"
    char removido;
    int tamanhoRegistro;
    long long encadeamentoLista; //inteiro de 8 bytes
    int idServidor;
    double salarioServidor;
    char telefoneServidor[14];
    int tamCampo4;  //indicador de tamanho
    char tagCampo4;
    char *nomeServidor; //string de tamanho variavel
    int tamCampo5;  //indicador de tamanho
    char tagCampo5;
    char *cargoServidor;  //string de tamanho variavel
} regDados;

/*
* NOTA: Por simplicidade, adotou-se que todas
* as funcoes que manipularem registros de dados
* conterao em seu nome apenas "Registro".
* Quando a funcao manipular o registro de cabecalho,
* isso estara explicito em seu nome.
*/

//inicializa um novo registro de cabecalho
    regCabec *criaCabecalho();
//inicializa um novo registro de dados
    regDados *criaRegistro();
//calcula e atualiza o indicador de tamanho do registro
    void calculaTamanho(regDados *registro);
//le o registro de cabecalho e o coloca na estrutura passada por referencia
    void leCabecalho(FILE *file, regCabec *cabecalho);
//le do arquivo um registro de dados e o coloca na estrutura passada por referencia
    void leRegistro(FILE *file, regDados *registro);
//insere o cabecalho no arquivo binario
    void insereCabecalho(FILE *file, regCabec *cabecalho);
//insere o registro no arquivo binario (CUIDADO: anda com o seek)
    void insereRegistro(FILE *file, regDados *registro);
//verifica o espaco disponivel na pagina de disco atual
    void checaFimPagina(FILE *file, regDados *registro, int tamAntigo);
//adiciona o registro (cujo byte offset eh newBO) na lista de removidos
    void adicionaLista(FILE *file, long long newBO, int tamRegistro);
//sobreescreve os campos do registro com lixo
    void completaLixo(FILE *file);
//insere o registro na primeira posicao disponivel, de acordo com a lista de removidos
    long long achaPosicaoInsere(FILE *file, regDados *registro, long long ultimoBO);
//insere o registro na primeira posicao disponivel, de acordo com a lista de removidos, e deixa o seek no final desse registro
    long long achaPosicaoInsereSeek(FILE *file, regDados *registro, long long ultimoBO);
//imprime na tela o registro de dados + os metadados referentes a cada campo
    void mostraRegistroMeta(regCabec *cabecalho, regDados *registro);
//imprime na tela o registro de dados passado como parametro
    void printRegistro(regDados *registro);
//imprime na tela a lista de removidos do arquivo
    void printLista(FILE *file);
//libera a memoria alocada para o registro
    void freeRegistro(regDados *registro);

#endif
