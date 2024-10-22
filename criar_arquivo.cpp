#include "main.h"

#define MAX_LINE_LEN 1024
#define MAX_BUFFER_SIZE 100 

int main (){
    FILE* filmes_txt;

    filmes_txt = fopen("2019-Oct.csv", "r");

    char buffer[MAX_LINE_LEN];
    Produto produtos[MAX_BUFFER_SIZE];
    int filme_index = 0;

    while(fgets(buffer, sizeof(buffer), filmes_txt) != NULL){
        char event_time[50] = {0};
        char event_type[10] = {0};
        int product_id = 0;
        int category_id = 0;
        char category_code[100] = {0};
        char brand[30] = {0};
        float price = 0;
        int user_id = 0;
        char user_session[100] = {0};

        sscanf(buffer, "%49[^,],%9[^,],%d,%d,%99[^,],%29[^,],%f,%d,%99[^,]", event_time, event_type, &product_id, &category_id, category_code, brand, &price, &user_id, user_session);

        printf("%s\n", category_code);
    }
    

    fclose(filmes_txt);
    return 0;
}

