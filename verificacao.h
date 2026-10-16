#ifndef VERIFICACAO_H
#define VERIFICACAO_H

/* Compara `pasta` com o manifesto em `arquivo_manifesto`. Imprime as
   diferenças. Retorna 0 se tudo igual, 1 se houver diferenças e -1 em erro
   (manifesto ilegível, pasta inacessível), para o chamador distinguir
   "encontrei alteração" de "não consegui verificar". */
int verificar_manifesto(const char *pasta, const char *arquivo_manifesto);

#endif