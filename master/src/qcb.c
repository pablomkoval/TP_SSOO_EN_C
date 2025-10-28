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

void cambiar_estado(t_qcb* qcb, int nuevo_estado) {
    qcb->estado = nuevo_estado;
    
    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){
        //actualizar el tiempo aging
    }
}