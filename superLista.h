#ifndef SUPER_LISTA_H
#define SUPER_LISTA_H

#include "listaOrdenada.h"

typedef unsigned char byte; //define o tipo de dados "byte"

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    int nroRegistros;   //numero de registros presentes no arquivo de indice
} regCabecI;

typedef struct {  //define o tipo de dados "registro de dados"
    char chaveBusca[120];
    long long byteOffset;   //byte offset do registro de dados principal referenciado por ele
} regDadosI;

typedef struct velo {   //vetor estatico de listas ordenadas
    ListaOrd alfabeto[26];
} SL;
typedef SL* SuperLista;


SuperLista criaSuperLista();
void adicionaSuperLista(SuperLista sl, regDadosI *dado);
void removeSuperLista(SuperLista sl, regDadosI *dado);
void printSuperLista(SuperLista sl);
void freeSuperLista(SuperLista sl);

#endif
