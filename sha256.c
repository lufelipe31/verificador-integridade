#include "sha256.h"

#include <stdio.h>
#include <string.h>

/* Constantes do padrão: as 32 primeiras casas dos bits fracionários das
   raízes cúbicas dos 64 primeiros números primos. */
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* Rotação circular à direita. O n nunca é 0 aqui, então o deslocamento
   (32 - n) é sempre válido. */
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sig0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define sig1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static void processar_bloco(Sha256 *ctx, const uint8_t bloco[SHA256_TAM_BLOCO]) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;

    /* O padrão lê os bytes em big-endian, independente da máquina. Por isso
       montamos cada palavra byte a byte, em vez de copiar a memória direto. */
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)bloco[i * 4] << 24) | ((uint32_t)bloco[i * 4 + 1] << 16) |
               ((uint32_t)bloco[i * 4 + 2] << 8) | (uint32_t)bloco[i * 4 + 3];
    }
    for (int i = 16; i < 64; i++) {
        w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
    }

    a = ctx->estado[0]; b = ctx->estado[1]; c = ctx->estado[2]; d = ctx->estado[3];
    e = ctx->estado[4]; f = ctx->estado[5]; g = ctx->estado[6]; h = ctx->estado[7];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + SIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = SIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->estado[0] += a; ctx->estado[1] += b; ctx->estado[2] += c; ctx->estado[3] += d;
    ctx->estado[4] += e; ctx->estado[5] += f; ctx->estado[6] += g; ctx->estado[7] += h;
}

void sha256_iniciar(Sha256 *ctx) {
    /* Valores iniciais do padrão: raízes quadradas dos 8 primeiros primos. */
    ctx->estado[0] = 0x6a09e667; ctx->estado[1] = 0xbb67ae85;
    ctx->estado[2] = 0x3c6ef372; ctx->estado[3] = 0xa54ff53a;
    ctx->estado[4] = 0x510e527f; ctx->estado[5] = 0x9b05688c;
    ctx->estado[6] = 0x1f83d9ab; ctx->estado[7] = 0x5be0cd19;
    ctx->total_bits = 0;
    ctx->tam_buffer = 0;
}

void sha256_atualizar(Sha256 *ctx, const uint8_t *dados, size_t tam) {
    ctx->total_bits += (uint64_t)tam * 8;

    /* Os dados chegam em pedaços de qualquer tamanho, mas o algoritmo só
       processa blocos de 64 bytes. O buffer guarda a sobra entre as chamadas. */
    while (tam > 0) {
        size_t espaco = SHA256_TAM_BLOCO - ctx->tam_buffer;
        size_t copiar = tam < espaco ? tam : espaco;

        memcpy(ctx->buffer + ctx->tam_buffer, dados, copiar);
        ctx->tam_buffer += copiar;
        dados += copiar;
        tam -= copiar;

        if (ctx->tam_buffer == SHA256_TAM_BLOCO) {
            processar_bloco(ctx, ctx->buffer);
            ctx->tam_buffer = 0;
        }
    }
}

void sha256_finalizar(Sha256 *ctx, uint8_t hash[SHA256_TAM_HASH]) {
    uint64_t bits = ctx->total_bits;

    /* Padding: um bit 1 (0x80), zeros, e o tamanho original em bits nos
       últimos 8 bytes. O tamanho precisa caber no fim de um bloco; se não
       couber, o padding ocupa um bloco extra. */
    ctx->buffer[ctx->tam_buffer++] = 0x80;

    if (ctx->tam_buffer > 56) {
        memset(ctx->buffer + ctx->tam_buffer, 0, SHA256_TAM_BLOCO - ctx->tam_buffer);
        processar_bloco(ctx, ctx->buffer);
        ctx->tam_buffer = 0;
    }
    memset(ctx->buffer + ctx->tam_buffer, 0, 56 - ctx->tam_buffer);

    for (int i = 0; i < 8; i++) {
        ctx->buffer[63 - i] = (uint8_t)(bits >> (i * 8));
    }
    processar_bloco(ctx, ctx->buffer);

    for (int i = 0; i < 8; i++) {
        hash[i * 4] = (uint8_t)(ctx->estado[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(ctx->estado[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(ctx->estado[i] >> 8);
        hash[i * 4 + 3] = (uint8_t)(ctx->estado[i]);
    }
}

void sha256_para_hex(const uint8_t hash[SHA256_TAM_HASH], char saida[SHA256_TAM_HEX]) {
    for (int i = 0; i < SHA256_TAM_HASH; i++) {
        sprintf(saida + i * 2, "%02x", hash[i]);
    }
    saida[SHA256_TAM_HEX - 1] = '\0';
}