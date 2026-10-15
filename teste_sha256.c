#include <stdio.h>
#include <string.h>

#include "sha256.h"

static int falhas = 0;

static void conferir(const char *nome, const char *hex_obtido, const char *esperado) {
    if (strcmp(hex_obtido, esperado) == 0) {
        printf("OK     %s\n", nome);
    } else {
        printf("FALHOU %s\n  esperado: %s\n  obtido:   %s\n", nome, esperado, hex_obtido);
        falhas++;
    }
}

static void hash_texto(const char *texto, char hex[SHA256_TAM_HEX]) {
    Sha256 ctx;
    uint8_t hash[SHA256_TAM_HASH];

    sha256_iniciar(&ctx);
    sha256_atualizar(&ctx, (const uint8_t *)texto, strlen(texto));
    sha256_finalizar(&ctx, hash);
    sha256_para_hex(hash, hex);
}

int main(void) {
    char hex[SHA256_TAM_HEX];

    /* Vetores oficiais do NIST (FIPS 180-4). */
    hash_texto("", hex);
    conferir("texto vazio", hex,
             "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    hash_texto("abc", hex);
    conferir("abc", hex,
             "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    /* 56 bytes: força o padding a ocupar um bloco extra. */
    hash_texto("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", hex);
    conferir("56 bytes (padding em bloco extra)", hex,
             "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    /* Um milhão de 'a' em pedaços de 1000 bytes: testa muitas chamadas de
       atualizar com o buffer parcial. */
    {
        Sha256 ctx;
        uint8_t hash[SHA256_TAM_HASH];
        uint8_t pedaco[1000];

        memset(pedaco, 'a', sizeof(pedaco));
        sha256_iniciar(&ctx);
        for (int i = 0; i < 1000; i++) {
            sha256_atualizar(&ctx, pedaco, sizeof(pedaco));
        }
        sha256_finalizar(&ctx, hash);
        sha256_para_hex(hash, hex);
        conferir("um milhao de 'a'", hex,
                 "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    }

    printf("\n%s\n", falhas == 0 ? "Todos os testes passaram." : "Houve falhas.");
    return falhas == 0 ? 0 : 1;
}