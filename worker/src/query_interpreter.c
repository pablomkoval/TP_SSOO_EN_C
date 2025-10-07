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

int leer_query(char* nombre_archivo, int pc){
    
    //fopen(path_queries+nombre_archivo)
    char* archivo = string_from_format("%s%s", path_queries, nombre_archivo);
    FILE* archivo = fopen( archivo, "r");
    //t_list* lista_queries = list_create();

    char linea[256];
    fgets(pc, sizeof(linea), archivo);
    linea[strcspn(linea, "\n")] = 0; // eliminar \n
    //list_add(lista_instrucciones, strdup(linea)); // guardar copia
    
    fclose(archivo);


    //leer del archivo y guardar en query
    int query;
    //query = parsear_query(query_str)

    return query;
}



void ejecutar_query(int query){
    switch(query){
        case -1:
            log_error(logger, "No se recibio query de master: Conexion cerrada");
            break;

        case CREATE:

            break;
        
        case TRUNCATE:

            break;
        
        case WRITE:

            break;

        case READ:

            break;

        case TAG:

            break;
            
        case COMMIT:

            break;

        case FLUSH:

            break;

        case DELETE:

            break;

        case END:

            break;
        
        default:
            log_error(logger, "Error al recibir el query por parte de master");
            break;
    }
}