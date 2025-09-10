#include<conexiones_master.h>

void* hilo_main_escucha (void* args){
    pthread_t hilo_cliente;

    while(1){
        int socket_cliente = esperar_cliente(socket_main_escucha);
        
    }
}

void* manejar_servidor_worker(void* arg){
    int socket_cliente = *(int*)arg;
    free(arg);

    while(1){
        int op_code = recibir_opcode(socket_cliente);

        if(op_code == -1){
            log_info(logger, "Se cerro la conexiopn de un worker");
            break;
        }

        switch(op_code){
            case HANDSHAKE:
                int worker_id;
                log_trace(logger, "Recibi handshake de un worker");
                recibir_handshake(socket_cliente);

                recv(socket_cliente, &worker_id, sizeof(int), MSG_WAITALL);
                log_trace(logger, "Conexion de Worker ID: %d", worker_id);
                break;
            default:
                log_debug(logger, "Error al recibir opcode, %d", op_code);
                break;
        }
    }

}