#include "verificacao.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arquivo_hash.h"
#include "lista.h"

#define TAM_CAMINHO 4096
#define TAM_LINHA (TAM_CAMINHO + SHA256_TAM_HEX + 8)

typedef struct {
    char hash[SHA256_TAM_HEX];
    char *caminho;
} Entrada;

static void liberar_entradas(Entrada *e, size_t qtd) {
    for (size_t i = 0; i < qtd; i++) {
        free(e[i].caminho);
    }
    free(e);
}

/* Lê o manifesto. Linha no formato: 64 hex, dois espaços, caminho. Qualquer
   linha fora desse formato é erro: um manifesto adulterado ou corrompido
   não pode ser aceito em silêncio. */
static int ler_manifesto(const char *arquivo, Entrada **saida, size_t *qtd_saida) {
    FILE *f = fopen(arquivo, "rb");
    if (f == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o manifesto: %s\n", arquivo);
        return -1;
    }

    Entrada *entradas = NULL;
    size_t qtd = 0, cap = 0;
    char linha[TAM_LINHA];
    int status = 0;

    while (fgets(linha, sizeof linha, f) != NULL) {
        size_t tam = strlen(linha);
        if (tam == 0 || linha[tam - 1] != '\n') {
            fprintf(stderr, "Linha invalida ou longa demais no manifesto\n");
            status = -1;
            break;
        }
        linha[--tam] = '\0';

        int hex_ok = tam > 66 && linha[64] == ' ' && linha[65] == ' ';
        for (int i = 0; hex_ok && i < 64; i++) {
            char c = linha[i];
            hex_ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        }
        if (!hex_ok) {
            fprintf(stderr, "Linha fora do formato no manifesto: %s\n", linha);
            status = -1;
            break;
        }

        if (qtd == cap) {
            size_t nova = cap ? cap * 2 : 64;
            Entrada *novo = realloc(entradas, nova * sizeof(Entrada));
            if (novo == NULL) {
                fprintf(stderr, "Sem memoria\n");
                status = -1;
                break;
            }
            entradas = novo;
            cap = nova;
        }

        char *copia = malloc(tam - 66 + 1);
        if (copia == NULL) {
            fprintf(stderr, "Sem memoria\n");
            status = -1;
            break;
        }
        memcpy(copia, linha + 66, tam - 66 + 1);
        memcpy(entradas[qtd].hash, linha, 64);
        entradas[qtd].hash[64] = '\0';
        entradas[qtd].caminho = copia;
        qtd++;
    }

    if (status == 0 && ferror(f)) {
        fprintf(stderr, "Erro ao ler o manifesto\n");
        status = -1;
    }
    fclose(f);

    if (status != 0) {
        liberar_entradas(entradas, qtd);
        return -1;
    }
    *saida = entradas;
    *qtd_saida = qtd;
    return 0;
}

int verificar_manifesto(const char *pasta, const char *arquivo_manifesto) {
    Entrada *esperado = NULL;
    size_t n_esperado = 0;
    if (ler_manifesto(arquivo_manifesto, &esperado, &n_esperado) != 0) {
        return -1;
    }

    Lista atual;
    lista_iniciar(&atual);
    if (listar_arquivos(pasta, &atual) != 0) {
        liberar_entradas(esperado, n_esperado);
        lista_liberar(&atual);
        return -1;
    }
    lista_ordenar(&atual);

    unsigned long modificados = 0, ausentes = 0, novos = 0;
    int status = 0;
    size_t i = 0, j = 0;

    /* As duas listas estão em ordem alfabética: avançamos nas duas ao mesmo
       tempo, comparando o caminho atual de cada uma. */
    while (status == 0 && (i < n_esperado || j < atual.qtd)) {
        int cmp;
        if (i >= n_esperado) {
            cmp = 1;
        } else if (j >= atual.qtd) {
            cmp = -1;
        } else {
            cmp = strcmp(esperado[i].caminho, atual.itens[j]);
        }

        if (cmp < 0) {
            printf("AUSENTE    %s\n", esperado[i].caminho);
            ausentes++;
            i++;
        } else if (cmp > 0) {
            printf("NOVO       %s\n", atual.itens[j]);
            novos++;
            j++;
        } else {
            char completo[TAM_CAMINHO];
            char hex[SHA256_TAM_HEX];
            int n = snprintf(completo, sizeof completo, "%s/%s", pasta, atual.itens[j]);

            if (n < 0 || (size_t)n >= sizeof completo ||
                hash_do_arquivo(completo, hex) != 0) {
                fprintf(stderr, "Erro ao calcular o hash de: %s\n", atual.itens[j]);
                status = -1;
            } else {
                if (strcmp(hex, esperado[i].hash) != 0) {
                    printf("MODIFICADO %s\n", atual.itens[j]);
                    modificados++;
                }
                i++;
                j++;
            }
        }
    }

    if (status == 0) {
        if (modificados + ausentes + novos == 0) {
            printf("Tudo certo: %lu arquivos conferem com o manifesto.\n",
                   (unsigned long)n_esperado);
        } else {
            printf("\nResumo: %lu modificados, %lu ausentes, %lu novos.\n",
                   modificados, ausentes, novos);
            status = 1;
        }
    }

    liberar_entradas(esperado, n_esperado);
    lista_liberar(&atual);
    return status;
}