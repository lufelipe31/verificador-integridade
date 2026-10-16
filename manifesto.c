#include "manifesto.h"

#include <stdio.h>

#include "arquivo_hash.h"
#include "lista.h"

#define TAM_CAMINHO 4096

int gerar_manifesto(const char *pasta, const char *saida) {
    Lista lista;
    lista_iniciar(&lista);

    if (listar_arquivos(pasta, &lista) != 0) {
        lista_liberar(&lista);
        return -1;
    }

    /* Ordem fixa: o mesmo conteúdo gera sempre o mesmo manifesto. */
    lista_ordenar(&lista);

    /* "wb": no Windows o modo texto gravaria \r\n. Queremos só \n, para o
       manifesto ser igual em qualquer sistema. */
    FILE *f = fopen(saida, "wb");
    if (f == NULL) {
        fprintf(stderr, "Nao foi possivel criar: %s\n", saida);
        lista_liberar(&lista);
        return -1;
    }

    int status = 0;
    for (size_t i = 0; i < lista.qtd; i++) {
        char completo[TAM_CAMINHO];
        char hex[SHA256_TAM_HEX];

        snprintf(completo, sizeof completo, "%s/%s", pasta, lista.itens[i]);

        if (hash_do_arquivo(completo, hex) != 0) {
            fprintf(stderr, "Erro ao calcular o hash de: %s\n", completo);
            status = -1;
            break;
        }
        fprintf(f, "%s  %s\n", hex, lista.itens[i]);
    }

    if (fclose(f) != 0) {
        status = -1;
    }

    if (status != 0) {
        remove(saida); /* não deixa um manifesto incompleto no disco */
    } else {
               printf("Manifesto gerado: %s (%lu arquivos)\n", saida, (unsigned long)lista.qtd);
    }

    lista_liberar(&lista);
    return status;
}