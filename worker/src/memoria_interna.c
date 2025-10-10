#include <memoria_interna.h>

void* memoria_interna;
int cant_frames;
//bitmap de marcos con {cant_frames} entradas
t_dictionary* tablas_de_paginas;


void inicializar_memoria_interna (){
    memoria_interna = malloc(tam_memoria);
    memset(memoria_interna, 0, tam_memoria);

    cant_frames = tam_memoria / tam_pagina;
    dictionary_create()

}

void crear_tabla(char* file_tag){
    tabla_paginas_t* tabla = malloc(sizeof(tabla_paginas_t));
    tabla->paginas = list_create();

    dictionary_put(tablas_de_paginas, file_tag, tabla);
}