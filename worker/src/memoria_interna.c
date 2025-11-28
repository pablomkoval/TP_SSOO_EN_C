#include <memoria_interna.h>

void* memoria_interna;
int cant_frames;
t_bitarray* bitmap_frames = NULL;
t_dictionary* tablas_de_paginas;
int contador_lru = 0;
int puntero_clock = 0;
t_list* paginas_en_memoria;

void inicializar_memoria_interna (){
    memoria_interna = malloc(tam_memoria);
    memset(memoria_interna, 0, tam_memoria);

    tablas_de_paginas = dictionary_create();
    paginas_en_memoria = list_create();

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
        pag->bit_uso = false;
        pag->frame = -1;
        pag->file_tag = strdup(file_tag);
        list_add(tabla->paginas, pag);
        log_debug(logger, "##DEBUG: cuando se creo la pagina");
    }

    if(pag && pag->bit_presencia){
        usleep(retardo_memoria * 1000);
        pag->bit_uso = true;
        pag->timestamp = contador_lru++;
        return pag;
    }

    if(pag->bit_presencia == false){
        log_trace(logger, "File_tag: (%s), pag-file_tag: (%s)", file_tag, pag->file_tag);
        char** separado = separar_file_tag(file_tag);
        char* file = separado[0];
        char* tag = separado[1];

        log_info(logger, "Query %d: - Memoria Miss - File: %s - Tag: %s - Pagina: %d", qid, file, tag, nro_pagina);
        int frame = buscar_frame_libre();
        if(frame == -1){
            log_debug(logger, "##DEBUG: antes de usar algoritmo de reemplazo");
            pagina_t* victima = buscar_victima_reemplazo();
            frame = liberar_frame(victima, qid);
            log_info(logger, "## Query %d: Se reemplaza la página %s/%d por la %s/%d", qid, victima->file_tag, victima->nro_pagina, pag->file_tag, pag->nro_pagina);
        }
        
        cargar_pagina_de_storage(file_tag, file, tag, nro_pagina, frame, qid);
        log_info(logger, "Query %d: - Memoria Add - File: %s - Tag: %s - Pagina: %d - Marco: %d", qid, file, tag, nro_pagina, frame);
        log_debug(logger, "##DEBUG: despues de cargar pagina de storage");
        pag->bit_presencia = true;
        pag->bit_uso = true;
        pag->timestamp = contador_lru++;
        pag->frame = frame;
        pag->bit_modificado = false;
        list_add(paginas_en_memoria, pag);
        
        log_info(logger, "Query %d: Se asigna el Marco: %d a la Página: %d perteneciente al - File: %s - Tag: %s", qid, frame, nro_pagina, file, tag);
        string_array_destroy(separado);
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

pagina_t* buscar_victima_reemplazo(){
    pagina_t * victima = NULL;

    if(strcmp(algoritmo_reemplazo, "LRU") == 0){

        for(int i = 0; i < list_size(paginas_en_memoria); i++){
            pagina_t* pag = list_get(paginas_en_memoria, i);

            if(victima == NULL || pag->timestamp < victima->timestamp) victima = pag;
        }
        if(victima) 
        log_debug(logger, "Reemplazo LRU eligio frame %d (pagina %d)",
         victima->frame, victima->nro_pagina);

    } else if(strcmp(algoritmo_reemplazo, "CLOCK-M") == 0){
        int cant_pags = list_size(paginas_en_memoria);

        while(1){
            for(int vuelta = 0; vuelta < 2; vuelta++){

                for(int i = 0; i < cant_pags; i++){
                    pagina_t* pag = list_get(paginas_en_memoria, puntero_clock);

                    log_trace(logger, "Evaluando frame %d: U=%d, M=%d (vuelta %d)",
                     pag->frame, pag->bit_uso, pag->bit_modificado, vuelta);

                    if (vuelta == 0) {
                        // Primera vuelta: buscamos una pagina (U=0, M=0)
                        if (!pag->bit_uso && !pag->bit_modificado) {
                            victima = pag;
                            log_debug(logger, "CLOCK-M eligió frame %d (página %d) [U=0,M=0]",
                             pag->frame, pag->nro_pagina);
                            puntero_clock = (puntero_clock + 1) % cant_pags;
                            return victima;
                        }
                    } else {
                        // Segunda vuelta: buscamos (U=0, M=1)
                        if (!pag->bit_uso && pag->bit_modificado) {
                            victima = pag;
                            log_debug(logger, "CLOCK-M eligio frame %d (página %d) [U=0,M=1]",
                             pag->frame, pag->nro_pagina);

                            puntero_clock = (puntero_clock + 1) % cant_pags;

                            return victima;
                        }
                        pag->bit_uso = false;
                    }
                    puntero_clock = (puntero_clock + 1) % cant_pags;
                }
            }
            log_debug(logger, "CLOCK-M no encontro victima en 2 vueltas, repitiendo bucle");

        }
    }

    log_error(logger, "El file tag de la victima seleccionada es: (%s)", victima->file_tag);
    return victima;
}

int liberar_frame(pagina_t* victima, int qid){
    if (victima == NULL) {
        log_error(logger, "Intento de reemplazo con víctima NULL");
        return -1;
    }

    if (victima->bit_modificado) {
        log_trace(logger, "file tag: %s", victima->file_tag);
        char** separado = separar_file_tag(victima->file_tag);
        hacer_flush_de_pagina(separado[0], separado[1], victima->nro_pagina, victima->frame, qid);
        log_info(logger, "Query %d: Se libera el Marco: %d perteneciente al - File: %s - Tag: %s", qid, victima->frame, separado[0], separado[1]);
        string_array_destroy(separado);
        victima->bit_modificado = false;
    }
    bitarray_clean_bit(bitmap_frames, victima->frame);
    victima->bit_presencia = false;
    list_remove_element(paginas_en_memoria, victima);

    return victima->frame;
}

void cargar_pagina_de_storage(char* file_tag, char* file, char* tag, int nro_pagina, int frame, int qid){
    log_debug(logger, "qid: %d file: %s, tag: %s", qid, file, tag);
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, READ);
    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &nro_pagina, sizeof(int));
    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);

    int opcode = recibir_opcode(socket_storage);
    if(recibir_opcode(socket_storage) != RESPUESTA_STORAGE){
        log_debug(logger, "No se recibio respuesta de storage");
        return;
    } 

    t_list* recibido = recibir_paquete(socket_storage);
    if (!recibido || list_size(recibido) < 2) {
        log_error(logger, "Se recibieron menos de 2 cosas de storage Resultado(%d)", *(int*)list_get(recibido, 0));
        memset(memoria_interna + frame * tam_pagina, 0, tam_pagina);//pongo la pagina en 0
        int resultado = *((int*)list_get(recibido, 0));
        manejar_respuesta(resultado);
        list_destroy_and_destroy_elements(recibido, free);
        return;
    }
    int* resultado = (int*)list_get(recibido, 0);

    if (!resultado || *resultado != 1) {
        log_error(logger, "Fallo al leer pagina de storage");
        list_destroy_and_destroy_elements(recibido, free);
        return;
    }
    
    char* contenido_raw = list_get(recibido, 1);

    if(!contenido_raw){
        log_error(logger, "Storage me mando contenido NULL");
        list_destroy_and_destroy_elements(recibido, free);
        memset(memoria_interna + frame * tam_pagina, 0, tam_pagina);
        return;
    }
    if(strlen(contenido_raw) < tam_pagina){
        log_trace(logger, "El contenido raw es menor a una pagina");
        list_destroy_and_destroy_elements(recibido, free);
        memset(memoria_interna + frame * tam_pagina, 0, tam_pagina);
        return;
    }
    char* contenido = strdup(contenido_raw); 
    //averiguar si el memset es correcto
    log_error(logger, "El contenido a insertar en memoria es (%s)", contenido);
    //memset(memoria_interna + frame * tam_pagina, 0, tam_pagina);// limpio la pagina vieja antes de traer el contenido nuevo
    memcpy(memoria_interna + frame * tam_pagina, contenido, tam_pagina); 
    list_destroy_and_destroy_elements(recibido, free);
    return;
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
    //log_debug(logger, "##EL CONTENIDO ANTES DEL FLUSH ES: %s, pagina a escribir: %d", (char*)contenido, nro_pagina);
    //este log genera segfault ya que contenido es una porcion de memoria y no un char* legible
    char* copia = malloc(tam_pagina + 1);
    memcpy(copia, contenido, tam_pagina);
    copia[tam_pagina] = '\0';  // asegurar terminación

    log_debug(logger, "Contenido previo al flush:\n%s", copia);

    free(copia);


    enviar_paquete(paquete, socket_storage,logger);
    borrar_paquete(paquete);
    int opcode = recibir_opcode(socket_storage);
    if(opcode == RESPUESTA_STORAGE){
        t_list* recibido = recibir_paquete(socket_storage);
        int respuesta = *((int*)list_get(recibido, 0));
        manejar_respuesta(respuesta);
        list_destroy_and_destroy_elements(recibido, free);
    }else{
        log_error(logger, "En vez de una RESPUESTA STORAGE se obtuvo el opcode: %d", opcode);
    }
    return;
}

int obtener_pagina_logica(int direccion_logica){
    return direccion_logica / tam_pagina;
}

int obtener_offset_pagina(int direccion_logica) {
    return direccion_logica % tam_pagina;
}
