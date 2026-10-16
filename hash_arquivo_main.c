#include <stdio.h>

#include "arquivo_hash.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo>\n", argv[0]);
        return 2;
    }

    char hex[SHA256_TAM_HEX];
    if (hash_do_arquivo(argv[1], hex) != 0) {
        fprintf(stderr, "Erro ao ler o arquivo: %s\n", argv[1]);
        return 1;
    }

    printf("%s  %s\n", hex, argv[1]);
    return 0;
}