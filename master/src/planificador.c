#include <planificador.h>

t_queue* cola_ready;
t_list* workers_libres;

void inicializar_planificador(){
    cola_ready = queue_create();
}

void enviar_qcb_a_worker(t_qcb* qcb, int socket_worker){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, SOLICITUD_NUEVA_QUERY);
    agregar_a_paquete(paquete, qcb->path, strlen(qcb->path) + 1);
    agregar_a_paquete(paquete, qcb->pc, strlen(qcb->pc) + 1);
    enviar_paquete(paquete, socket_worker, logger);
    borrar_paquete(paquete);
}

int obtener_worker_libre(){
    int* worker_id_ptr = list_remove(workers_libres, 0);
    int worker_id = *worker_id_ptr;
    return worker_id;
}

void* planificador(){
    while(1){
        //poner sus respectivos semaforos
        t_qcb* a_ejecutar ;
        if(strcmp(algoritmo_planificacion, "FIFO") == 1){
            a_ejecutar = queue_pop(cola_ready);
        }
        
        int socket_worker = obtener_worker_libre();
        enviar_qcb_a_worker(a_ejecutar, socket_worker);
    }
}
