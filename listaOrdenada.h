#ifndef LISTA_ORD_H
#define LISTA_ORD_H

typedef struct lo LO;
typedef LO* ListaOrd;

ListaOrd criaListaOrd(int (*funcao_comparacao)(void *, void *), void (*funcao_free)(void *), void (*funcao_print)(void *));
void insereListaOrd(ListaOrd l, void *elem);
void removeListaOrd(ListaOrd l, void *elem);
void *primeiroListaOrd(ListaOrd l);
int vaziaListaOrd(ListaOrd l);
void printListaOrd(ListaOrd l);
void freeListaOrd(ListaOrd l);

#endif
