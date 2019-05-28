#include "listaOrdenada.h"
#include <stdlib.h>

typedef struct no {
    struct no *prox;
    void *dado;
} Bloco;

struct lo {
    int (*funcao_comparacao)(void *, void *);
    void (*funcao_free)(void *);
    void (*funcao_print)(void *);
    Bloco *inicio;
};

Bloco *novoBloco(void *dado) {
    Bloco *novo = malloc(sizeof(Bloco));
    novo->dado = dado;
    novo->prox = NULL;
    return novo;
}

void destroiBloco(Bloco *b, void (*funcao_free)(void *dado)) {
    if (b == NULL) return;

    b->prox = NULL;
    free_function(b->dado);
    free(b);
}

void insereListaOrd(ListaOrd *l, void *elem) {
    if (l == NULL || elem == NULL) return;

    Bloco *atual = l->inicio;
    Bloco *novo = novoBloco(elem);

    while (atual->prox != NULL && l->funcao_comparacao(novo->dado, atual->prox->dado) >= 0) {
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

void removeListaOrd(ListaOrd *l, void *elem) {
    if (l == NULL || elem == NULL) return;

    Bloco *atual = l->inicio;
    Bloco *antAtual = NULL;
    Bloco *proxAtual = atual->prox;

    while (atual != NULL) {
        if (l->funcao_comparacao(atual->dado, elem) == 0) { //se achou o bloco a ser removido...
            if (ant != NULL) {   //remove do meio/fim
                ant->prox = proxAtual;
            }
            else {   //remove do inicio
                l->inicio = atual->prox;
            }
            destroiBloco(atual, l->funcao_free);
            atual = antAtual;
        }
        antAtual = atual;
        atual = proxAtual;
        proxAtual = proxAtual->prox;
    }
}

void printListaOrd(ListaOrd *l) {
    if (l == NULL) return;

    Bloco *atual = l->inicio;

    while (atual != NULL) {
        l->funcao_print(atual->dado);
        atual = atual->prox;
    }
}

void freeRecursivo(Bloco *b, void (*funcao_free)(void *)) {
    if (b == NULL) return;
    freeRecursivo(b->prox, funcao_free);
    destroiBloco(b, funcao_free);
}

void freeListaOrd(ListaOrd *l) {
    freeRecursivo(l->inicio, l->funcao_free);
    free(l);
}
