#ifndef MANIFESTO_H
#define MANIFESTO_H

/* Percorre `pasta`, calcula o SHA-256 de cada arquivo e grava o manifesto
   em `saida`. Retorna 0 em sucesso e -1 em erro (nesse caso nenhum
   manifesto parcial fica no disco). */
int gerar_manifesto(const char *pasta, const char *saida);

#endif