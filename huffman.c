#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAM 256

typedef struct no{
    unsigned char caracter;
    int frequencia;
    struct no *esquerda, *direita, *proximo;
}No;

typedef struct lista{
    No *inicio;
    int tamanho;
}Lista;

void criar_lista(Lista *lista)
{
    lista->inicio = NULL;
    lista->tamanho = 0;
}

bool lista_vazia(Lista *lista)
{
    return (lista->inicio == NULL);
}

void inserir_ordenado(Lista *lista, No *no)
{
    No *aux;

    if (lista->inicio == NULL)
    {
        lista->inicio = no;
    }

    else if (no->frequencia < lista->inicio->frequencia)
    {
        no->proximo = lista->inicio; 
        lista->inicio = no;
    }

    else
    {
        aux = lista->inicio;

        while(aux->proximo && aux->proximo->frequencia <= no->frequencia)
        {
            aux = aux->proximo;
        }

        no->proximo = aux->proximo;
        aux->proximo = no;
    }

    lista->tamanho++;
}

void preencher_lista(unsigned int *tabela_frequencia, Lista *lista)
{
    No *novo;

    for (int i = 0; i < TAM; i++)
    {
        if (*(tabela_frequencia + i) > 0)
        {
            novo = malloc(sizeof(No));

            if (novo)
            {
                novo->caracter = i;
                novo->frequencia = *(tabela_frequencia + i);
                novo->direita = NULL;
                novo->esquerda = NULL;
                novo->proximo = NULL;

                inserir_ordenado(lista, novo);
            }

            else
            {
                printf("\tERRO ao alocar memória em preencher_lista()\n");
            }
        }    
    }
}

No* remover_inicio(Lista *lista)
{
    if (lista_vazia(lista))
        return NULL;

    No *aux = lista->inicio;
    lista->inicio = aux->proximo;
    lista->tamanho--;
    aux->proximo = NULL;
    
    return aux;  
}

No* montar_arvore(Lista *lista)
{
    No *primeiro, *segundo, *novo;
    while (lista->tamanho > 1)
    {
        primeiro = remover_inicio(lista);
        segundo = remover_inicio(lista);

        novo = malloc(sizeof(No));
        if (novo == NULL)
        {
            puts("Erro ao alocar memoria em montar_arvore()");
            break;
        }

        novo->caracter = '*';
        novo->frequencia = primeiro->frequencia + segundo->frequencia;
        
        novo->esquerda = primeiro;
        novo->direita = segundo;
        novo->proximo = NULL;

        inserir_ordenado(lista, novo);
    }
    
    return lista->inicio;
}

int altura_arvore(No *raiz)
{
    if(raiz == NULL)
        return -1;
    else
    {
        int esq = altura_arvore(raiz->esquerda) + 1;
        int dir = altura_arvore(raiz->direita) + 1;

        if(esq > dir)
            return esq;
        else
            return dir;
    }
}

char** aloca_dicionario(int colunas)
{
    char **dicionario;

    dicionario = malloc(sizeof(char *) * TAM);

    for (int i = 0; i < TAM; i++)
        dicionario[i] = calloc(colunas, sizeof(char));
    
    return dicionario;
}

void gerar_dicionario(char **dicionario, No *raiz, char *caminho, int colunas)
{
    char esquerda[colunas], direita[colunas];
    
    if(raiz->esquerda == NULL && raiz->direita == NULL)
        strcpy(dicionario[raiz->caracter], caminho);
    else
    {
        //salvando caminho atual
        strcpy(esquerda, caminho);
        strcpy(direita, caminho);

        //concatenando 0 e 1 para os caminhos
        strcat(esquerda, "0");
        strcat(direita, "1");

        //descendo a árvore com os novos caminhos
        gerar_dicionario(dicionario, raiz->esquerda, esquerda, colunas);
        gerar_dicionario(dicionario, raiz->direita, direita, colunas);
    }
    
}

void imprimir_dicionario(char **dicionario)
{
    for (int i = 0; i < TAM; i++)
        if(strlen(dicionario[i]) > 0)
            printf("\t%3c: %s\n", i, dicionario[i]);
}

int calcula_tamanho_texto(char **dicionario, FILE* arquivo)
{
    int tam = 0;

    unsigned char byte;

    while (fread(&byte, sizeof(byte), 1, arquivo))
        tam = tam + strlen(dicionario[byte]);

    return tam;
}

char* codificar(char **dicionario, FILE* arquivo)
{
    unsigned char byte;

    int tam = calcula_tamanho_texto(dicionario, arquivo);

    char *codigo = calloc(tam, sizeof(char));

    rewind(arquivo);

    while (fread(&byte, sizeof(byte), 1, arquivo))
        strcat(codigo, dicionario[byte]);
    
    return codigo;
}

char* decodificar(char *codificado, No *raiz)
{
    No *aux = raiz;

    char temp[2];

    char *decodificado = calloc(strlen(codificado), sizeof(char));

    int i = 0;

    while (codificado[i] != '\0')
    {
        if (codificado[i] == '0')
            aux = aux->esquerda;
        else
            aux = aux->direita;

        if(aux->esquerda == NULL && aux->direita == NULL)
        {
            temp[0] = aux->caracter;
            temp[1] = '\0';
            strcat(decodificado, temp);
            aux = raiz;
        }

        i++;
    }
    
    return decodificado;
}

void compactar(char *compactado)
{
    FILE* saida = fopen("compactado.huff", "wb");

    int i = 0, j = 7;

    unsigned char mask, byte = 0;

    if (saida)
    {
        while (compactado[i] != '\0')
        {
            mask = 1;

            if(compactado[i] == '1')
            {
                mask = mask << j;
                byte = byte | mask;
            }
            j--;

            if (j < 0)
            {
                fwrite(&byte, sizeof(unsigned char), 1, saida);
                byte = 0;
                j = 7;
            }

            i++;
        }
        
        if (j != 7)
            fwrite(&byte, sizeof(unsigned char), 1, saida);
        
        fclose(saida);
    }

    else
    {
        printf("\nErro na função compactar().");
    }
}

unsigned int is_bit_i_set(unsigned char byte, int i)
{
    unsigned char mask = (1 << i);
    return byte & mask;
}

void descompactar(No *raiz)
{
    FILE* entrada= fopen("compactado.huff", "rb");
    FILE* saida = fopen("descompactado.huff", "wb");

    No *aux = raiz;
    unsigned char byte;
    
    if (!entrada || !saida)
    {
        printf("\nErro ao abrir arquivo em descompactar()\n");
        return;
    }

    else
    {
        while (fread(&byte, sizeof(unsigned char), 1, entrada))
        {
            for (int i = 7; i >= 0; i--) // Iniciar de 7 (bit mais significativo) para 0
            {
                if(is_bit_i_set(byte, i))
                {
                    aux = aux->direita;
                }
                else
                {
                    aux = aux->esquerda;
                }

                if (aux->esquerda == NULL && aux->direita == NULL)
                {
                    fputc(aux->caracter, saida);
                    aux = raiz; // Reiniciar a busca a partir da raiz
                }
            }
        }
        fclose(entrada);        
        fclose(saida);
    }
}

void liberar_dicionario(char **dicionario)
{
    for (int i = 0; i < TAM; i++)
    {
        free(dicionario[i]);
    }
    free(dicionario);
}

void liberar_arvore(No *raiz)
{
    if (raiz == NULL)
        return;

    liberar_arvore(raiz->esquerda);
    liberar_arvore(raiz->direita);

    free(raiz);
}

int main()
{
    FILE* arquivo;

    char nome_arquivo[TAM];

    scanf("%s", nome_arquivo);

    arquivo = fopen(nome_arquivo, "rb");

    unsigned char byte;
    unsigned int tabela_frequencia[TAM] = {0};
    char **dicionario;
    char *codificado, *decodificado;
    
    Lista lista;

    No *arvore;

    if (arquivo)
    {
        while (fread(&byte, sizeof(byte), 1, arquivo))
            tabela_frequencia[byte]++;
        
        fclose(arquivo);
    }
    else
        printf("Erro ao abrir o arquivo.\n");


    // CRIAÇÃO LISTA DE FREQUENCIA
    criar_lista(&lista);
    preencher_lista(tabela_frequencia, &lista);

    // MONTAGEM DA ÁRVORE DE HUFFMAN
    arvore = montar_arvore(&lista);

    // CRIAÇÃO DO DICIONÁRIO BASEADO NA ÁRVORE
    int colunas = altura_arvore(arvore) + 1;
    dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, arvore, "", colunas);

    // CODIFICAÇÃO
    arquivo = fopen(nome_arquivo, "rb");
    codificado = codificar(dicionario, arquivo);
    fclose(arquivo);

    // DECODIFICAÇÃO
    decodificado = decodificar(codificado, arvore);

    // COMPACTAÇÃO
    compactar(codificado);

    // DESCOMPACTAÇÃO
    printf("\nARQUIVO DESCOMPACTADO!\n");
    descompactar(arvore);
    printf("\n\n");


    // LIBERAÇÃO DE MEMÓRIA ALOCADA DINAMICAMENTE
    free(codificado);
    free(decodificado);
    liberar_arvore(arvore);
    liberar_dicionario(dicionario);
    return 0;
}
