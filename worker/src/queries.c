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

void ejecutar_write(char* file_tag, int direccion_base, char* contenido, int qid){
    int bytes_restantes = strlen(contenido);
    int direccion_actual = direccion_base;
    int bytes_escritos = 0;

    int pagina_logica_inicial = obtener_pagina_logica(direccion_base);
    int offset_inicial = obtener_offset_pagina(direccion_base);

    pagina_t* pag_inicial = obtener_pagina(file_tag, pagina_logica_inicial, qid);
    int direccion_fisica_inicial = pag_inicial->frame * tam_pagina + offset_inicial;

    char* escrito = malloc(bytes_restantes + 1);

    while(bytes_restantes > 0){
        int pagina_logica = obtener_pagina_logica(direccion_actual);
        int offset = obtener_offset_pagina(direccion_actual);

        log_trace(logger, "Write -> file_tag:(%s)", file_tag);
        pagina_t* pag = obtener_pagina(file_tag, pagina_logica, qid);

        log_debug(logger, "##DEBUG: OBTUVO PAGINA");

        int faltante_pagina = tam_pagina - offset;

        int cant_escritura;
        if(faltante_pagina < bytes_restantes){
            cant_escritura = faltante_pagina;
        }else{
            cant_escritura = bytes_restantes;
        }

        int direccion_fisica = pag->frame * tam_pagina + offset;
        void* destino = memoria_interna + direccion_fisica;

        memcpy(destino, contenido + bytes_escritos, cant_escritura);
        memcpy(escrito + bytes_escritos, contenido + bytes_escritos, cant_escritura);

        pag->bit_modificado = true;

        bytes_restantes -= cant_escritura;
        bytes_escritos += cant_escritura;
        direccion_actual += cant_escritura;
    }

    escrito[bytes_escritos] = '\0';

    log_debug(logger, "WRITE de %s desde %d (%d bytes): '%s'",
             file_tag, direccion_base, (int)strlen(contenido), contenido);

    log_info(logger, "Query %d: Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", qid, direccion_fisica_inicial, escrito);

    free(escrito);
    return;
}


void ejecutar_read(char* file_tag, int direccion_base, int tamanio, int qid){
    int bytes_restantes = tamanio;
    char* buffer = malloc(tamanio + 1);
    int direccion_actual = direccion_base;
    int bytes_leidos = 0;

    int pagina_logica_inicial = obtener_pagina_logica(direccion_base);
    int offset_inicial = obtener_offset_pagina(direccion_base);
    pagina_t* pag_inicial = obtener_pagina(file_tag, pagina_logica_inicial, qid);
    int direccion_fisica_inicial = pag_inicial->frame * tam_pagina + offset_inicial;

    log_debug(logger,
        "READ(qid=%d) INICIO: base=%d tamanio=%d | pagina_log=%d offset=%d frame=%d dir_fisica_ini=%d",
        qid, direccion_base, tamanio,
        pagina_logica_inicial, offset_inicial,
        pag_inicial->frame, direccion_fisica_inicial
    );

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

        int direccion_fisica = pag->frame * tam_pagina + offset;

        log_debug(logger,
            "[READ paso] qid=%d | pagina=%d frame=%d offset=%d "
            "| dir_fisica=%d cant_lectura=%d bytes_restantes=%d bytes_leidos=%d",
            qid, pagina_logica, pag->frame, offset,
            direccion_fisica, cant_lectura, bytes_restantes, bytes_leidos
        );

        memcpy(buffer + bytes_leidos, memoria_interna + direccion_fisica, cant_lectura);

        char parcial[bytes_leidos + cant_lectura + 1];
        memcpy(parcial, buffer, bytes_leidos + cant_lectura);
        parcial[bytes_leidos + cant_lectura] = '\0'; // Seguro para textos
        log_debug(logger, "Contenido parcial (string-safe): %s", parcial);


        bytes_restantes -= cant_lectura;
        bytes_leidos += cant_lectura;
        direccion_actual += cant_lectura;
    }
    buffer[bytes_leidos] = '\0';
    log_info(logger, "Query %d: Acción: LEER - Dirección Física: %d - Valor: %s", qid, direccion_fisica_inicial, buffer);

    char printable[bytes_leidos + 1];
    memcpy(printable, buffer, bytes_leidos);
    printable[bytes_leidos] = '\0';
    log_debug(logger,
        "READ(qid=%d) FIN: total_leido=%d | ValorFinal(string-safe): %s",
        qid, bytes_leidos, printable
    );

    

    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, READ);
    agregar_a_paquete(paquete, file_tag, strlen(file_tag) + 1);
    agregar_a_paquete(paquete, buffer, bytes_leidos + 1);
    enviar_paquete(paquete, socket_master, logger);


    
    borrar_paquete(paquete);
    free(buffer);
    return;
}

void ejecutar_tag(char* file_origen, char* tag_origen, char* file_tag_destino, int qid){
    char** partes = separar_file_tag(file_tag_destino);
    char* file_destino = strdup(partes[0]);
    char* tag_destino = strdup(partes[1]);
    
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, TAG);

    agregar_a_paquete(paquete, &qid, sizeof(int));
    agregar_a_paquete(paquete, file_origen, strlen(file_origen) + 1);
    agregar_a_paquete(paquete, tag_origen, strlen(tag_origen) + 1);
    agregar_a_paquete(paquete, file_destino, strlen(file_destino) + 1);
    agregar_a_paquete(paquete, tag_destino, strlen(tag_destino) + 1);

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
    string_array_destroy(partes);
    free(file_destino);
    free(tag_destino);
    return;
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
    pthread_mutex_lock(&mutex_interpreter);
    interpreter_ocupado = false;
    pthread_mutex_unlock(&mutex_interpreter);
    
    char* motivo = "FIN";
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, END);
    agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
    enviar_paquete(paquete, socket_master, logger);
    borrar_paquete(paquete);
    printf("Enviando FIN: size=%d\n", (int)strlen(motivo)+1);
    //enviar_cod_op(socket_master, END);
}