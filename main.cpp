#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

#include "main.h"

#define REGISTROS_POR_BLOCO 5
#define TAMANHO_EXTENSAO 100

template <typename Registro>
void recriar_arquivos(std::FILE* dados_arq, std::FILE* indice_arq, Registro_Extensao<Registro>* EXTENSAO) {
    int extensao_end = -1, num_registros = 1;
    Registro registros[REGISTROS_POR_BLOCO * 5];

    std::FILE* novo_dados = std::fopen("dados2.bin", "wb+");
    std::FILE* novo_indice = std::fopen("indice2.bin", "wb+");

    std::fseek(novo_indice, sizeof(int), SEEK_SET);
    std::fseek(dados_arq, 0, SEEK_SET);

    int block_index = 0;
    int last_block_end = -1;
    int indice_entradas = 0;
    Indice ind;

    while (num_registros != 0) {
        num_registros = std::fread(&registros, sizeof(Registro), REGISTROS_POR_BLOCO, dados_arq);
        extensao_end = -1;

        if (num_registros == REGISTROS_POR_BLOCO) {
            std::fread(&extensao_end, sizeof(int), 1, dados_arq);
        }

        int i = num_registros;

        if (extensao_end != -1) {
            int extensao_index = extensao_end / sizeof(Registro_Extensao<Registro>);
            Registro_Extensao<Registro>* current = &EXTENSAO[extensao_index];
            int next_index = 0, next_pos = 0;

            while (next_index != -1) {
                registros[i] = current->registro;
                next_pos = current->next;
                next_index = next_pos / sizeof(Registro_Extensao<Registro>);
                current = &EXTENSAO[next_index];
                i++;
            }
        }

        int ext = 0;
        int j = 0;
        while (j < i) {
            if (last_block_end == -1) {
                last_block_end = block_index * sizeof(Registro);
            }

            std::fwrite(&registros[j], sizeof(Registro), 1, novo_dados);
            if ((block_index + 1) % REGISTROS_POR_BLOCO == 0 || j == i - 1) {
                std::fwrite(&ext, sizeof(int), 1, novo_dados);
                ind.end = last_block_end;
                ind.chave = registros[j].product_id;
                std::fwrite(&ind, sizeof(Indice), 1, novo_indice);
                indice_entradas++;
                last_block_end = -1;
            }
            block_index++;
            j++;
        }
    }

    std::fseek(novo_indice, 0, SEEK_SET);
    std::fwrite(&indice_entradas, sizeof(int), 1, novo_indice);

    std::fclose(novo_indice);
    std::fclose(novo_dados);
    std::fclose(dados_arq);
    std::fclose(indice_arq);

    std::remove("dados.bin");
    std::remove("indice.bin");
    std::rename("indice2.bin", "indice.bin");
    std::rename("dados2.bin", "dados.bin");
}

template <typename Registro>
void salvar_extensao(std::string file_name, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count) {
    FILE* extensao_arq = std::fopen(file_name.c_str(), "wb");
    std::fwrite(EXTENSAO, sizeof(Registro_Extensao<Registro>), extensao_count, extensao_arq);
    std::fclose(extensao_arq);
}

template <typename Registro>
void recuperar_extensao(std::string file_name, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count) {
    FILE* extensao_arq = std::fopen(file_name.c_str(), "ab+");
    extensao_count = std::fread(EXTENSAO, sizeof(Registro_Extensao<Registro>), 100, extensao_arq);
    std::fclose(extensao_arq);
}

template <typename Registro>
int insert_extension(Registro* r, int extensao_end, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count) {
    std::cout << r->chave << " para a extensao " << extensao_end << std::endl;

    if (extensao_end == -1) {
        int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);
        EXTENSAO[extensao_count] = {*r, -1, -1};
        extensao_count++;
        return new_pos;
    }

    int extensao_index = extensao_end / sizeof(Registro_Extensao<Registro>);
    Registro_Extensao<Registro>* current = &EXTENSAO[extensao_index];
    int current_pos = extensao_end;
    int i = 0;

    while (i < TAMANHO_EXTENSAO) {
        if (current->registro.chave == r->chave) {
            return -1;
        }

        if (current->registro.chave > r->chave) {
            int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);

            EXTENSAO[extensao_count] = {*r, current_pos, current->prev};
            int prev_index = current->prev / sizeof(Registro_Extensao<Registro>);
            if (current->prev != -1) {
                EXTENSAO[prev_index].next = new_pos;
            }
            int new_ext_pos = EXTENSAO[extensao_count].prev == -1 ? new_pos : extensao_end;
            current->prev = new_pos;
            extensao_count++;
            return new_ext_pos;
        }

        if (current->next == -1) {
            int new_pos = extensao_count * sizeof(Registro_Extensao<Registro>);
            EXTENSAO[extensao_count] = {*r, current_pos, current->prev};
            current->next = new_pos;
            extensao_count++;
            return extensao_end;
        }

        current_pos = current->next;
        int next_index = current->next / sizeof(Registro_Extensao<Registro>);
        current = &EXTENSAO[next_index];
        i++;
    }

    return 0;
}

template <typename Registro>
Indice pesquisa_binaria(int chave, int count, FILE* indice, int* pos) {
    int mid = 0, high = count - 1, low = 0;
    Indice indice_reg = {0};
    int indice_tamanho = sizeof(Indice);

    while (low <= high) {
        mid = (low + high) / 2;
        std::fseek(indice, mid * indice_tamanho, SEEK_SET);
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
void insert_register(FILE* dados_arq, FILE* indice_arq, int indice_tamano, Registro dados, Registro_Extensao<Registro>* EXTENSAO, int& count) {
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

    int i, j = 0;
    for (i = 0; i < num_registros; i++) {
        if (chave < registros[i].chave) {
            break;
        } else if (chave == registros[i].chave) {
            return;
        }
    }

        std::cout << 1;
    if (i < num_registros) {
        if (num_registros + 1 > REGISTROS_POR_BLOCO) {
            extensao_end = registros[num_registros-1].elo;
            registros[num_registros-1].elo = -1;
            extensao_end = insert_extension<Registro>(&registros[num_registros-1], extensao_end, EXTENSAO, count);
            if (extensao_end == -1) return;
        }

        int end_index = std::min(num_registros, REGISTROS_POR_BLOCO - 1);

        for (j = end_index; j > i; j--) {
            registros[j] = registros[j - 1];
        }

        registros[i] = dados;
        registros[end_index].elo = extensao_end;

        int to_write = std::min(num_registros + 1, REGISTROS_POR_BLOCO);
        std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
        std::fwrite(registros, sizeof(Registro), to_write, dados_arq);
    } else if (i == REGISTROS_POR_BLOCO) {
        extensao_end = insert_extension<Registro>(&registros[num_registros-1], extensao_end, EXTENSAO, count);
        if (extensao_end == -1) return;
        registros[num_registros-1].elo = extensao_end;
        std::fseek(dados_arq, bloco_escolhido + sizeof(Registro) * num_registros-1, SEEK_SET);
        std::fwrite(&extensao_end, sizeof(int), 1, dados_arq);
    } else {
        idx.chave = dados.chave;
        idx.end = bloco_escolhido;
        std::fseek(indice_arq, indice_pos, SEEK_SET);
        std::fwrite(&idx, sizeof(Indice), 1, indice_arq);
        std::cout << "sadkjsdakjl";

        registros[i] = dados;
        std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
        int bloco_novo_tamanho = num_registros + 1;
        std::fwrite(registros, sizeof(Registro), bloco_novo_tamanho, dados_arq);
    }
}

template <typename Registro>
Registro* buscar_registro (int chave, FILE* dados_arq, int indice_tamanho, FILE* indice_arq, int& file_pos, Registro_Extensao<Registro>* EXTENSAO){
    int indice_pos,bloco_escolhido = 0;
    Indice idx = pesquisa_binaria<Registro>(chave, indice_tamanho, indice_arq, &indice_pos);
    bloco_escolhido = idx.end;

    Registro* registro_return = (Registro*)malloc(sizeof(Registro));
    Registro registros[REGISTROS_POR_BLOCO] = {0};
    std::fseek(dados_arq, bloco_escolhido, SEEK_SET);
    int num_registros = std::fread(registros, sizeof(Registro), REGISTROS_POR_BLOCO, dados_arq);

    int extensao_end = registros[num_registros-1].elo;

    int i, j = 0;
    for (i = 0; i < num_registros; i++) {
        if (chave == registros[i].chave && !registros[i].removido) {
            *registro_return = registros[i];
            file_pos += bloco_escolhido + sizeof(Registro) * i;
            return registro_return;
        }
    }

    if (extensao_end == -1) {
        return NULL;
    }

    int extensao_index = extensao_end / sizeof(Registro_Extensao<Registro>);
    Registro_Extensao<Registro>* current = &EXTENSAO[extensao_index];
    int current_pos = extensao_end;

    i =0;

    while (i < TAMANHO_EXTENSAO) {
        if (current->registro.chave == chave && !current->registro.removido) {
            *registro_return = current->registro;
            file_pos += bloco_escolhido + sizeof(Registro) * i;
            return registro_return;
        }

        if (current->next == -1) {
            return NULL;
        }

        int next_index = current->next / sizeof(Registro_Extensao<Registro>);
        current = &EXTENSAO[next_index];
        i++;
    }

    return NULL;
}

template <typename Registro>
void remover_registro (int chave, FILE* dados_arq, int indice_tamanho, FILE* indice_ar, Registro_Extensao<Registro>* EXTENSAO) {
    int reg_pos = 0;
    Registro* reg = buscar_registro(chave, dados_arq, indice_tamanho, indice_ar, reg_pos, EXTENSAO);
    if(reg == NULL) {
        std::cout << "Registro não existe!";
    }
    reg.removido = true;

    std::fseek(dados_arq, reg_pos, SEEK_SET);
    std::fwrite(reg, sizeof(Registro), 1, dados_arq);
} 

//implementar função de busca e remoção
int main() {
    recuperar_extensao<Produto>(PRODUTOS_EXTENSAO_NAME, EXTENSAO_PRODUTOS, extensao_count_produtos);
    recuperar_extensao<Usuario>(USUARIOS_EXTENSAO_NAME, EXTENSAO_USUARIOS, extensao_count_usuarios);

    FILE* indice_arq = std::fopen("produtos_indice.bin", "r+b");
    FILE* dados_arq = std::fopen("produtos_merged.bin", "r+b");

    std::fseek(indice_arq, 0, SEEK_END);
    int tamanho_indice = ftell(indice_arq);
    tamanho_indice /= sizeof(Indice);


    int reg_pos = 0;
    Produto* reg = buscar_registro<Produto>(1005135, dados_arq, tamanho_indice, indice_arq, reg_pos, EXTENSAO_PRODUTOS);

    if (reg == NULL){
        std::cout << "Produto não existe!";
        return 0;
    }
    std::cout << "Chave: " << reg->chave << std::endl;
    std::cout << "Brand: " << reg->brand<< std::endl;
    std::cout << "Price: " << reg->price << std::endl;

    //quais eventos algum usuario criou



    // novo_registro = {1230, 999};
    // insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    // novo_registro = {590, 999};
    // insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    // novo_registro = {9999, 999};
    // insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    // novo_registro = {444, 999};
    // insert_register(dados_arq, indice_arq, tamanho_indice, novo_registro);

    //salvar_extensao<Produto>(PRODUTOS_EXTENSAO_NAME, EXTENSAO_PRODUTOS, extensao_count_produtos);

    std::fclose(dados_arq);
    std::fclose(indice_arq);

    return 0;
}
