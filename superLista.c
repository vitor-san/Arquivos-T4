#include "superLista.h"
#include "listaOrdenada.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct velo {   //vetor estatico de listas ordenadas
    ListaOrd alfabeto[26];
};

/*
    Funcao de comparacao entre dois registros
    de dados do arquivo de indices.
*/
int compareIndice(void *reg1, void *reg2) {
    regDadosI *r = (regDadosI *)reg1;
    regDadosI *s = (regDadosI *)reg2;

    int comp = strcmp(r->chaveBusca, s->chaveBusca);

    if (comp == 0) {
        long long byteOff1 = r->byteOffset;
        long long byteOff2 = s->byteOffset;
        return (r > s) - (r < s);
    }
    else return comp;
}

/*
    Funcao para imprimir um registro de
    dados do arquivo de indices.
*/
void printIndice(void *reg) {
    regDadosI print = *((regDadosI *)reg);
    printf("Chave de busca: %s\n", print.chaveBusca);
    printf("Byte offset: %lld\n\n", print.byteOffset);
}

/*
    Funcao para liberar memoria, anteriormente
    alocada pelo indice.
*/
void freeIndice(void *reg) {
    free(reg);
}

/*
    Instancia uma nova "Super Lista",
    que nada mais é do que um conjunto
    de listas ordenadas. Cada elemento
    do conjunto eh indexado pela letra
    inicial que todos os nomes daquela
    lista ordenada terao.

    Retorno:
        SuperLista - a estrutura descrita
    acima
*/
SuperLista criaSuperLista() {
    SuperLista nova = malloc(sizeof(SL));

    for (int i = 0; i < 26; i++) {
        nova->alfabeto[i] = criaListaOrd(compareIndice, freeIndice, printIndice);
    }

    return nova;
}

/*
    Adiciona para a Super Lista
    um novo elemento.

    Parametros:
        SuperLista sl - Super Lista
    a ser modificada
        regDadosI *dado - elemento
    a ser adicionado
*/
void adicionaSuperLista(SuperLista sl, regDadosI *dado) {
    if (sl == NULL || dado == NULL) return;

    regDadosI *copiaDado = malloc(sizeof(regDadosI));
    memcpy(copiaDado, dado, sizeof(regDadosI)); //faco uma copia do dado que me foi passado como parametro, para nao perder seu conteudo

    int index = dado->chaveBusca[0] - 'A';
    insereListaOrd(sl->alfabeto[index], copiaDado);
}

/*
    Remove um elemento ja existente
    da Super Lista.

    Parametros:
        SuperLista sl - Super Lista
    a ser modificada
        regDadosI *dado - elemento
    a ser removido
*/
void removeSuperLista(SuperLista sl, regDadosI *dado) {
    if (sl == NULL || dado == NULL) return;

    regDadosI *copiaDado = malloc(sizeof(regDadosI));
    memcpy(copiaDado, dado, sizeof(regDadosI)); //faco uma copia do dado que me foi passado como parametro, para nao perder seu conteudo

    int index = dado->chaveBusca[0] - 'A';
    removeListaOrd(sl->alfabeto[index], copiaDado);
    free(copiaDado);
}

/*
    Imprime na tela a Super Lista
    passada por parametro.

    Parametro:
        SuperLista sl - lista a ser
    impressa
*/
void printSuperLista(SuperLista sl) {
    if (sl == NULL) return;

    for (int i = 0; i < 26; i++) printListaOrd(sl->alfabeto[i]);
}

/*
    Libera a memoria alocada para
    a Super Lista.

    Parametro:
        SuperLista sl - lista a ter
    sua memoria liberada
*/
void freeSuperLista(SuperLista sl) {
    if (sl == NULL) return;

    for (int i = 0; i < 26; i++) freeListaOrd(sl->alfabeto[i]);
    free(sl);
}
