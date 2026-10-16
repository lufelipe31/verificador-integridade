#include <stdio.h>
#include <string.h>

#include "manifesto.h"
#include "verificacao.h"

static void uso(const char *programa) {
    fprintf(stderr,
            "Uso:\n"
            "  %s gerar <pasta> <manifesto>\n"
            "  %s verificar <pasta> <manifesto>\n"
            "\n"
            "Codigos de saida de 'verificar': 0 tudo igual, 1 diferencas, 2 uso incorreto, 3 erro.\n",
            programa, programa);
}

int main(int argc, char *argv[]) {
    if (argc == 4 && strcmp(argv[1], "gerar") == 0) {
        return gerar_manifesto(argv[2], argv[3]) == 0 ? 0 : 3;
    }

    if (argc == 4 && strcmp(argv[1], "verificar") == 0) {
        int r = verificar_manifesto(argv[2], argv[3]);
        return r < 0 ? 3 : r;
    }

    uso(argv[0]);
    return 2;
}