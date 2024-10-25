#include "main.h"
#include <string>
#include <algorithm>
#include <vector>
#include <queue>

#define MAX_LINE_LEN 1024
#define MAX_BUFFER_SIZE 100 
#define MAX_OPEN_FILES 1024
#define PARTITIONS_ENTRIES 5

const char* NOME_ARQUIVO = "2019-Oct.csv";
const std::string prod_part = "produtos_particoes/produto_part_";
const std::string user_part = "usuarios_particoes/usuarios_part_";

template <typename Registro>
bool comparaPorChave(const Registro &a, const Registro &b) {
    return a.chave < b.chave;
}

void criar_particoes(){
    FILE* arquivo_csv = fopen(NOME_ARQUIVO, "r");

    Produto produtos[PARTITIONS_ENTRIES];
    Usuario usuarios[PARTITIONS_ENTRIES];

    int current_patition = 0;

    FILE *produtos_part_file;
    FILE *usuarios_part_file;

    std::string part_number = std::to_string(current_patition);
    produtos_part_file = std::fopen((prod_part + part_number).c_str(), "wb+");
    usuarios_part_file = std::fopen((user_part + part_number).c_str(), "wb+");

    char buffer[MAX_LINE_LEN];
    int entry_index = 0;
    char *result;

    while((result = fgets(buffer, sizeof(buffer), arquivo_csv)) != NULL){
        char category_code[100] = {0};
        char user_session[100] = {0};

        Produto novoProduto;
        Usuario novoUsuario;

        novoProduto.removido = 0;
        novoProduto.elo = -1;

        novoUsuario.removido = 0;
        novoUsuario.elo = -1;
    
        sscanf(buffer, "%[^,],%[^,],%d,%d,%[^,],%[^,],%f,%d,%[^,]", 
            &novoUsuario.event_time, 
            &novoUsuario.event_type, 
            &novoProduto.chave, 
            &novoProduto.category, 
            &category_code, 
            &novoProduto.brand, 
            &novoProduto.price, 
            &novoUsuario.chave, 
            user_session
        );

        produtos[entry_index] = novoProduto;
        usuarios[entry_index] = novoUsuario;

        // strncpy(produtos[entry_index].brand, brand, 40);
        // strncpy(usuarios[entry_index].event_time, event_time, 50);
        // strncpy(usuarios[entry_index].event_type, event_type, 10);
        
        entry_index++;

        if (entry_index == PARTITIONS_ENTRIES) {
            current_patition++;
            //ordenar partição
            std::sort(produtos, produtos + PARTITIONS_ENTRIES, comparaPorChave<Produto>);
            std::sort(usuarios, usuarios + PARTITIONS_ENTRIES, comparaPorChave<Usuario>);
            //escreve
            std::fwrite(produtos, sizeof(Produto), entry_index, produtos_part_file);
            std::fwrite(usuarios, sizeof(Usuario), entry_index, usuarios_part_file);
            std::fclose(produtos_part_file);
            std::fclose(usuarios_part_file);
            //abrir outra partição
            part_number = std::to_string(current_patition);
            produtos_part_file = std::fopen((prod_part + part_number).c_str(), "wb+");
            usuarios_part_file = std::fopen((user_part + part_number).c_str(), "wb+");
            //começar do zero
            entry_index = 0;
        }
    }

    if (entry_index > 0){
        //escreve
        std::fwrite(produtos, sizeof(Produto), entry_index, produtos_part_file);
        std::fwrite(usuarios, sizeof(Usuario), entry_index, usuarios_part_file);
        std::fclose(produtos_part_file);
        std::fclose(usuarios_part_file);
    }
    
    fclose(arquivo_csv);
}

template <typename Registro>
struct RegistroMenor {
    Registro registro;
    int fileIndex;
    bool operator>(const RegistroMenor& other) const {
        return registro.chave > other.registro.chave;
    }
};

template <typename Registro>
void intercalar_particoes(const std::string partition_label, const std::string output_name) {
    FILE* particoes[MAX_OPEN_FILES];
    FILE* particao_saida;
    int i = 0;
    char particao_nome[20];
    std::string part_number;
    std::priority_queue<RegistroMenor<Registro>, std::vector<RegistroMenor<Registro>>, std::greater<RegistroMenor<Registro>>> fila_prioridade;

    while (i < MAX_OPEN_FILES) {
        part_number = std::to_string(i);
        particoes[i] = std::fopen((partition_label + part_number).c_str(), "rb");
        if (particoes[i] == NULL) break;
        
        //ler primeiro registro de cada partição
        Registro registro_lido;
        if (std::fread(&registro_lido, sizeof(Registro), 1, particoes[i]) == 1) {
            fila_prioridade.push({registro_lido, i});
        }
        i++;
    }

    //arquivo de saída
    particao_saida = std::fopen(output_name.c_str(), "wb");

    //intercalar
    while (!fila_prioridade.empty()) {
        //menor registro
        RegistroMenor<Registro> menor = fila_prioridade.top();
        fila_prioridade.pop();

        //escrever no arquivo de saída
        std::fwrite(&menor.registro, sizeof(Registro), 1, particao_saida);

        //ler o próximo registro do arquivo de onde veio o menor
        Registro registro_lido;
        if (std::fread(&registro_lido, sizeof(Registro), 1, particoes[menor.fileIndex]) == 1) {
            fila_prioridade.push({registro_lido, menor.fileIndex});
        }
    }

    for (int j = 0; j < i; j++) {
        std::fclose(particoes[j]);
    }
    std::fclose(particao_saida);
}

template <typename Registro>
void imprimir_chaves_arquivo(const std::string& merged_file) {
    FILE* arquivo = std::fopen(merged_file.c_str(), "rb");
    if (arquivo == NULL) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return;
    }

    Registro registro_lido;
    //ler cada registro até o final do arquivo
    while (std::fread(&registro_lido, sizeof(Registro), 1, arquivo) == 1) {
        std::cout << "Chave: " << registro_lido.chave << std::endl;
    }

    std::fclose(arquivo);
}

template <typename Registro>
void criar_arquivo_indice(const std::string& merged_file, const std::string& index_file) {
    FILE* arquivo = std::fopen(merged_file.c_str(), "rb");
    FILE* arquivo_indice = std::fopen(index_file.c_str(), "wb");

    if (arquivo == NULL || arquivo_indice == NULL) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return;
    }

    Registro buffer[REGISTROS_POR_BLOCO];
    Indice indice;
    long first_offset;

    while (true) {
        //salva o offset do primeiro registro do bloco
        first_offset = std::ftell(arquivo);

        //lê o bloco de registros
        size_t registros_lidos = std::fread(buffer, sizeof(Registro), REGISTROS_POR_BLOCO, arquivo);
        if (registros_lidos == 0) break;

        //cria o índice para o bloco lido
        indice.chave = buffer[registros_lidos - 1].chave;
        indice.end = first_offset;

        //escreve o índice no arquivo de índices
        std::fwrite(&indice, sizeof(Indice), 1, arquivo_indice);

        if (registros_lidos < REGISTROS_POR_BLOCO) break;
    }

    std::fclose(arquivo);
    std::fclose(arquivo_indice);
}

int main (){
    criar_particoes();
    intercalar_particoes<Produto>(prod_part, "produtos_merged.bin");
    criar_arquivo_indice<Produto>("produtos_merged.bin", "produtos_indice.bin");
    imprimir_chaves_arquivo<Produto>("produtos_merged.bin");

    intercalar_particoes<Produto>(prod_part, "usuarios_merged.bin");
    //imprimir_chaves_arquivo<Produto>(prod_part + "merged");
    criar_arquivo_indice<Produto>("usuarios_merged.bin", "usuarios_indice.bin");

}