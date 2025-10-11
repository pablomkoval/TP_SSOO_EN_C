#include <planificador.h>

void inicializar_fifo(t_queue* cola_ready){
    cola_ready = queue_create();
}

void agregar_query_fifo(t_queue* cola_ready, t_query* query){
    queue_push(cola_ready, query);
}

void* obtener_query_siguiente_fifo(t_queue* cola_ready){
    if (queue_is_empty(cola_ready))
        return NULL;
    return queue_pop(cola_ready);
}

void destruir_fifo(t_queue* cola_ready){
    queue_destroy(cola_ready);
}