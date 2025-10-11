#include <memoria_interna.h>

void* memoria_interna;
int cant_frames;
t_bitarray* bitmap_frames = NULL;
t_dictionary* tablas_de_paginas;


void inicializar_memoria_interna (){
    memoria_interna = malloc(tam_memoria);
    memset(memoria_interna, 0, tam_memoria);

    tablas_de_paginas = dictionary_create();

    cant_frames = tam_memoria / tam_pagina;
    int tam_bitmap = (cant_frames + 7) / 8;//

    
    char* bitarray = calloc(tam_bitmap, sizeof(char));
    bitmap_frames = bitarray_create(bitarray, tam_bitmap);
}

tabla_paginas_t* crear_tabla(char* file_tag){
    tabla_paginas_t* tabla = malloc(sizeof(tabla_paginas_t));
    tabla->paginas = list_create();

    dictionary_put(tablas_de_paginas, strdup(file_tag), tabla);
    return tabla;
}

tabla_paginas_t* obtener_tabla(char* file_tag){
    tabla_paginas_t* tabla = dictionary_get(tablas_de_paginas, file_tag);

    if(tabla == NULL){
        tabla = crear_tabla(file_tag);
    }
    return tabla;
}

pagina_t* buscar_pagina(tabla_paginas_t* tabla, int nro_pagina) {
    for (int i = 0; i < list_size(tabla->paginas); i++) {
        pagina_t* pag = list_get(tabla->paginas, i);
        if (pag->nro_pagina == nro_pagina) return pag;
    }
    return NULL;
}


pagina_t* obtener_pagina(char* file_tag, int nro_pagina){
    tabla_paginas_t* tabla = obtener_tabla(file_tag);
    pagina_t* pag = buscar_pagina(tabla, nro_pagina);

    if(pag == NULL){
        pag = malloc(sizeof(pagina_t));
        pag->nro_pagina = nro_pagina;
        pag->bit_modificado = false;
        pag->bit_presencia = false;
        pag->frame = -1;
        list_add(tabla->paginas, pag);
    }

    if(pag && pag->bit_presencia){
        usleep(retardo_memoria * 1000);
        return pag;
    }

    if(pag->bit_presencia == false){
        int frame = buscar_frame_libre();
        if(frame == -1){
            frame = buscar_victima_reemplazo();
        }
        cargar_pagina_de_storage(file_tag, nro_pagina, frame);
        pag->bit_presencia = true;
        pag->frame = frame;
        pag->bit_modificado = false;
        return pag;
    }
    return pag;
}

int buscar_frame_libre(){
    for(int i = 0; i < cant_frames; i++){
        if(!bitarray_test_bit(bitmap_frames, i)){
            bitarray_set_bit(bitmap_frames, i);
            return i;
        }
    }
    return -1;
}

int buscar_victima_reemplazo(){
    
    return 0;
}

void cargar_pagina_de_storage(char* file_tag, int nro_pagina, int frame){
    //placeholder para que despues reciba de memoria informacion real
    int contenido = 0;
    memset(memoria_interna + frame * tam_pagina, contenido, tam_pagina);
}

void hacer_flush_de_pagina(){
    return;
}

int obtener_pagina_logica(int direccion_logica){
    return direccion_logica / tam_pagina;
}

int obtener_offset_pagina(int direccion_logica) {
    return direccion_logica % tam_pagina;
}