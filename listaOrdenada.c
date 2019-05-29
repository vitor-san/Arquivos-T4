#include "listaOrdenada.h"
#include <stdlib.h>

typedef struct no {
    struct no *prox;
    void *dado;
} Bloco;

struct lo {
    int (*funcaoComparacao)(void *, void *);
    void (*funcaoFree)(void *);
    void (*funcaoPrint)(void *);
    Bloco *inicio;
};

Bloco *novoBloco(void *dado) {
    Bloco *novo = malloc(sizeof(Bloco));
    novo->dado = dado;
    novo->prox = NULL;
    return novo;
}

void destroiBloco(Bloco *b, void (*funcaoFree)(void *dado)) {
    if (b == NULL) return;

    b->prox = NULL;
    funcaoFree(b->dado);
    free(b);
}

ListaOrd criaListaOrd(int (*funcaoComparacao)(void *, void *), void (*funcaoFree)(void *), void (*funcaoPrint)(void *)) {
    ListaOrd nova = malloc(sizeof(LO));

    nova->funcaoComparacao = funcaoComparacao;
    nova->funcaoFree = funcaoFree;
    nova->funcaoPrint = funcaoPrint;
    nova->inicio = novoBloco(NULL);

    return nova;
}

void insereListaOrd(ListaOrd l, void *elem) {
    if (l == NULL || elem == NULL) return;

    Bloco *atual = l->inicio;
    Bloco *novo = novoBloco(elem);

    while (atual->prox != NULL && l->funcaoComparacao(novo->dado, atual->prox->dado) >= 0) {
        atual = atual->prox;
    }

    if (atual->prox == NULL) {
        atual->prox = novo;
    }
    else {
        novo->prox = atual->prox;
        atual->prox = novo;
    }
}

void removeListaOrd(ListaOrd l, void *elem) {
    if (l == NULL || elem == NULL) return;

    Bloco *atual = l->inicio->prox;
    Bloco *antAtual = l->inicio;
    Bloco *proxAtual = atual->prox;

    while (atual != NULL) {
        int comparacao = l->funcaoComparacao(atual->dado, elem);
        if (comparacao == 0) { //se achou o bloco a ser removido...
            if (antAtual != NULL) {   //remove do meio/fim
                antAtual->prox = proxAtual;
            }
            else {   //remove do inicio
                l->inicio = atual->prox;
            }
            if (l->funcaoComparacao(elem, proxAtual->dado) == 0 && proxAtual != NULL) elem = proxAtual->dado;
            destroiBloco(atual, l->funcaoFree);
            atual = antAtual;
        }
        else if (comparacao > 0) return;    //os elementos restantes da lista sao maiores do que o elemento buscado
        antAtual = atual;
        atual = proxAtual;
        if (proxAtual != NULL) proxAtual = proxAtual->prox;
    }
}

void printListaOrd(ListaOrd l) {
    if (l == NULL) return;

    Bloco *atual = l->inicio->prox;

    while (atual != NULL) {
        l->funcaoPrint(atual->dado);
        atual = atual->prox;
    }
}

void freeRecursivo(Bloco *b, void (*funcaoFree)(void *)) {
    if (b == NULL) return;
    freeRecursivo(b->prox, funcaoFree);
    destroiBloco(b, funcaoFree);
}

void freeListaOrd(ListaOrd l) {
    freeRecursivo(l->inicio, l->funcaoFree);
    free(l);
}
