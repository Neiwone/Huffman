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

/*
    Função: 
        Lê o arquivo e enquanto adiciona a frequencia de cada byte presente no arquivo.

    Argumentos:
        FILE *arquivo -> É o arquivo que será compactado e de onde iremos ler os bytes;
        unsigned int tab[] -> É a nossa tabela de frequencia dos bytes.

    Retorno:
        A função não possui retorno, porém ao final de ler o arquivo retornamos o ponteiro
        do arquivo para o início.
*/

void frequencia_bytes(FILE *arquivo, unsigned int tab[])
{
    unsigned char byte;
    while(fread(&byte, sizeof(unsigned char), 1, arquivo))
        tab[byte]++;
    
    rewind(arquivo); //retornar o ponteiro do arquivo para o início
}

/*
    Função: 
        Inicializa a lista.

    Argumentos:
        Lista *lista -> Lista a ser inicializada.

    Retorno:
        A função não possui retorno.
*/

void criar_lista(Lista *lista)
{
    lista->inicio = NULL;
    lista->tamanho = 0;
}

/*
    Função: 
        Insere novos nós de maneira ordenada de forma que os bytes menos recorentes fiquem no início da fila.

    Argumentos:
        Lista *lista -> É a nossa lista com os nós correspondentes aos bytes e suas frequencias;
        No *no -> É o nó que desejamos que seja inserido na lista;
        (LEMBRETE: Em um primeiro momento, nós estão sendo usados apenas como nós de uma lista.
        E posteriomente essa função será usada para inserção de nós da árvore também).

    Retorno:
        A função não possui retorno.
*/

void inserir(Lista *lista, No *no)
{
    No *aux;

    if (lista->inicio == NULL)
        lista->inicio = no;
    else if (no->frequencia < lista->inicio->frequencia)
    {
        no->proximo = lista->inicio; 
        lista->inicio = no;
    }
    else
    {
        aux = lista->inicio;

        while(aux->proximo && aux->proximo->frequencia <= no->frequencia)
            aux = aux->proximo;

        no->proximo = aux->proximo;
        aux->proximo = no;
    }

    lista->tamanho++;
}

/*
    Função: 
        Preenche a lista de prioriadade de acordo com cada ocorencia na nossa tabela de frequencia de bytes.

    Argumentos:
        unsigned int tab[] -> É a nossa tabela de frequencia dos bytes;
        Lista *lista -> É a nossa lista com os nós correspondentes aos bytes e suas frequencias.

    Retorno:
        A função não possui retorno.
*/

void preencher_lista(unsigned int tab[], Lista *lista)
{
    No *novo;

    for (int i = 0; i < TAM; i++)
    {
        if (tab[i] > 0)
        {
            novo = malloc(sizeof(No));

            if (novo)
            {
                novo->caracter = i;
                novo->frequencia = tab[i];
                novo->direita = NULL;
                novo->esquerda = NULL;
                novo->proximo = NULL;

                inserir(lista, novo);
            }
            else
                printf("\tErro ao alocar memória em preencher_lista()\n");
        }    
    }
}

/*
    Função: 
        Remove um nó que será usado posteriomente para criar nossa Árvore de Huffman.

    Argumentos:
        Lista *lista -> É a nossa lista de onde iramos remover o nó inicial.

    Retorno:
        Retorna o nó removido.
*/

No* remover_inicio(Lista *lista)
{
    if (lista->inicio == NULL)
        return NULL;

    No *aux = lista->inicio;
    lista->inicio = aux->proximo;
    lista->tamanho--;
    aux->proximo = NULL;
    
    return aux;  
}

/*
    Função: 
        Monta Árvore de Huffman, removendo sempre os dois primeiros nós da lista e criando um nó com
        o caracter '*' e com a soma das frequencias dos nós e colocando como filhos desse nó, os dois
        nós removidos da lista, e por fim inserindo esse novo nó na lista, até que haja apenas um nó
        na lista(a raiz da nossa árvore).

    Argumentos:
        Lista *lista -> É a nossa lista de onde iramos remover os nós para ir montando a árvore.

    Retorno:
        Retorna a raiz da nossa Árvore de Huffman.
*/

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

        inserir(lista, novo);
    }
    
    return lista->inicio;
}

/*
    Função: 
        Calcula a altura da árvore, para definir o tamanho das colunas do dicionário.

    Argumentos:
        No *raiz -> É a raiz da árvore.

    Retorno:
        Retorna a altura da nossa Árvore de Huffman.
*/

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

/*
    Função: 
        Aloca memória para o dicionário.

    Argumentos:
        int colunas -> Quantidade de colunas do dicionário.

    Retorno:
        Retorna um ponteiro para a região de memória alocada para o dicionário.
*/

char** aloca_dicionario(int colunas)
{
    char **dicionario;

    dicionario = malloc(sizeof(char *) * TAM);

    for (int i = 0; i < TAM; i++)
        dicionario[i] = calloc(colunas, sizeof(char));
    
    return dicionario;
}

/*
    Função: 
        Define o dicionário baseado na Árvore de Huffman criada pelas frequencias de bytes do arquivo
        a ser comprimido. 

    Argumentos:
        char **dicionario -> Dicionário de bits;
        No *raiz -> Raiz da Árvore de Huffman
        char *caminho -> variável necessária para salvar o caminho atual até chegar em uma folha.
        int colunas -> Quantidade máxima de um caminho.

    Retorno:
        A função não possui retorno.
*/

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

/*
    Função: 
        Calcula o tamanho que deve ser alocado para a string de bits codificada.

    Argumentos:
        char **dicionario -> Dicionário de bits;
        FILE* arquivo -> Arquivo que será comprimido.

    Retorno:
        Faz o ponteiro do arquivo voltar para o início e retorna o tamanho do texto.
*/

int calcula_tamanho_texto(char **dicionario, FILE* arquivo)
{
    int tam = 0;

    unsigned char byte;

    while (fread(&byte, sizeof(byte), 1, arquivo))
        tam = tam + strlen(dicionario[byte]);

    rewind(arquivo);
    return tam;
}

/*
    Função: 
        Cria uma string com os bytes traduzidos pelo dicionário.

    Argumentos:
        char **dicionario -> Dicionário de bits;
        FILE* arquivo -> Arquivo que será comprimido.

    Retorno:
        Retorna a string codificada de acordo com o dicionário.
*/

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

/*
    Função: 
        Escreve a Árvore de Huffman no arquivo de saida.

    Argumentos:
        No *raiz -> Ponteiro para a raiz da Árvore de Huffman;
        FILE *saida -> Arquivo comprimido.

    Retorno:
        Retorna o tamanho da árvore (também será escrita no arquivo ).
*/

short escrever_arvore_header(No *raiz, FILE *saida)
{
    if (raiz == NULL)
        return 0;

    int scape = 0;
    if((raiz->caracter == '*' || raiz->caracter == '\\') && raiz->esquerda == NULL && raiz->direita == NULL)
    {
        fwrite("\\", sizeof(unsigned char), 1, saida); // caracter de escape
        scape = 1; // +1 no tamanho caso tenhamos um caracter de escape.
    }

    fwrite(&raiz->caracter, sizeof(unsigned char), 1, saida);
    int esquerda = escrever_arvore_header(raiz->esquerda, saida);
    int direita = escrever_arvore_header(raiz->direita, saida);

    return 1 + esquerda + direita + scape;
}

/*
    Função: 
        Escreve os bits comprimidos no arquivo de saida.

    Argumentos:
        FILE *saida -> Arquivo comprimido;
        char *codigo_codificado -> Codigo de bits que será escrito no arquivo;
        int tamanho_arvore -> Tamanho da árvore (pularemos esse tamanho e escreveremos após isso).

    Retorno:
        Retorna o tamanho do lixo (também será escrita no arquivo).
*/

int escrever_bits(FILE *saida, char *codigo_codificado, int tamanho_arvore)
{

    fseek(saida, tamanho_arvore + 2, SEEK_SET); // pular o header

    int i = 0, j = 7;

    unsigned char mask, byte = 0;

    if (saida)
    {
        for(i = 0; codigo_codificado[i] != '\0'; i++)
        {
            mask = 1;

            if(codigo_codificado[i] == '1')
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

        }
        
        if (j != 7)
            fwrite(&byte, sizeof(unsigned char), 1, saida);

        rewind(saida);
        return (j != 7) ? 8 - j : 0; // tamanho do lixo
    }
    else
        printf("\nErro na função compactar().");
    return 0;
}

/*
    Função: 
        Escreve os bits do header no início do arquivo de saida.

    Argumentos:
        FILE *saida -> Arquivo comprimido;
        unsigned short lixo -> Tamanho do lixo no final do arquivo comprimido;
        unsigned short tamanho_arvore -> Tamanho da árvore.

    Retorno:
        A função não possui retorno.
*/

void escrever_header(FILE *saida, unsigned short lixo, unsigned short tamanho_arvore)
{
    // unsigned short tem 16bits, portanto daremos um shift left de 13 pois vamos escrever apenas 3bits.
    lixo <<= 13;
    // nosso header recebe um OU bit a bit com o tamanho do lixo e o tamnanho da árvore, teremos 16bits.
    short header = lixo | tamanho_arvore;
    // daremos um shift right para pegar os 8 bits mais significativos e escrever eles primeiro.
    unsigned char byte = header >> 8;
    fwrite(&byte, sizeof(unsigned char), 1, saida);
    byte = header;
    fwrite(&byte, sizeof(unsigned char), 1, saida);
}

/*
    Função: 
        Lê os bits do header no início do arquivo.

    Argumentos:
        FILE *entrada -> Arquivo comprimido;
        unsigned short *lixo -> Ponteiro para o tamanho do lixo;
        unsigned short *tamanho_arvore -> Ponteiro para o tamanho da árvore.

    Retorno:
        A função não possui retorno.
*/

void ler_header(FILE *entrada, unsigned char *lixo, unsigned short *tamanho_arvore)
{

    unsigned char byte;

    fread(&byte, sizeof(char), 1, entrada); // primeiro byte lido
    *lixo = byte >> 5;
    byte = byte & 0b00011111;
    unsigned short header = byte << 8; // coloca o primeiro byte lá pra frente
    fread(&byte, sizeof(char), 1, entrada); // segundo byte lido
    header = header | byte;

    *tamanho_arvore = header;
}

/*
    Função: 
        Cria um nó para Árvore de Huffman

    Argumentos:
        unsigned char byte -> Byte do nó;
        No *esquerda -> Ponteiro para o filho à esquerda;
        No *direita -> Ponteiro para o filho à direita;

    Retorno:
        Retorna o nó criado.
*/

No *criar_no(unsigned char byte, No *esquerda, No *direita)
{
    No *no = malloc(sizeof(No));
    no->caracter = byte;
    no->esquerda = esquerda;
    no->direita = direita;

    return no;
}

/*
    Função: 
        Cria a Árvore de Huffman ao mesmo tempo que lê ela do arquivo.

    Argumentos:
        FILE *entrada -> Arquivo comprimido à ser descomprimido;
        unsigned short *tamanho_arvore -> Tamanho da árvore.

    Retorno:
        Retorna um ponteiro para nó da raiz.
*/

No* ler_arvore_header(FILE *entrada, unsigned short *tamanho_arvore)
{
    unsigned char byte;
    fread(&byte, sizeof(unsigned char), 1, entrada);
    bool is_leaf = false;

    if (*tamanho_arvore == 0) // todos os bits da árvore foram lidos.
        return NULL;

    (*tamanho_arvore)--;

    if (byte == '\\')
    {
        (*tamanho_arvore)--;
        fread(&byte, sizeof(unsigned char), 1, entrada);
        is_leaf = 1;
    }

    if (byte != '*')
        is_leaf = true;

    if (is_leaf)
        return criar_no(byte, NULL, NULL);
    

    No *left = ler_arvore_header(entrada, tamanho_arvore);
    No *right = ler_arvore_header(entrada, tamanho_arvore);

    return criar_no('*', left, right);
}

unsigned int is_bit_i_set(unsigned char byte, int i)
{
    unsigned char mask = (1 << i);
    return byte & mask;
}

void descompactar(No *raiz, FILE *entrada, FILE *saida, unsigned char lixo)
{
    No *aux = raiz;
    unsigned char byte;
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

    for(int i = 7; lixo <= 8; lixo++, i--){
        if(aux->esquerda == NULL && aux->direita == NULL)
        {
            fprintf(saida,"%c",aux->caracter);
            aux = raiz;
        }
        if(is_bit_i_set(byte, i))
            aux = aux->direita;
        else
            aux = aux->esquerda;
    }

    fclose(entrada);        
    fclose(saida);
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

/*
    Função: 
        Executa o algoritmo para a compressão do arquivo de entrada e escreve no arquivo de saida.

    Argumentos:
        FILE *entrada -> É o arquivo que será compactado e de onde iremos ler os bytes;
        FILE *saida -> É o arquivo compactado e de onde escreveremos os bytes;

    Retorno:
        A função não possui retorno.
*/

void comprimir(FILE *entrada, FILE *saida);

/*
    Função: 
        Executa o algoritmo para a descompressão do arquivo de entrada e escreve no arquivo de saida.

    Argumentos:
        FILE *entrada -> É o arquivo que será descompactado e de onde iremos ler os bytes;
        FILE *saida -> É o arquivo descompactado e de onde escreveremos os bytes;

    Retorno:
        A função não possui retorno.
*/

void descomprimir(FILE *entrada, FILE *saida);

/*
    Função: 
        Função main, onde serão lidos os argumentos e selecionado a opção(compressão ou descompressão).

    Argumentos:
        int argc -> É a quantidade de argumentos;
        char **argv -> São os argumentos;

    Retorno:
        É retornado numeros de 1 a 4 para caso haja um erro, ou 0 para caso ocorra tudo bem.
*/

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Use: <opcao> <arquivo de entrada> <arquivo de saida>\n\tOpcoes: -c -> Compressao de arquivo\n\t\t-d -> Descompressao de arquivo\n");
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

//-------------------------------//

void comprimir(FILE *entrada, FILE *saida)
{
    unsigned int tabela_frequencia[TAM] = {0};
    frequencia_bytes(entrada, tabela_frequencia);
    
    Lista lista;
    criar_lista(&lista);
    preencher_lista(tabela_frequencia, &lista);

    No *arvore = montar_arvore(&lista);

    int colunas = altura_arvore(arvore) + 1;

    // atribuição do ponteiro para o endereço alocado
    char **dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, arvore, "", colunas);
    
    char *codificado = codificar(dicionario, entrada);

    fseek(saida, 2, SEEK_SET); // pula os dois bytes, serão escritos depois...

    int tamanho_arvore = escrever_arvore_header(arvore, saida);

    short lixo = escrever_bits(saida, codificado, tamanho_arvore);

    rewind(saida);
    escrever_header(saida, lixo, tamanho_arvore);
    
    fclose(entrada);
    fclose(saida);

    printf("\n\t----------ARQUIVO COMPRIMIDO!----------\n\n");

    free(codificado);
    liberar_arvore(arvore);
    liberar_dicionario(dicionario);
    return;
}

void descomprimir(FILE *entrada, FILE *saida)
{
    unsigned short tamanho_arvore;
    unsigned char lixo;
    ler_header(entrada, &lixo, &tamanho_arvore);
    No *arvore = ler_arvore_header(entrada, &tamanho_arvore);
    descompactar(arvore, entrada, saida, lixo);

    printf("\n\t----------ARQUIVO DESCOMPRIMIDO!----------\n\n");

    liberar_arvore(arvore);
}