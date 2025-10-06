#include <query_interpreter.h>

void* ciclo_query_interpreter(){
    while(1){
        int opcode = recibir_opcode(socket_master);
        if(opcode != SOLICITUD_NUEVA_QUERY){
            log_error(logger, "blabla");
            return NULL;
        }
        recibir_query();
        //chequear interrupcion 
    }
}

void recibir_query(){
    int opcode = recibir_opcode(socket_master);

    switch(opcode){
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