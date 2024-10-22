#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

#define REGISTROS_POR_BLOCO 5
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct {
    int product_id;
    float price;
    int category;
    int brand;
} Product;

struct Indice {
    int chave;
    int end;
};

template <typename Registro>
struct Registro_Extensao {
    Registro registro;
    int next;
    int prev;
};

struct Produto {
    int chave;
    int teste;
};

#define TAMANHO_EXTENSAO 100
Registro_Extensao<Produto> EXTENSAO[TAMANHO_EXTENSAO];

int extensao_count;

template <typename Registro>
void insert_register(FILE* dados_arq, FILE* indice_arq, int indice_tamano, Registro dados);

template <typename Registro>
void recuperar_extensao();

template <typename Registro>
Indice pesquisa_binaria(int chave, int count, FILE* indice, int* pos);

template <typename Registro>
int insert_extension(Registro* r, int extensao_end);