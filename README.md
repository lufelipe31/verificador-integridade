# verificador-integridade

Programa de linha de comando em C que verifica a integridade de uma pasta. Ele gera um manifesto com o hash SHA-256 de cada arquivo e, depois, compara a pasta com o manifesto para apontar o que foi modificado, apagado ou adicionado. É a mesma ideia de ferramentas como o Tripwire.

O SHA-256 foi implementado do zero, seguindo o padrão FIPS 180-4, e testado com os vetores oficiais do NIST.

## Funcionalidades

- `gerar`: percorre a pasta (inclusive subpastas) e grava o manifesto, uma linha por arquivo, em ordem alfabética
- `verificar`: compara a pasta com o manifesto e classifica cada diferença
- Hash lido em blocos de 64 KB, então a memória usada não depende do tamanho do arquivo
- Manifesto no formato do `sha256sum` (hash, dois espaços, caminho relativo)
- Códigos de saída que distinguem "achei alteração" de "não consegui verificar", para uso em scripts

## Tipos de diferença

| Tipo | Significado |
|------|-------------|
| MODIFICADO | O arquivo existe, mas o hash mudou |
| AUSENTE | Está no manifesto, mas não está mais na pasta |
| NOVO | Está na pasta, mas não estava no manifesto |

## Tecnologias

- C (padrão GNU11)
- Nenhuma biblioteca externa: SHA-256 próprio e funções padrão do sistema

## Como compilar

Pré-requisito: um compilador GCC (testado no Windows com MinGW GCC 6.3.0).

```powershell
gcc -Wall -Wextra -std=gnu11 -o verificador sha256.c arquivo_hash.c lista.c manifesto.c verificacao.c main.c
```

## Como usar

Gerar o manifesto de uma pasta:

```powershell
.\verificador.exe gerar minha_pasta manifesto.txt
```

Verificar a pasta depois:

```powershell
.\verificador.exe verificar minha_pasta manifesto.txt
```

Exemplo de saída quando algo mudou:

```
MODIFICADO a.txt
AUSENTE    docs/meu arquivo.txt
NOVO       novo.txt

Resumo: 1 modificados, 1 ausentes, 1 novos.
```

### Códigos de saída

| Código | Significado |
|--------|-------------|
| 0 | Tudo confere (ou manifesto gerado com sucesso) |
| 1 | `verificar` encontrou diferenças |
| 2 | Uso incorreto dos argumentos |
| 3 | Erro (pasta ou manifesto ilegível, arquivo sem permissão) |

## Testes

O SHA-256 é testado com os vetores oficiais do NIST: texto vazio, `abc`, a mensagem de 56 bytes (que força o padding em um bloco extra) e um milhão de `a` em pedaços.

```powershell
gcc -Wall -Wextra -std=c11 -o teste_sha256 sha256.c teste_sha256.c
.\teste_sha256.exe
```

Também comparei o hash de arquivos reais, pequenos e de 5 MB, com o `Get-FileHash` do Windows, e os resultados foram idênticos.

## Estrutura

```
verificador-integridade/
├── sha256.h / sha256.c               # SHA-256 do zero
├── arquivo_hash.h / arquivo_hash.c   # hash de um arquivo, em blocos
├── lista.h / lista.c                 # lista dinâmica e percurso de pastas
├── manifesto.h / manifesto.c         # comando gerar
├── verificacao.h / verificacao.c     # comando verificar
├── main.c                            # linha de comando
├── hash_arquivo_main.c               # utilitário: hash de um arquivo
└── teste_sha256.c                    # testes com vetores do NIST
```

## Decisões

- Os caminhos ficam ordenados no manifesto e a comparação é feita como um merge de duas listas ordenadas, em uma única passada.
- Qualquer erro de leitura interrompe o programa e apaga o manifesto parcial. Um manifesto incompleto daria uma falsa sensação de segurança.
- Uma linha fora do formato no manifesto é erro, não é ignorada.
- Arquivos abertos em modo binário, porque o modo texto do Windows alteraria os bytes lidos.
- Caminhos montados com `snprintf` e checagem de tamanho, para evitar estouro de buffer.

## Limitações

- O manifesto não é assinado. Quem consegue alterar a pasta e o manifesto ao mesmo tempo engana a ferramenta. Em uso real, o manifesto deve ficar em local protegido ou ser assinado.
- Grave o manifesto fora da pasta analisada. Se ficar dentro, ele entra no próprio manifesto na próxima geração.
- O SHA-256 próprio serve para aprendizado. Em produção, o recomendado é uma biblioteca auditada, como OpenSSL ou libsodium.

## Melhorias futuras

- Assinatura do manifesto
- Opção para ignorar arquivos ou pastas
- Testes automatizados dos comandos `gerar` e `verificar`
- Execução agendada, usando os códigos de saída