#ifndef MANIPULA_INDICE_H
#define MANIPULA_INDICE_H

#include <stdio.h>
#include "superLista.h"

#define TAMPAG 32000  //tamanho da pagina de disco (em bytes)

/*
* NOTA: Por simplicidade, adotou-se que todas
* as funcoes que manipularem registros de dados
* conterao em seu nome apenas "Registro".
* Quando a funcao manipular o registro de cabecalho,
* isso estara explicito em seu nome.
*/

//inicializa um novo registro de cabecalho
    regCabecI *criaCabecalhoIndice();
//inicializa um novo registro de dados
    regDadosI *criaRegistroIndice();
//le o registro de cabecalho e o coloca na estrutura passada por referencia
    void leCabecalhoIndice(FILE *file, regCabecI *cabecalho);
//le um registro de dados e o coloca na estrutura passada por referencia (CUIDADO: anda com o seek)
    void leRegistroIndice(FILE *file, regDadosI *registro);
//insere o cabecalho no arquivo binario
    void insereCabecalhoIndice(FILE *file, regCabecI *cabecalho);
//insere o registro no arquivo binario (CUIDADO: anda com o seek)
    void insereRegistroIndice(FILE *file, regDadosI *registro);
//verifica o espaco disponivel na pagina de disco atual (CUIDADO: anda com o seek quando completa a pagina com lixo)
    void checaFimPaginaIndice(FILE *file);
//carrega todos os registros do arquivo de indices para um vetor na RAM
    regDadosI *carregaIndiceVetor(FILE *file);
//busca o registro no arquivo de indices
    long long* buscaRegistroIndice(regDadosI *v, char* chave, int ini, int fim, int* comeco, int* tam);
//carrega todos os registros do arquivo de indices para uma "super lista" na RAM
    SuperLista carregaIndiceLista(FILE *file);
//reescreve o arquivo de indices, atualizando-o com as modificacoes feitas em memoria RAM (CUIDADO: anda com o seek)
    void reescreveArquivoIndice(FILE *file, regCabecI *cabec, SuperLista base);

#endif
