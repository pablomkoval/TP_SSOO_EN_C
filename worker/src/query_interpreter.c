#include <query_interpreter.h>


void* iniciar_query_interpreter(void* args){
    //hacer un chequeo de que este hilo esta ejecutando antes de recibir una nueva solicitud
    t_args_query_interpreter* argumentos = (t_args_query_interpreter*) args;
    int pc = argumentos->pc;
    int qid = argumentos->qid;
    char* nombre_archivo = strdup(argumentos->archivo);
    free(argumentos->archivo);
    free(args);
    //en vez de un while 1 simplemente se llama a esta funcion cuando llega una solicitud nueva query de master al hilo de conexiones!!!!
    

    log_info(logger, "## Query %d: Se recibe la Query. El path de operaciones es: %s", qid, nombre_archivo);
    ciclo_ejecucion(nombre_archivo, pc, qid);
    free(nombre_archivo);
    
    // pthread_mutex_lock(&mutex_interpreter);
    // interpreter_ocupado = false;
    // pthread_mutex_unlock(&mutex_interpreter);

    return NULL;
}

void ciclo_ejecucion(char* nombre_archivo, int pc, int qid){
    int resultado_ejecucion = 1;
    while(resultado_ejecucion > 0){// > 0
        //fetch
        char* instruccion = NULL;
        query_t* query_a_ejecutar = leer_query(nombre_archivo, pc, &instruccion);
        log_info(logger, "## Query %d: FETCH - Program Counter: %d - %s", qid, pc, instruccion);
        
        //decode + execute
        resultado_ejecucion = ejecutar_query(query_a_ejecutar, qid);

        if(resultado_ejecucion == -2){
            log_warning(logger, "## Query %d: - Desalojada por error en storage al ejecutar: %s", qid, instruccion);
            free(query_a_ejecutar);
            free(instruccion);
            return; // caso para errores
        }

        //free(query_a_ejecutar->file_tag);
        
        if(resultado_ejecucion == -1) {
            free(instruccion);
            free(query_a_ejecutar);
            return; // caso para END 
        }

        //aguardar respuesta siempre, todas las instrucciones son bloqueantes
        log_debug(logger, "Resultado ejecucion = %d", resultado_ejecucion);
        if(resultado_ejecucion == 1){
            int opcode = recibir_opcode(socket_storage);
            if(opcode == RESPUESTA_STORAGE){
                t_list* recibido = recibir_paquete(socket_storage);
                int respuesta = *((int*)list_get(recibido, 0));
                resultado_ejecucion = manejar_respuesta(respuesta);
                list_destroy_and_destroy_elements(recibido, free);
                if(resultado_ejecucion < 0){
                    log_error(logger, "## Query %d: - Desalojada por error en storage al ejecutar: %s", qid, instruccion);
                    free(instruccion);
                    free(query_a_ejecutar);
                    return;
                }else{
                    log_info(logger, "## Query %d: - Instrucción realizada: %s", qid, instruccion);
                    free(instruccion);
                    free(query_a_ejecutar);
                }
            } else{
                log_error(logger, "Opcode: %d", opcode);
                free(instruccion);
                free(query_a_ejecutar);
            }
        } else{
            log_debug(logger, "No se espera respuesta de storage, Resultado de Ejecucion. %d", resultado_ejecucion);
            free(instruccion);
            free(query_a_ejecutar);
        }
        
        
        resultado_ejecucion = check_interrupt(qid, pc);
        //verifica si llego una interrupción
        pc++;
    }
}
query_t* leer_query(char* nombre_archivo, int pc, char** instruccion){
    
    char* path_completo = string_from_format("%s%s", path_queries, nombre_archivo);
    //log_debug(logger, "El Archivo queda (%s)", path_completo);
    FILE* archivo = fopen( path_completo, "r");
    free(path_completo);

    if (!archivo) 
    {
        log_error(logger, "No se logró abrir el archivo");
        return NULL;
    }

    char buffer[256];
    int linea_actual = 0;
    query_t* query = NULL;

    while(fgets(buffer, sizeof(buffer), archivo)){
        //log_trace(logger, "linea actual (%d), pc (%d)", linea_actual, pc);
        if(linea_actual == pc){
            buffer[strcspn(buffer, "\n")] = 0; // eliminar \n
            query = parsear_query(buffer, instruccion);
            fclose(archivo);
            return query;
        }
        linea_actual++;
    }
    //log_error(logger, "PC=%d linea=(%s) len=%d", linea_actual, buffer, strlen(buffer));
    log_debug(logger, "No paso por la linea del pc %d", pc);
    fclose(archivo);
    return query;
}

id_query_t parsear_query_id(char* identificador) {
    if (strcmp(identificador, "CREATE") == 0) return CREATE_Q;
    if (strcmp(identificador, "TRUNCATE") == 0) return TRUNCATE_Q;
    if (strcmp(identificador, "WRITE") == 0) return WRITE_Q;
    if (strcmp(identificador, "READ") == 0) return READ_Q;
    if (strcmp(identificador, "TAG") == 0) return TAG_Q;
    if (strcmp(identificador, "COMMIT") == 0) return COMMIT_Q;
    if (strcmp(identificador, "FLUSH") == 0) return FLUSH_Q;
    if (strcmp(identificador, "DELETE") == 0) return DELETE_Q;
    if (strcmp(identificador, "END") == 0) return END_Q;

    log_error(logger, "la query que llego no es valida");
    return END_Q;
}

query_t* parsear_query(char* query_raw, char** instruccion){

    query_t* query = malloc(sizeof(query_t));
    query->file_tag = NULL;
    query->param1 = NULL;
    query->param2 = NULL;

    log_trace(logger, "La query raw es: %s", query_raw);
    char **separado = string_split(query_raw, " ");

    int cant_param = 0;
    while(separado[cant_param] != NULL) cant_param++;

    *instruccion = strdup(separado[0]);
    query->identificador = parsear_query_id(separado[0]);


    if(cant_param > 1){
        query->file_tag = strdup(separado[1]);
    }
    if(cant_param > 2){
        query->param1 = strdup(separado[2]);
    }
    if(cant_param > 3){
        query->param2 = strdup(separado[3]);
    }
    
    string_array_destroy(separado);
    return query;
}


int ejecutar_query(query_t* query, int qid){
    if(query->identificador == END_Q){
        log_debug(logger, "##DEBUG: Se esta por ejecutar un END");
        ejecutar_end();
        return -1;
    }
    char** partes = separar_file_tag(query->file_tag);
    char* file = strdup(partes[0]);
    char* tag = strdup(partes[1]);
    string_array_destroy(partes);
    int resultado;

    switch(query->identificador){
        case -1:
            log_error(logger, "No se recibio query de master: Conexion cerrada");
            return -2;
            break;

        case CREATE_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un CREATE");
            ejecutar_create(file, tag, qid);
            break;
        
        case TRUNCATE_Q:
        
            log_debug(logger, "##DEBUG: Se esta por ejecutar un TRUNCATE");
            ejecutar_truncate(file, tag, atoi(query->param1), qid);
            free(query->param1);
            break;
        
        case WRITE_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un WRITE");
            resultado = ejecutar_write(query->file_tag, atoi(query->param1), query->param2, qid);
            free(query->param1);
            free(query->file_tag);
            free(query->param2);
            free(file);
            free(tag);
            return resultado;
            break;

        case READ_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un READ");
            resultado = ejecutar_read(query->file_tag, atoi(query->param1), atoi(query->param2), qid);
            free(query->param1);
            free(query->param2);
            free(query->file_tag);
            free(file);
            free(tag);
            return resultado;
            break;

        case TAG_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un TAG");
            ejecutar_tag(file, tag, query->param1, qid);
            free(query->param1);
            break;
            
        case COMMIT_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un COMMIT");
            resultado = ejecutar_commit(file, tag, query->file_tag, qid);
            free(query->file_tag);
            free(file);
            free(tag);
            return resultado;
            break;

        case FLUSH_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un FLUSH");
            resultado = ejecutar_flush(file, tag, query->file_tag, qid);
            free(query->file_tag);
            free(file);
            free(tag);
            return resultado;
            break;

        case DELETE_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un DELETE");
            ejecutar_delete(file, tag, qid);
            break;
        
        default:
            log_error(logger, "Error al recibir el query por parte de master");
            return -2;
            break;
    }
    free(file);
    free(tag);
    free(query->file_tag);
    return 1;
}


int check_interrupt(int qid, int pc){
    log_trace(logger, "Intento abrir mutex interrupcion");
    pthread_mutex_lock(&mutex_interrupcion);
    if(hay_interrupcion){
        log_info(logger, "## Query %d: Desalojada por pedido del Master", qid);
        hay_interrupcion = false;
        pthread_mutex_unlock(&mutex_interrupcion);

        //flushear todas las paginas en memoria
        pthread_mutex_lock(&mutex_paginas_en_memoria);
        for(int i = 0; i < list_size(paginas_en_memoria); i++){
            pagina_t* pag = list_get(paginas_en_memoria, i);
            
            if(pag->bit_modificado){
                char** partes = separar_file_tag(pag->file_tag);
                char* file = strdup(partes[0]);
                char* tag = strdup(partes[1]);
                string_array_destroy(partes);
            
                hacer_flush_de_pagina(file, tag, pag->nro_pagina, pag->frame, qid);
                pag->bit_modificado = false;
                free(file);
                free(tag);
            }
        }
        pthread_mutex_unlock(&mutex_paginas_en_memoria);

        pthread_mutex_lock(&mutex_interpreter);
        interpreter_ocupado = false;
        pthread_mutex_unlock(&mutex_interpreter);

        t_paquete* contestacion = crear_paquete();
        cambiar_opcode_paquete(contestacion, INTERRUPCION_RTA);
        agregar_a_paquete(contestacion, &qid, sizeof(int));
        agregar_a_paquete(contestacion, &pc, sizeof(int));
        enviar_paquete(contestacion, socket_master, logger);
        borrar_paquete(contestacion);

        return -1;
    } else {
        log_debug(logger, "Caso else en check interrupt");
        pthread_mutex_unlock(&mutex_interrupcion);
        return 1;
    }
}

char** separar_file_tag(char* file_tag){
    log_trace(logger, "String a separar: %s", file_tag);
    char** partes = string_split(file_tag, ":");
    return partes;
}

char* parsear_errores(int error){
    switch(error){
        case FILE_TAG_PREEXISTENTE:
            return "File_tag preexistente";
            break;
        case FILE_TAG_INEXISTENTE:
            return "File_tag inexistente";
            break;
        case ESPACIO_INSUFICIENTE:
            return "Espacio insuficiente";
            break;
        case ESCRITURA_NO_PERMITIDA:
            return "Escritura no permitida";
            break;
        case LECTURA_O_ESCRITURA_FUERA_DE_RANGO:
            return "Lectura o escritura fuera de rango";
            break;
        default:
            return "Error desconocido";
    }
}

int manejar_respuesta(int respuesta){
    
    if(respuesta != 1){
        log_warning(logger, "## Storage responde con un error, %d", respuesta);
        char* motivo = parsear_errores(respuesta);

        pthread_mutex_lock(&mutex_interpreter);
        interpreter_ocupado = false;
        pthread_mutex_unlock(&mutex_interpreter);

        t_paquete* paquete = crear_paquete();
        cambiar_opcode_paquete(paquete, END);
        agregar_a_paquete(paquete, motivo, strlen(motivo) + 1);
        enviar_paquete(paquete, socket_master, logger);
        borrar_paquete(paquete);
        return -1;
    }else{
        log_debug(logger, "Storage respondio un 1 a la ejecucion");
        return 1;
    }
}