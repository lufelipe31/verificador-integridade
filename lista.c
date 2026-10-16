#include "lista.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define TAM_CAMINHO 4096

void lista_iniciar(Lista *lista) {
    lista->itens = NULL;
    lista->qtd = 0;
    lista->cap = 0;
}

int lista_adicionar(Lista *lista, const char *texto) {
    /* Dobra a capacidade quando enche: poucas chamadas a realloc, mesmo
       com milhares de arquivos. */
    if (lista->qtd == lista->cap) {
        size_t nova = lista->cap ? lista->cap * 2 : 64;
        char **novo = realloc(lista->itens, nova * sizeof(char *));
        if (novo == NULL) {
            return -1;
        }
        lista->itens = novo;
        lista->cap = nova;
    }

    size_t tam = strlen(texto) + 1;
    char *copia = malloc(tam);
    if (copia == NULL) {
        return -1;
    }
    memcpy(copia, texto, tam);
    lista->itens[lista->qtd++] = copia;
    return 0;
}

static int comparar(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

void lista_ordenar(Lista *lista) {
    if (lista->qtd > 1) {
        qsort(lista->itens, lista->qtd, sizeof(char *), comparar);
    }
}

void lista_liberar(Lista *lista) {
    for (size_t i = 0; i < lista->qtd; i++) {
        free(lista->itens[i]);
    }
    free(lista->itens);
    lista_iniciar(lista);
}

/* Junta a e b com '/'. Se a for vazio, devolve só b. Retorna -1 se não
   couber em `saida`, para nunca truncar um caminho em silêncio. */
static int juntar(char *saida, size_t tam, const char *a, const char *b) {
    int n = (a[0] == '\0') ? snprintf(saida, tam, "%s", b)
                           : snprintf(saida, tam, "%s/%s", a, b);
    return (n < 0 || (size_t)n >= tam) ? -1 : 0;
}

static int percorrer(const char *base, const char *rel, Lista *lista) {
    char dirbuf[TAM_CAMINHO];
    const char *dir = base;

    if (rel[0] != '\0') {
        if (juntar(dirbuf, sizeof dirbuf, base, rel) != 0) {
            fprintf(stderr, "Caminho muito longo: %s/%s\n", base, rel);
            return -1;
        }
        dir = dirbuf;
    }

    DIR *d = opendir(dir);
    if (d == NULL) {
        fprintf(stderr, "Nao foi possivel abrir a pasta: %s\n", dir);
        return -1;
    }

    int status = 0;
    struct dirent *e;

    while (status == 0 && (e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
            continue;
        }

        char novo_rel[TAM_CAMINHO];
        char completo[TAM_CAMINHO];
        if (juntar(novo_rel, sizeof novo_rel, rel, e->d_name) != 0 ||
            juntar(completo, sizeof completo, base, novo_rel) != 0) {
            fprintf(stderr, "Caminho muito longo: %s\n", e->d_name);
            status = -1;
            break;
        }

        /* Um arquivo que não dá para ler é erro, não "ignorar": um manifesto
           incompleto daria uma falsa sensação de segurança. */
        struct stat st;
        if (stat(completo, &st) != 0) {
            fprintf(stderr, "Nao foi possivel ler: %s\n", completo);
            status = -1;
            break;
        }

        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            status = percorrer(base, novo_rel, lista);
        } else if ((st.st_mode & S_IFMT) == S_IFREG) {
            if (lista_adicionar(lista, novo_rel) != 0) {
                fprintf(stderr, "Sem memoria\n");
                status = -1;
            }
        }
    }

    closedir(d);
    return status;
}

int listar_arquivos(const char *base, Lista *lista) {
    return percorrer(base, "", lista);
}