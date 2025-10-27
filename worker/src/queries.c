#include <queries.h>

void ejecutar_create(char* file, char* tag, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, CREATE);

    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_truncate(char* file, char* tag, int tamanio, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, TRUNCATE);
    
    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);
    agregar_a_paquete(paquete, &tamanio, sizeof(int));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_write(char* file_tag, char* direccion_base_str, char* contenido, int qid){
    int direccion_base = atoi(direccion_base_str);
    int bytes_restantes = strlen(contenido);
    int direccion_actual = direccion_base;
    int bytes_escritos = 0;

    while(bytes_restantes > 0){
        int pagina_logica = obtener_pagina_logica(direccion_actual);
        int offset = obtener_offset_pagina(direccion_actual);

        pagina_t* pag = obtener_pagina(file_tag, pagina_logica, qid);

        log_debug(logger, "##DEBUG: OBTUVO PAGINA");

        int faltante_pagina = tam_pagina - offset;
        int cant_escritura;
        if(faltante_pagina < bytes_restantes){
            cant_escritura = faltante_pagina;
        }else{
            cant_escritura = bytes_restantes;
        }

        void* destino = memoria_interna + pag->frame * tam_pagina + offset;

        memcpy(destino, contenido + bytes_escritos, cant_escritura);

        pag->bit_modificado = true;

        bytes_restantes -= cant_escritura;
        bytes_escritos += cant_escritura;
        direccion_actual += cant_escritura;
    }
    log_info(logger, "WRITE de %s desde %d (%d bytes): '%s'",
             file_tag, direccion_base, (int)strlen(contenido), contenido);
    return;
}


void ejecutar_read(char* file_tag, int direccion_base, int tamanio, int qid){
    int bytes_restantes = tamanio;
    char* buffer = malloc(tamanio);
    int direccion_actual = direccion_base;
    int bytes_leidos = 0;

    while(bytes_restantes > 0){
        int pagina_logica = obtener_pagina_logica(direccion_actual);
        int offset = obtener_offset_pagina(direccion_actual);

        pagina_t* pag = obtener_pagina(file_tag, pagina_logica, qid);

        int faltante_pagina = tam_pagina - offset;
        int cant_lectura;
        if(faltante_pagina < bytes_restantes){
            cant_lectura = faltante_pagina;
        }else{
            cant_lectura = bytes_restantes;
        }

        void* inicio = memoria_interna + pag->frame * tam_pagina + offset;

        memcpy(buffer + bytes_leidos, inicio, cant_lectura);

        bytes_restantes -= cant_lectura;
        bytes_leidos += cant_lectura;
        direccion_actual += cant_lectura;
    }
    //mandarselo a master
    return;
}

void ejecutar_tag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, TAG);

    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file_origen, strlen(file_origen) + 1);
    agregar_a_paquete(paquete, tag_origen, strlen(tag_origen) + 1);
    agregar_a_paquete(paquete, file_destino, strlen(file_destino) + 1);
    agregar_a_paquete(paquete, tag_destino, strlen(tag_destino) + 1);

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_flush(char* file, char* tag, char* file_tag, int qid){
    tabla_paginas_t* tabla = obtener_tabla(file_tag);
    //falta agregarlo antes de realizar el desalojo del query del worker

    if(!tabla) return;

    for(int i=0; i < list_size(tabla->paginas); i++){
        pagina_t* pag = list_get(tabla->paginas, i);

        if(pag->bit_presencia && pag->bit_modificado){
            hacer_flush_de_pagina(file, tag, pag->nro_pagina, pag->frame, qid);
            pag->bit_modificado = false;
        }
    }
}

void ejecutar_commit(char* file, char* tag, char* file_tag, int qid){

    ejecutar_flush(file, tag, file_tag, qid);

    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, COMMIT);
    
    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file, strlen(file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_delete(char* nombre_file, char* tag, int qid){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, DELETE);

    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, nombre_file, strlen(nombre_file) + 1);
    agregar_a_paquete(paquete, tag, strlen(tag) + 1);

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_end(){
    //finalizar query
    //provocar interrupcion(mediante una variable quizas(?))
    enviar_cod_op(socket_master, END);
}