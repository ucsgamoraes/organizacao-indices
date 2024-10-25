#include <cstdio>
#include <iostream>
#include <cstring>
#include <cmath>

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

typedef struct {
    int chave; // user_id
    char event_time[50];
    char event_type[10];
    int elo;
    bool removido;
} Usuario;

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

//EXTENSAO
#define TAMANHO_EXTENSAO 100
#define PRODUTOS_EXTENSAO_NAME "produtos_extensao.bin"
#define USUARIOS_EXTENSAO_NAME "usuarios_extensao.bin"

Registro_Extensao<Produto> EXTENSAO_PRODUTOS[TAMANHO_EXTENSAO] = {0, -1, -1};
Registro_Extensao<Usuario> EXTENSAO_USUARIOS[TAMANHO_EXTENSAO] = {0, -1, -1};

int extensao_count_produtos = 0;
int extensao_count_usuarios = 0;

template <typename Registro>
void recriar_arquivos(std::FILE* dados_arq, std::FILE* indice_arq, Registro_Extensao<Registro>* EXTENSAO);

template <typename Registro>
void salvar_extensao(std::string file_name, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count);

template <typename Registro>
void recuperar_extensao(std::string file_name, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count);

template <typename Registro>
int insert_extension(Registro* r, int extensao_end, Registro_Extensao<Registro>* EXTENSAO, int &extensao_count);

template <typename Registro>
void insert_register(FILE* dados_arq, FILE* indice_arq, int indice_tamano, Registro dados, Registro_Extensao<Registro>* EXTENSAO, int& count);