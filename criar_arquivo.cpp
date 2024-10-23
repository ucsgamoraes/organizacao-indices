#include "main.h"
#include <string>

#define MAX_LINE_LEN 1024
#define MAX_BUFFER_SIZE 100 
#define PARTITIONS_ENTRIES 5

const char* NOME_ARQUIVO = "2019-Oct.csv";

void criar_particoes(){
    FILE* arquivo_csv = fopen(NOME_ARQUIVO, "r");

    Produto produtos[PARTITIONS_ENTRIES];
    Usuario usuarios[PARTITIONS_ENTRIES];

    int current_patition = 0;

    const std::string prod_part = "produtos_particoes/produto_part_";
    const std::string user_part = "usuarios_particoes/usuarios_part_";

    const char* prod_part_name = (prod_part + std::to_string(current_patition)).c_str();
    const char* user_part_name = (user_part + std::to_string(current_patition)).c_str();

    FILE *produtos_part_file;
    FILE *usuarios_part_file;

    produtos_part_file = std::fopen(prod_part_name, "wb+");
    usuarios_part_file = std::fopen(user_part_name, "wb+");

    char buffer[MAX_LINE_LEN];
    int entry_index = 0;
    char *result;

    while((result = fgets(buffer, sizeof(buffer), arquivo_csv)) != NULL){
        char event_time[50] = {0};
        char event_type[10] = {0};
        int product_id = 0;
        int category_id = 0;
        char category_code[100] = {0};
        char brand[40] = {0};
        float price = 0;
        int user_id = 0;
        char user_session[100] = {0};

        sscanf(buffer, "%49[^,],%9[^,],%d,%d,%99[^,],%29[^,],%f,%d,%99[^,]", event_time, event_type, &product_id, &category_id, category_code, brand, &price, &user_id, user_session);

        produtos[entry_index] = (Produto) {product_id, price, category_id, 0};
        strncpy(produtos[entry_index].brand, brand, 40);

        usuarios[entry_index] = (Usuario) {user_id, 0};
        strncpy(usuarios[entry_index].event_time, event_time, 50);
        strncpy(usuarios[entry_index].event_type, event_type, 10);
        
        entry_index++;

        if (entry_index == PARTITIONS_ENTRIES) {
            current_patition++;
            //escreve
            std::fwrite(produtos, sizeof(Produto), entry_index, produtos_part_file);
            std::fwrite(usuarios, sizeof(Usuario), entry_index, usuarios_part_file);
            //abrir outra partição
            prod_part_name = (prod_part + std::to_string(current_patition)).c_str();
            user_part_name = (user_part + std::to_string(current_patition)).c_str();
            produtos_part_file = std::fopen(prod_part_name, "wb+");
            usuarios_part_file = std::fopen(user_part_name, "wb+");
            //começar do zero
            entry_index = 0;
        }
    }

    if (entry_index > 0){
        //escreve
        std::fwrite(produtos, sizeof(Produto), entry_index, produtos_part_file);
        std::fwrite(usuarios, sizeof(Usuario), entry_index, usuarios_part_file);
    }
    
    fclose(arquivo_csv);
}

void intercalar_particoes (){

}

int main (){
    criar_particoes();
}