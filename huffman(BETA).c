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

void frequencia_bytes(FILE *file, unsigned int tab[])
{
    unsigned char byte;
    while(fread(&byte, sizeof(unsigned char), 1, file))
        tab[byte]++;
    
    rewind(file);
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

void escrever_arvore_header(No *raiz, FILE *arquivo)
{
    if(raiz->esquerda == NULL && raiz->direita == NULL)
    {
        if(raiz->caracter == '\\' || raiz->caracter == '*'){

            unsigned char scape = '\\';
            fprintf(arquivo, "%c", scape);
        }

        fprintf(arquivo, "%c", raiz->caracter);
        return;
    }

    fprintf(arquivo, "%c", raiz->caracter);

    if(raiz->esquerda != NULL)
        escrever_arvore_header(raiz->esquerda, arquivo);
    if(raiz->direita != NULL)
        escrever_arvore_header(raiz->direita, arquivo);
}

short calcula_tamanho_arvore(No* raiz)
{
    if (raiz == NULL)
        return 0;

    return 1 + calcula_tamanho_arvore(raiz->esquerda) + calcula_tamanho_arvore(raiz->direita);
}

void escrever_header(FILE *arquivo, short lixo, unsigned int tamanho_arvore)
{
    rewind(arquivo);
    unsigned char byte = lixo;

    byte <<= 5; // byte = byte << 5
    byte |= tamanho_arvore >> 8;

    unsigned char byte2;
    byte2 = tamanho_arvore;

    fwrite(&byte, sizeof(byte), 1, arquivo);   
    fwrite(&byte2, sizeof(byte), 1, arquivo);

}

short compactar(char *compactado, FILE *saida)
{

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
        return (j != 7) ? 8 - j : 0;
    }
    else
        printf("\nErro na função compactar().");
    
}

unsigned int is_bit_i_set(unsigned char byte, int i)
{
    unsigned char mask = (1 << i);
    return byte & mask;
}

void ler_header(FILE *arquivo, unsigned char *lixo, unsigned short *tamanho_arvore)
{

    unsigned char buffer; // Le o primeiro byte
    fread(&buffer, sizeof(char), 1, arquivo);

    unsigned short header = buffer << 8; // prepara o header para possuir os 2 primeiros bytes
    fread(&buffer, sizeof(char), 1, arquivo);

    header |= buffer;

    *lixo = header >> 13;
    *tamanho_arvore = header & 0x1FFF;
}

No* criar_no(unsigned char byte, No *esquerda, No *direita)
{
    No *tree = malloc(sizeof(No));
    tree->caracter = byte;
    tree->esquerda = esquerda;
    tree->direita = direita;

    return tree;
}

No* ler_arvore_header(FILE *arquivo, unsigned short *tamanho_arvore)
{
    unsigned char byte;
    fread(&byte, sizeof(unsigned char), 1, arquivo);
    bool is_leaf = false;

    if(*tamanho_arvore == 0)
        return NULL;

    (*tamanho_arvore)--;

    if (byte == '\\') // scape character
    {
        (*tamanho_arvore)--;
        fread(&byte, sizeof(unsigned char), 1, arquivo);
        is_leaf = true;
    }
    if (byte != '*') // é uma folha
        is_leaf = true;
    if (is_leaf) // se é uma folha, cria um nó com o byte
        return criar_no(byte, NULL, NULL);

    No *left = ler_arvore_header(arquivo, tamanho_arvore);
    No *right = ler_arvore_header(arquivo, tamanho_arvore);

    return criar_no('*', left, right);
}

void descompactar(No *raiz, FILE *entrada, FILE *saida)
{
    No *aux = raiz;
    unsigned char byte;
    
    if (!entrada || !saida)
    {
        printf("\nErro ao abrir arquivo em descompactar()\n");
        return;
    }
    else
    {
        while(fscanf(entrada, "%c", &byte) != EOF)
        {
            for (int i = 7; i >= 0; i--) // Iniciar de 7 (bit mais significativo) para 0
            {
                if(is_bit_i_set(byte, i))
                    aux = aux->direita;
                else
                    aux = aux->esquerda;

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

void comprimir(FILE *entrada, FILE *saida);

void descomprimir(FILE *entrada, FILE *saida);

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s <mode> <input file> <output file>\n", argv[0]);
        return 1;
    }

    FILE *entrada = fopen(argv[2], "rb");
    FILE *saida = fopen(argv[3], "wb");

    if(entrada == NULL)
    {
        printf("Erro ao encontrar o arquivo de entrada.\n");
        return 2;
    }
    if(saida == NULL)
    {
        printf("Erro ao criar o arquivo de saida.\n");
        return 3;
    }


    if (argv[1][1] == 'c')
        comprimir(entrada, saida);
    else if (argv[1][1] == 'd')
        descomprimir(entrada, saida);
    else
    {
        printf("opcao invalida.\n");
        return 4;
    }

    return 0;
}


void comprimir(FILE *entrada, FILE *saida)
{
    unsigned int tabela_frequencia[TAM] = {0};
    frequencia_bytes(entrada, tabela_frequencia);
    
    Lista lista;
    criar_lista(&lista);
    preencher_lista(tabela_frequencia, &lista);

    No *arvore = montar_arvore(&lista);

    int colunas = altura_arvore(arvore) + 1;
    char **dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, arvore, "", colunas);
    
    char *codificado = codificar(dicionario, entrada);

    int tamanho_arvore = calcula_tamanho_arvore(arvore);
    short lixo = compactar(codificado, saida);
    escrever_header(saida, lixo, tamanho_arvore);
    escrever_arvore_header(arvore, saida);

    printf("\nARQUIVO COMPRIMIDO!\n");

    free(codificado);
    liberar_arvore(arvore);
    liberar_dicionario(dicionario);
    return;
}

void descomprimir(FILE *entrada, FILE *saida)
{
    unsigned short tamanho_arvore = 0;
    unsigned char lixo = 0;
    ler_header(entrada, &lixo, &tamanho_arvore);
    No *arvore = ler_arvore_header(entrada, &tamanho_arvore);
    descompactar(arvore, entrada, saida);

    printf("\nARQUIVO DESCOMPRIMIDO!\n");

    liberar_arvore(arvore);
}