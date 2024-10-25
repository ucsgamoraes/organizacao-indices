#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

FILE* indice_arq;
FILE* dados_arq;

void inserirProduto(Produto produto) {
    int reg_pos = 0;
    Produto* reg = buscar_registro<Produto>(produto.chave, dados_arq, indice_arq, reg_pos, EXTENSAO_PRODUTOS);

    if (reg != NULL){
        std::cout << "Produto já existe.";
        return;
    }

    insert_register<Produto>(dados_arq,indice_arq, produto, EXTENSAO_PRODUTOS, extensao_count_produtos);

}

void buscarProdutoPorChave(int chave) {
    int reg_pos = 0;
    Produto* reg = buscar_registro<Produto>(chave, dados_arq, indice_arq, reg_pos, EXTENSAO_PRODUTOS);

    if (reg != NULL){
        std::cout << "Produto já existe.";
        return;
    }

    std::cout << "Chave: " << reg->chave << std::endl;
    std::cout << "Marca: " << reg->brand << std::endl;
    std::cout << "Preço: " << reg->price << std::endl;
}

int removerProdutoPorChave(int chave) {
    return remover_registro (chave, dados_arq, indice_arq, EXTENSAO_PRODUTOS);
}

int main() {
    indice_arq = std::fopen(PRODUTOS_INDICE_NAME, "r+b");
    dados_arq = std::fopen(PRODUTOS_DADOS_NAME, "r+b");

    int opcao;
    do {
        printf("\nMenu:\n");
        printf("1. Adicionar Produto\n");
        printf("2. Buscar Produto\n");
        printf("3. Remover Produto\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: {
                Produto produto;
                
                printf("Digite o ID do produto: ");
                scanf("%d", &produto.chave);
                
                printf("Digite a marca do produto: ");
                scanf("%s", produto.brand);
                
                printf("Digite o preço do produto: ");
                scanf("%f", &produto.price);
                
                inserirProduto(produto);
                break;
            }
            case 2: {
                int chave;
                printf("Digite a chave do produto para buscar: ");
                scanf("%d", &chave);
                
                buscarProdutoPorChave(chave);
                break;
            }
            case 3: {
                int chave;
                printf("Digite a chave do produto para remover: ");
                scanf("%d", &chave);
                
                removerProdutoPorChave(chave);
                break;
            }
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
                break;
        }
    } while (opcao != 0);

    std::fclose(dados_arq);
    std::fclose(indice_arq);

    return 0;
}