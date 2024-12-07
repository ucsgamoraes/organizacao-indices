#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

#ifndef UTILS_H
#define UTILS_H

#define REGISTROS_POR_BLOCO 5
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct {
    int chave; //product id  
    float price;
    int category;
    char brand[40];
    int elo;
    bool removido;
} Produto;

//EXTENSAO
#define TAMANHO_EXTENSAO 100
#define PRODUTOS_DADOS_NAME "produtos.bin"

#endif
