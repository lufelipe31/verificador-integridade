#include <stdio.h>
#include <string.h>

#include "manifesto.h"

static void uso(const char *programa) {
    fprintf(stderr,
            "Uso:\n"
            "  %s gerar <pasta> <manifesto>\n",
            programa);
}

int main(int argc, char *argv[]) {
    if (argc == 4 && strcmp(argv[1], "gerar") == 0) {
        return gerar_manifesto(argv[2], argv[3]) == 0 ? 0 : 1;
    }

    uso(argv[0]);
    return 2;
}