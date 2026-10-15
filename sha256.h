#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_TAM_BLOCO 64
#define SHA256_TAM_HASH 32
#define SHA256_TAM_HEX 65 /* 64 caracteres + o '\0' final */

typedef struct {
    uint32_t estado[8];
    uint64_t total_bits;
    uint8_t buffer[SHA256_TAM_BLOCO];
    size_t tam_buffer;
} Sha256;

void sha256_iniciar(Sha256 *ctx);
void sha256_atualizar(Sha256 *ctx, const uint8_t *dados, size_t tam);
void sha256_finalizar(Sha256 *ctx, uint8_t hash[SHA256_TAM_HASH]);
void sha256_para_hex(const uint8_t hash[SHA256_TAM_HASH], char saida[SHA256_TAM_HEX]);

#endif