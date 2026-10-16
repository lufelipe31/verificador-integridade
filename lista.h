#ifndef LISTA_H
#define LISTA_H

#include <stddef.h>

/* Lista dinâmica de textos (aqui, caminhos de arquivos). */
typedef struct {
    char **itens;
    size_t qtd;
    size_t cap;
} Lista;

void lista_iniciar(Lista *lista);
int lista_adicionar(Lista *lista, const char *texto); /* 0 ok, -1 sem memória */
void lista_ordenar(Lista *lista);
void lista_liberar(Lista *lista);

/* Percorre `base` recursivamente e adiciona à lista o caminho relativo de
   cada arquivo regular, usando '/' como separador. Retorna 0 ou -1. */
int listar_arquivos(const char *base, Lista *lista);

#endif