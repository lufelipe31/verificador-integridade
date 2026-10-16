#include "arquivo_hash.h"

#include <stdio.h>

#define TAM_LEITURA 65536

int hash_do_arquivo(const char *caminho, char hex[SHA256_TAM_HEX]) {
    /* "rb" (binário) é essencial no Windows: no modo texto, o C converte
       quebras de linha e pára em certos bytes, o que corromperia o hash. */
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) {
        return -1;
    }

    Sha256 ctx;
    uint8_t hash[SHA256_TAM_HASH];
    /* static: 64 KB na pilha seria desperdício; assim fica na área de dados. */
    static uint8_t bloco[TAM_LEITURA];
    size_t lidos;

    sha256_iniciar(&ctx);
    while ((lidos = fread(bloco, 1, TAM_LEITURA, f)) > 0) {
        sha256_atualizar(&ctx, bloco, lidos);
    }

    /* fread devolve 0 tanto no fim do arquivo quanto em erro de leitura.
       Só ferror diferencia os dois, e um erro não pode virar um hash falso. */
    if (ferror(f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    sha256_finalizar(&ctx, hash);
    sha256_para_hex(hash, hex);
    return 0;
}