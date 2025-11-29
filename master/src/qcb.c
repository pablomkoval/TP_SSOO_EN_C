#include <qcb.h>
#include <planificador.h>

pthread_mutex_t mutex_qid_global;
int qid_global = 0;

t_qcb* crear_qcb(char* query_entrante, int prioridad_query, int socket){
    t_qcb* qcb = malloc(sizeof(t_qcb));
    qcb->socket = socket; 
    pthread_mutex_lock(&mutex_qid_global);
    qcb->qid = qid_global++;
    pthread_mutex_unlock(&mutex_qid_global);
    qcb->prioridad = prioridad_query;
    qcb->path = strdup(query_entrante);
    qcb->id_worker_asociado = -1;
    //qcb->estado = READY;
    qcb->pc = 0;
    //qcb->tiempo_aging = NULL;
    log_trace(logger, "Se creo la qcb con qid %d", qcb->qid);
    return qcb;
}

void cambiar_estado(t_qcb* qcb, int nuevo_estado){
    
    int estado_actual = qcb->estado;
    qcb->estado = nuevo_estado;
    
    log_debug(logger, "Se cambia estado de query %d de %d -> %d", qcb->qid, estado_actual, nuevo_estado);

    if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){
        if(nuevo_estado == READY && qcb->prioridad > 0){
            comenzar_aging_query(qcb);
        }else if(nuevo_estado != READY && estado_actual != EXEC){
            pthread_cancel(qcb->hilo_aging_id);
            pthread_join(qcb->hilo_aging_id, NULL);
        }
    }
}

void comenzar_aging_query(t_qcb* qcb){
    //qcb->tiempo_aging = temporal_create();
    pthread_create(&(qcb->hilo_aging_id), NULL, hilo_aging_individual, (void*)qcb);
    pthread_detach(qcb->hilo_aging_id);
}