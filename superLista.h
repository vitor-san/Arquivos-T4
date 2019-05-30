#ifndef SUPER_LISTA_H
#define SUPER_LISTA_H

typedef struct velo SL;
typedef SL* SuperLista;

typedef struct {  //define o tipo de dados "registro de cabecalho"
    byte status;
    int nroRegistros;   //numero de registros presentes no arquivo de indice
} regCabecI;

typedef struct {  //define o tipo de dados "registro de dados"
    char chaveBusca[120];
    long long byteOffset;   //byte offset do registro de dados principal referenciado por ele
} regDadosI;

SuperLista criaSuperLista();
void adicionaSuperLista(SuperLista sl, regDadosI *dado)
void removeSuperLista(SuperLista sl, regDadosI *dado)
void freeSuperLista(SuperLista sl);

#endif
