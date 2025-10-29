#include <query_interpreter.h>


void* iniciar_query_interpreter(){
    //en vez de un while 1 simplemente se llama a esta funcion cuando llega una solicitud nueva query de master al hilo de conexiones!!!!
    t_list* recibido = recibir_paquete(socket_master);
        
    int qid = *((int*)list_get(recibido, 0));
    void* nombre_elem = list_get(recibido, 1);
    int pc = *((int*)list_get(recibido, 2));
    char* nombre_archivo = strdup((char*)nombre_elem);

    log_info(logger, "## Query %d: Se recibe la Query. El path de operaciones es: %s", qid, nombre_archivo);
    ciclo_ejecucion(nombre_archivo, pc, qid);
    list_destroy_and_destroy_elements(recibido, free);
    
    return NULL;
}

void ciclo_ejecucion(char* nombre_archivo, int pc, int qid){
    int resultado_ejecucion = 1;
    while(resultado_ejecucion != -1){
        //fetch
        char* instruccion = NULL;
        query_t* query_a_ejecutar = leer_query(nombre_archivo, pc, &instruccion);
        log_info(logger, "## Query %d: FETCH - Program Counter: %d - %s", qid, pc, instruccion);
        
        //decode + execute
        resultado_ejecucion = ejecutar_query(query_a_ejecutar, qid);
        if(resultado_ejecucion == -2) return; // caso para errores extraordinarios 
        log_info(logger, "## Query %d: - Instrucción realizada: %s", qid, instruccion);

        if(resultado_ejecucion == -1) return; // caso para el END


        //aguardar respuesta siempre, todas las instrucciones son bloqueantes
        if(resultado_ejecucion != 2){
            if(recibir_opcode(socket_storage) == RESPUESTA_STORAGE){
                t_list* recibido = recibir_paquete(socket_storage);
                int respuesta = *((int*)list_get(recibido, 0));
                manejar_respuesta(respuesta);
                list_destroy_and_destroy_elements(recibido, free);
            } else return;
        }
        
        
        resultado_ejecucion = check_interrupt(qid);
        //verifica si llego una interrupción
        pc++;
    }
}
query_t* leer_query(char* nombre_archivo, int pc, char** instruccion){
    
    char* path_completo = string_from_format("%s%s", path_queries, nombre_archivo);
    log_debug(logger, "El Archivo queda (%s)", path_completo);
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
    return END;
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
            break;
        
        case WRITE_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un WRITE");
            ejecutar_write(query->file_tag, atoi(query->param1), query->param2, qid);
            return 2;
            break;

        case READ_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un READ");
            ejecutar_read(query->file_tag, atoi(query->param1), atoi(query->param2), qid);
            return 2;
            break;

        case TAG_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un TAG");
            ejecutar_tag(file, tag, query->param1, qid);
            break;
            
        case COMMIT_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un COMMIT");
            ejecutar_commit(file, tag, query->file_tag, qid);
            break;

        case FLUSH_Q:
            log_debug(logger, "##DEBUG: Se esta por ejecutar un FLUSH");
            ejecutar_flush(file, tag, query->file_tag, qid);
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
    return 1;
}


int check_interrupt(int qid){
    pthread_mutex_lock(&mutex_interrupcion);
    if(hay_interrupcion){
        log_info(logger, "## Query %d: Desalojada por pedido del Master", qid);
        return 1;
    } else {
        log_debug(logger, "Caso else en check interrupt");
        return 0;
    }
    pthread_mutex_unlock(&mutex_interrupcion);
}

char** separar_file_tag(char* file_tag){
    log_trace(logger, "String a separar: %s", file_tag);
    char** partes = string_split(file_tag, ":");
    return partes;
}

void manejar_respuesta(int respuesta){

}