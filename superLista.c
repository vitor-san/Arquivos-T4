#include "superLista.h"
#include "listaOrdenada.h"

struct velo {   //vetor estatico de listas ordenadas
    ListaOrd[26] alfabeto;
};

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
        nova->alfabeto[i] = criaListaOrd(compare, freeRegistroIndice, printRegistroIndice);
    }

    return nova;
}

/*
    Funcao de comparacao entre dois registros
    de dados do arquivo de indices.
*/
int compare(void *reg1, void *reg2) {
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
void printRegistroIndice(void *reg) {
    regDadosI print = *((regDadosI *)reg);
    printf("Chave de busca: %s\n", print.chaveBusca);
    printf("Byte offset: %lld\n\n", print.byteOffset);
}

/*
    Funcao para liberar memoria, anteriormente
    alocada pelo indice.
*/
void freeRegistroIndice(void *reg) {
    free(reg);
}
