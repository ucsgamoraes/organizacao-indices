#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

#include "main.h"

#define REGISTROS_POR_BLOCO 5
#define TAMANHO_EXTENSAO 100

template <typename Registro>
void inicializar_extensao() {
    for (int i = 0; i < TAMANHO_EXTENSAO; ++i) {
        EXTENSAO<Registro>[i] = {0};
    }
}

int extensao_count = 0;

template <typename Registro>
int insert_extension(Registro* r, int extensao_end) {
    std::cout << r->chave << " para a extensao " << extensao_end << std::endl;

    if (extensao_end == -1) {
        int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);
        EXTENSAO<Registro>[extensao_count] = {r->chave, 9, -1, -1};
        extensao_count++;
        return new_pos;
    }

    int extensao_index = extensao_end / sizeof(Registro_Extensao<Registro>);
    Registro_Extensao<Registro>* current = &EXTENSAO<Registro>[extensao_index];
    int current_pos = extensao_end;
    int i = 0;

    while (i < TAMANHO_EXTENSAO) {
        if (current->registro.chave == r->chave) {
            return -1;
        }

        if (current->registro.chave > r->chave) {
            int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);

            EXTENSAO<Registro>[extensao_count] = {r->chave, 9, current_pos, current->prev};
            int prev_index = current->prev / sizeof(Registro_Extensao<Registro>);
            if (prev_index != -1) {
                EXTENSAO<Registro>[prev_index].next = new_pos;
            }
            int new_ext_pos = EXTENSAO<Registro>[extensao_count].prev == -1 ? new_pos : extensao_end;
            current->prev = new_pos;
            extensao_count++;
            return new_ext_pos;
        }

        if (current->next == -1) {
            int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);
            EXTENSAO<Registro>[extensao_count] = {r->chave, 9, current_pos, current->prev};
            current->next = new_pos;
            extensao_count++;
            return extensao_end;
        }

        current_pos = current->next;
        int next_index = current->next / sizeof(Registro_Extensao<Registro>);
        current = &EXTENSAO<Registro>[next_index];
        i++;
    }

    return 0;
}

// Replacing std::fstream with std::FILE functions
template <typename Registro>
Indice pesquisa_binaria(int chave, int count, FILE* indice, int* pos) {
    int mid = 0, high = count - 1, low = 0;
    Indice indice_reg = {0};
    int indice_tamanho = sizeof(Indice);

    while (low <= high) {
        mid = (low + high) / 2;
        std::fseek(indice, 4 + mid * indice_tamanho, SEEK_SET);
        std::fread(&indice_reg, indice_tamanho, 1, indice);

        if (indice_reg.chave < chave) {
            low = mid + 1;
        } else if (indice_reg.chave > chave) {
            high = mid - 1;
        } else {
            (*pos) = mid * indice_tamanho;
            return indice_reg;
        }
    }

    if (indice_reg.chave < chave && mid < count - 1) {
        mid += 1;
        std::fseek(indice, mid * indice_tamanho, SEEK_SET);
        std::fread(&indice_reg, indice_tamanho, 1, indice);
    }

    (*pos) = mid * indice_tamanho;
    return indice_reg;
}

template <typename Registro>
void salvar_extensao() {
    FILE* extensao_arq = std::fopen("extensao.bin", "wb");
    std::fwrite(EXTENSAO<Registro>, sizeof(Registro_Extensao<Registro>), extensao_count, extensao_arq);
    std::fclose(extensao_arq);
}

template <typename Registro>
void recuperar_extensao() {
    FILE* extensao_arq = std::fopen("extensao.bin", "rb");
    extensao_count = std::fread(EXTENSAO<Registro>, sizeof(Registro_Extensao<Registro>), 100, extensao_arq);
    std::fclose(extensao_arq);
}

// Replacing std::fstream with std::FILE functions
template <typename Registro>
void insert_register(FILE* dados_arq, FILE* indice_arq, int indice_tamano, Registro dados) {
    int bloco_escolhido = -1;
    int indice_pos = 0;
    int chave = dados.chave;

    Indice idx = pesquisa_binaria<Registro>(dados.chave, indice_tamano, indice_arq, &indice_pos);
    bloco_escolhido = idx.end;

    if (idx.chave == dados.chave) {
        return;
    }

    Registro registros[REGISTROS_POR_BLOCO] = {0};
    std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
    int num_registros = std::fread(registros, sizeof(Registro), REGISTROS_POR_BLOCO, dados_arq);

    int extensao_end = -1;

    if (num_registros == REGISTROS_POR_BLOCO) {
        std::fread(&extensao_end, sizeof(int), 1, dados_arq);
    }

    int i, j = 0;
    for (i = 0; i < num_registros; i++) {
        if (chave < registros[i].chave) {
            break;
        } else if (chave == registros[i].chave) {
            return;
        }
    }

    if (i < num_registros) {
        if (num_registros + 1 > REGISTROS_POR_BLOCO) {
            extensao_end = insert_extension<Registro>(&registros[num_registros - 1], extensao_end);
            if (extensao_end == -1) return;
            std::fseek(dados_arq, bloco_escolhido + sizeof(Registro) * REGISTROS_POR_BLOCO, SEEK_SET);
            std::fwrite(&extensao_end, sizeof(int), 1, dados_arq);
        }

        int last_index = num_registros - 1;
        int start_index = std::min(last_index + 1, REGISTROS_POR_BLOCO - 1);
        for (j = start_index; j > i; j--) {
            registros[j] = registros[j - 1];
        }

        registros[i] = dados;
        int to_write = std::min(num_registros + 1, REGISTROS_POR_BLOCO);
        std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
        std::fwrite(registros, sizeof(Registro), to_write, dados_arq);
    } else if (i == REGISTROS_POR_BLOCO) {
        extensao_end = insert_extension<Registro>(&dados, extensao_end);
        if (extensao_end == -1) return;
        std::fseek(dados_arq, bloco_escolhido + sizeof(Registro) * REGISTROS_POR_BLOCO, SEEK_SET);
        std::fwrite(&extensao_end, sizeof(int), 1, dados_arq);
    } else {
        idx.chave = dados.chave;
        idx.end = bloco_escolhido;
        std::fseek(indice_arq, 4 + indice_pos, SEEK_SET);
        std::fwrite(&idx, sizeof(Indice), 1, indice_arq);

        registros[i] = dados;
        std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
        int bloco_novo_tamanho = num_registros + 1;
        std::fwrite(registros, sizeof(Registro), bloco_novo_tamanho, dados_arq);

        if (bloco_novo_tamanho == REGISTROS_POR_BLOCO) {
            int extensao_end = -1;
            std::fwrite(&extensao_end, sizeof(int), 1, dados_arq);
        }
    }
}

int main() {
    inicializar_extensao<Produto>();
    recuperar_extensao<Produto>();

    FILE* indice_arq = std::fopen("indice.bin", "r+b");
    if (!indice_arq) {
        std::cerr << "Failed to open indice.bin" << std::endl;
        return 1;
    }

    FILE* dados_arq = std::fopen("dados.bin", "r+b");
    if (!dados_arq) {
        std::cerr << "Failed to open dados.bin" << std::endl;
        std::fclose(indice_arq);
        return 1;
    }

    int tamanho_indice = 0;
    std::fseek(indice_arq, 0, SEEK_SET);
    std::fread(&tamanho_indice, sizeof(int), 1, indice_arq);

    if (tamanho_indice == 0) {
        tamanho_indice = 1;
        std::fseek(indice_arq, 0, SEEK_SET);
        std::fwrite(&tamanho_indice, sizeof(int), 1, indice_arq);
    }

    Produto novo_registro = {199, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);
    novo_registro = {250, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    novo_registro = {1230, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    novo_registro = {590, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    novo_registro = {9999, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    novo_registro = {444, 999};
    insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);
    salvar_extensao<Produto>();

    std::fclose(dados_arq);
    std::fclose(indice_arq);

    return 0;
}
