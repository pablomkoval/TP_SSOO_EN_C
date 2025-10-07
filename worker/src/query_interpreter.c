#include <query_interpreter.h>

void* ciclo_query_interpreter(){
    while(1){
        int opcode = recibir_opcode(socket_master);
        if(opcode != SOLICITUD_NUEVA_QUERY){
            log_error(logger, "blabla");
            return NULL;
        }
        t_list* archivo_recibido = recibir_paquete(socket_memoria);
        void* nombre_elem = list_get(recibido, 0);
        void* pc_elem = list_get(recibido, 1);
        char* nombre_archivo = strdup((char*)nombre_elem);
        int pc = *((int*)pc_elem);

        leer_query();
        //chequear interrupcion 
        list_destroy_and_destroy_elements(recibido, free);
    }
}

query_t leer_query(char* nombre_archivo, int pc){
    
    char* path_completo = string_from_format("%s%s", path_queries, nombre_archivo);
    FILE* archivo = fopen( path_completo, "r");
    free(path_completo);
    //t_list* lista_queries = list_create();

    char* buffer;
    int linea_actual;

    while(fgets(buffer, sizeof(buffer), archivo)){
        if(linea_actual == pc){
            buffer[strcspn(linea, "\n")] = 0; // eliminar \n
            fclose(archivo);
            int query = parsear_query(buffer);
        }
        linea_actual++;
    }
    
    fclose(archivo);
    return NULL;
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


    // if (cant_param > 1){
    //     //aca iria algo para separar el :
    //     query->file_tag = strdup(separado[1]);
    // }
    // if (cant_param > 2){
    //     if(query->identificador == TAG){
    //         char* file_tag_destino = strdup(separado[2]);
    //         query->param1 = file_tag_destino;
    //     }else{
    //         int* param1 = malloc(sizeof(int));
    //         *param1 = atoi(separado[2]);
    //         query->param1 = strdup(separado[2]);
    //     }
    // }
    // if (cant_param > 3){
    //     if(query->identificador == READ){
    //         int* tam = malloc(sizeof(int));
    //         *tam = atoi(separado[3]);
    //         query->param2 = tam;
    //     }else{
    //         query->param2 = strdup(separado[3]);
    //     }
    // }
    if(cant_param > 1){
        char** partes = string_split(separado[1], ":");
        query->file = strdup(partes[0]);
        query->tag = strdup(partes[1]);
        string_array_destroy;
    }
    if(cant_param > 2){
        query->param1 = strdup(separado[2]);
    }
    if(cant_param > 3){
        query->param2 = strdup(separado[3]);
    }
    
    string_array_destroy(separado);
    free(query_raw);
    return query;
}


void ejecutar_query(int query){
    switch(query){
        case -1:
            log_error(logger, "No se recibio query de master: Conexion cerrada");
            break;

        case CREATE_Q:

            break;
        
        case TRUNCATE_Q:

            break;
        
        case WRITE_Q:

            break;

        case READ_Q:

            break;

        case TAG_Q:

            break;
            
        case COMMIT_Q:

            break;

        case FLUSH_Q:

            break;

        case DELETE_Q:

            break;

        case END_Q:

            break;
        
        default:
            log_error(logger, "Error al recibir el query por parte de master");
            break;
    }
}