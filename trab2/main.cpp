#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

#include "bptree.h"
#include "utils.h"
#include "hash.h"

using namespace std;

vector<Produto> produtosEmMemoria;
int memoriaCurrentIndex = 0;

Produto buscarProdutoArvore(BPlusTree& tree, int chave, const string& arquivoDados) {
    Node* cursor = tree.getRoot();
    while (!cursor->isLeaf) {
        auto it = upper_bound(cursor->keys.begin(), cursor->keys.end(), chave);
        int pos = it - cursor->keys.begin();
        cursor = cursor->children[pos];
    }

    auto it = find(cursor->keys.begin(), cursor->keys.end(), chave);
    if (it == cursor->keys.end()) {
        throw runtime_error("Produto não encontrado na Árvore B+.");
    }
    int pos = it - cursor->keys.begin();
    long endereco = cursor->addresses[pos];

    ifstream file(arquivoDados, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Erro ao abrir o arquivo de dados.");
    }

    file.seekg(endereco, ios::beg);

    Produto produto;
    file.read(reinterpret_cast<char*>(&produto), sizeof(Produto));
    file.close();
    return produto;
}

Produto buscarProdutoHash(HashTable& hashTable, int categoria, const string& arquivoDados) {
    HashNode* result = hashTable.search(categoria);
    if (!result) {
        throw runtime_error("Nenhum produto encontrado para a categoria fornecida.");
    }

    ifstream file(arquivoDados, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Erro ao abrir o arquivo de dados.");
    }

    Produto produtoEncontrado;

    cout << "Produtos na categoria " << categoria << ":\n";

    while (result) {
        file.seekg(result->fileIndex, ios::beg);

        Produto produto;
        file.read(reinterpret_cast<char*>(&produto), sizeof(Produto));

        cout << " - Produto encontrado: " << produto.brand << " (Chave: " << produto.chave << ")\n";

        produtoEncontrado = produto;

        result = result->next;
    }

    file.close();

    return produtoEncontrado;
}

void buscarProdutoPorCategoriaEMarca(HashTable& hashTable, int categoria, const string& marca, const string& arquivoDados) {
    try {
        HashNode* resultado = hashTable.search(categoria);
        if (!resultado) {
            cout << "Nenhum produto encontrado para a categoria " << categoria << ".\n";
            return;
        }

        ifstream file(arquivoDados, ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Erro ao abrir o arquivo de dados.");
        }

        cout << "Produtos na categoria " << categoria << " da marca \"" << marca << "\":\n";

        bool encontrado = false;
        while (resultado) {
            file.seekg(resultado->fileIndex, ios::beg);
            Produto produto;
            file.read(reinterpret_cast<char*>(&produto), sizeof(Produto));

            if (produto.brand == marca) {
                encontrado = true;
                cout << " - Produto: " << produto.brand << " (Chave: " << produto.chave << ", Preco: " << produto.price << ")\n";
            }

            resultado = resultado->next;
        }

        if (!encontrado) {
            cout << "Nenhum produto encontrado com a marca \"" << marca << "\" na categoria " << categoria << ".\n";
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }
}

int main() {
    int order = 4;
    BPlusTree tree(order);

    ifstream file(PRODUTOS_DADOS_NAME, ios::binary);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo 'produtos_merged.bin'." << endl;
        return 1;
    }

    Produto produto;
    long address = 0;

    auto start_tree = chrono::high_resolution_clock::now();
    while (file.read(reinterpret_cast<char*>(&produto), sizeof(Produto))) {
        if (produto.removido) {
            address += sizeof(Produto);
            continue;
        }
        tree.insert(produto.chave, address);
        address += sizeof(Produto);
    }

    auto end_tree = chrono::high_resolution_clock::now();
    auto duration_tree = chrono::duration_cast<chrono::microseconds>(end_tree - start_tree).count();
    cout << "Tempo de criacao da arvore B+: " << duration_tree << " us" << endl;

    file.clear();
    file.seekg(0, ios::beg);
    address = 0;

    auto start_hash = chrono::high_resolution_clock::now();

    HashTable hashTable;
    while (file.read(reinterpret_cast<char*>(&produto), sizeof(Produto))) {
        if (produto.removido) {
            address += sizeof(Produto);
            continue;
        }
        hashTable.insert(produto.category, produto.chave, address);
        address += sizeof(Produto);
    }
    auto end_hash = chrono::high_resolution_clock::now();
    auto duration_hash = chrono::duration_cast<chrono::microseconds>(end_hash - start_hash).count();
    cout << "Tempo de criacao da tabela hash: " << duration_hash << " us" << endl;

    file.close();

    Produto novoProduto = {
        9999,
        123.99,
        67287,
        "ExemploMarca",
        false,
    };

    auto start_insert = chrono::high_resolution_clock::now();

    std::ofstream fs(PRODUTOS_DADOS_NAME, std::ios::out | std::ios::binary | std::ios::in);
    if (!fs.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita." << std::endl;
        return 1;
    }

    fs.seekp(std::ios::end);
    address = fs.tellp();

    fs.write(reinterpret_cast<const char*>(&novoProduto), sizeof(Produto));
    if (!fs) {
        std::cerr << "Erro ao escrever no arquivo." << std::endl;
        return 1;
    }

    std::cout << "Produto salvo no arquivo com sucesso." << std::endl;

    tree.insert(novoProduto.chave, address);
    //hashTable.insert(novoProduto.category, novoProduto.chave, address);

    auto end_insert = chrono::high_resolution_clock::now();
    auto duration_insert = chrono::duration_cast<chrono::microseconds>(end_insert - start_insert).count();
    cout << "Tempo para inserir novo registro: " << duration_insert << " us" << endl;

    // Testar remoção
    // auto start_remove = chrono::high_resolution_clock::now();
    // tree.remove(novoProduto.chave); // Supondo que remove é implementado
    // hashTable.remove(novoProduto.category, novoProduto.chave); // Supondo que remove é implementado
    // auto end_remove = chrono::high_resolution_clock::now();
    // auto duration_remove = chrono::duration_cast<chrono::milliseconds>(end_remove - start_remove).count();
    // cout << "Tempo para remover registro: " << duration_remove << " ms" << endl;


    // Consulta 1: Buscar Produto na Árvore B+
    int chaveBusca = 9999; // Exemplo de chave
    auto start_query_tree = chrono::high_resolution_clock::now();
    try {
        Produto produtoArvore = buscarProdutoArvore(tree, chaveBusca, PRODUTOS_DADOS_NAME);
        cout << "Produto encontrado na arvore B+: " << produtoArvore.brand << " (Chave: " << produtoArvore.chave << ")\n";
    } catch (const exception& e) {
        cerr << "Erro na busca pela arvore B+: " << e.what() << endl;
    }
    auto end_query_tree = chrono::high_resolution_clock::now();
    auto duration_query_tree = chrono::duration_cast<chrono::microseconds>(end_query_tree - start_query_tree).count();
    cout << "Tempo da consulta na arvore B+: " << duration_query_tree << " us\n";

    // Consulta 2: Buscar Produto na Tabela Hash
    int categoriaBusca = 67287; // Exemplo de categoria
    auto start_query_hash = chrono::high_resolution_clock::now();
    Produto produtoHash = buscarProdutoHash(hashTable, categoriaBusca, PRODUTOS_DADOS_NAME);
    auto end_query_hash = chrono::high_resolution_clock::now();
    auto duration_query_hash = chrono::duration_cast<chrono::microseconds>(end_query_hash - start_query_hash).count();
    cout << "Tempo da consulta na Tabela Hash: " << duration_query_hash << " us\n";

    // Consulta 3: Buscar Produto por Categoria e Marca
    string marcaBusca = "ExemploMarca";
    auto start_query_combined = chrono::high_resolution_clock::now();
    buscarProdutoPorCategoriaEMarca(hashTable, categoriaBusca, marcaBusca, PRODUTOS_DADOS_NAME);
    auto end_query_combined = chrono::high_resolution_clock::now();
    auto duration_query_combined = chrono::duration_cast<chrono::microseconds>(end_query_combined - start_query_combined).count();
    cout << "Tempo da consulta por Categoria e Marca: " << duration_query_combined << " us\n";

    return 0;
}
