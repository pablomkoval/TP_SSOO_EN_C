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
    bitmap_frames = bitarray_create_with_mode(bitarray, tam_bitmap, LSB_FIRST);
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


pagina_t* obtener_pagina(char* file_tag, int nro_pagina, int qid){
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

        char** separado = separar_file_tag(file_tag);
        char* file = separado[0];
        char* tag = separado[1];
        string_array_destroy(separado);
        cargar_pagina_de_storage(file_tag, file, tag, nro_pagina, frame, qid);
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

void cargar_pagina_de_storage(char* file_tag, char* file, char* tag, int nro_pagina, int frame, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, READ);
    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &nro_pagina, sizeof(int));
    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);

    if(recibir_opcode(socket_storage) != READ) return;

    t_list* recibido = recibir_paquete(socket_storage);
    char* contenido = list_get(recibido, 0);

    memset(memoria_interna + frame * tam_pagina, 0, tam_pagina);// limpio la pagina vieja antes de traer el contenido nuevo
    memcpy(memoria_interna + frame * tam_pagina, contenido, tam_pagina);

    list_destroy_and_destroy_elements(recibido, free);
}

void hacer_flush_de_pagina(char* file, char* tag, int nro_pagina, int frame, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, WRITE);
    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &nro_pagina, sizeof(int));

    void* contenido = memoria_interna + frame * tam_pagina;
    agregar_a_paquete(paquete, contenido, tam_pagina);

    enviar_paquete(paquete, socket_storage,logger);
    borrar_paquete(paquete);
    return;
}

int obtener_pagina_logica(int direccion_logica){
    return direccion_logica / tam_pagina;
}

int obtener_offset_pagina(int direccion_logica) {
    return direccion_logica % tam_pagina;
}

void liberar_frame(int frame){
    bitarray_clean_bit(bitmap_frames, frame);
}