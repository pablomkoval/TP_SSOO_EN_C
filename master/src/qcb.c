#include <qcb.h>

int qid_global = 0;

t_qcb* crear_qcb(char* query_entrante, int prioridad_query, int socket){
    t_qcb* qcb = malloc(sizeof(t_qcb));
    qcb->socket = socket; 
    qcb->qid = qid_global++;
    qcb->prioridad = prioridad_query;
    qcb->path = strdup(query_entrante);
    qcb->id_worker_asociado = -1;
    qcb->estado = READY;
    qcb->pc = 0;
    
    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){
        qcb->tiempo_aging = temporal_create();
    } else{
        qcb->tiempo_aging = NULL;
    }
    return qcb;
}

void cambiar_estado(t_qcb* qcb, int nuevo_estado){
    int estado_anterior = qcb->estado;
    qcb->estado = nuevo_estado;
    
    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0 && estado_anterior == READY){

        if(nuevo_estado == EXEC || nuevo_estado == EXIT){
            //log_info(logger, "qid: %d. Termino hilo de aging. Estado nuevo de qcb: %s", qcb->qid, qcb->estado);
            
            //terminar el hilo
            //pthread_cancel() poner ese??? o otro que haga terminar el hilo de una
        }
    }
}