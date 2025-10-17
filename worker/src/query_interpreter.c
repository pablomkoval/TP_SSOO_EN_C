#include <query_interpreter.h>

void* ciclo_query_interpreter(){
    while(1){
        int opcode = recibir_opcode(socket_master);
        if(opcode != SOLICITUD_NUEVA_QUERY){
            log_error(logger, "blabla");
            return NULL;
        }
        t_list* recibido = recibir_paquete(socket_master);
        
        int qid = *((int*)list_get(recibido, 0));
        void* nombre_elem = list_get(recibido, 1);
        void* pc = *((int*)list_get(recibido, 2));
        char* nombre_archivo = strdup((char*)nombre_elem);

        ciclo_ejecucion(nombre_archivo, pc, qid);
        list_destroy_and_destroy_elements(recibido, free);
    }
    return NULL;
}

void ciclo_ejecucion(char* nombre_archivo, int pc, int qid){
    while(1){
        query_t* query_a_ejecutar = leer_query(nombre_archivo, pc);
        ejecutar_query(query_a_ejecutar, qid);
        //chequear interrupcion 


        //aguardar respuesta siempre, todas las instrucciones son bloqueantes
        int respuesta = recibir_opcode(socket_storage);
        manejar_respuesta(socket_storage);

        //añadir retardo antes de volver a empezar?
        pc++;
    }
}
query_t* leer_query(char* nombre_archivo, int pc){
    
    char* path_completo = string_from_format("%s%s", path_queries, nombre_archivo);
    log_debug(logger, "El Archivo queda (%s)", path_completo);
    FILE* archivo = fopen( path_completo, "r");
    free(path_completo);
    //t_list* lista_queries = list_create();

    if (!archivo) 
    {
        log_error(logger, "No se logro abrir el archivo");
        query_t* vacio = {0};
        return vacio;
    }

    char buffer[256];
    int linea_actual = 0;
    query_t* query = NULL;

    while(fgets(buffer, sizeof(buffer), archivo)){
        log_debug(logger, "linea actual (%d), pc (%d)", linea_actual, pc);
        if(linea_actual == pc){
            buffer[strcspn(buffer, "\n")] = 0; // eliminar \n
            query = parsear_query(buffer);
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

query_t* parsear_query(char* query_raw){

    query_t* query = malloc(sizeof(query_t));
    query->file_tag = NULL;
    query->param1 = NULL;
    query->param2 = NULL;

    char **separado = string_split(query_raw, " ");

    int cant_param = 0;
    while(separado[cant_param] != NULL) cant_param++;
    
    query->identificador = parsear_query_id(separado[0]);

    if(cant_param > 1){
        query->file_tag = separado[1];
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


void ejecutar_query(query_t* query, int qid){
    char** partes = separar_file_tag(query->file_tag);
    char* file = strdup(partes[0]);
    char* tag = strdup(partes[1]);
    string_array_destroy(partes);

    switch(query->identificador){
        case -1:
            log_error(logger, "No se recibio query de master: Conexion cerrada");
            break;

        case CREATE_Q:
            log_info(logger, "se quiso ejecutar un create");
            ejecutar_create(file, tag, qid);
            break;
        
        case TRUNCATE_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;
        
        case WRITE_Q:
            log_info(logger, "se quiso ejecutar un WRITE");
            ejecutar_write(query->file_tag, query->param1, query->param2, qid);
            break;

        case READ_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;

        case TAG_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;
            
        case COMMIT_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;

        case FLUSH_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;

        case DELETE_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;

        case END_Q:
            log_info(logger, "se quiso ejecutar un ");
            break;
        
        default:
            log_error(logger, "Error al recibir el query por parte de master");
            break;
    }
}

char** separar_file_tag(char* file_tag){
    char** partes = string_split(file_tag, ":");
    return partes;
}

void manejar_respuesta(int respuesta){

}