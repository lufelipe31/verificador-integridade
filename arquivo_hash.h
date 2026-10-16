#ifndef ARQUIVO_HASH_H
#define ARQUIVO_HASH_H

#include "sha256.h"

/* Calcula o SHA-256 do arquivo em `caminho` e escreve os 64 caracteres
   hexadecimais (mais o '\0') em `hex`. Retorna 0 em sucesso e -1 em erro
   (arquivo inexistente, sem permissão ou falha de leitura). */
int hash_do_arquivo(const char *caminho, char hex[SHA256_TAM_HEX]);

#endif