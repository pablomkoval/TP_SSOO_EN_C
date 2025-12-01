#include <qcb.h>
#include <planificador.h>

pthread_mutex_t mutex_aging = PTHREAD_MUTEX_INITIALIZER;
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
    qcb->estado = READY;
    qcb->pc = 0;
    qcb->tiempo_aging_qcb = 0;
    qcb->chequeo_desalojo_pendiente = false;
    //qcb->semaforo_mutex = PTHREAD_MUTEX_INITIALIZER;

    //qcb->aging_activo = false;
    //qcb->tiempo_aging = NULL;

    log_trace(logger, "Se creo la qcb con qid %d", qcb->qid);
    return qcb;
}

void cambiar_estado(t_qcb* qcb, int nuevo_estado){
    
    int estado_actual = qcb->estado; //comentable si se saca el log_debug
    qcb->estado = nuevo_estado;
    
    log_debug(logger, "Se cambia estado de query %d de %d -> %d", qcb->qid, estado_actual, nuevo_estado);

    /* if(strcmp(algoritmo_planificacion, "PRIORIDADES") == 0){

        pthread_mutex_lock(&mutex_aging);

        if(nuevo_estado == READY && qcb->prioridad > 0){

            if(!qcb->aging_activo && tiempo_aging != 0){
                comenzar_aging_query(qcb);
            }
            pthread_mutex_unlock(&mutex_aging);
        }else if(qcb->aging_activo){
            pthread_t hilo = qcb->hilo_aging_id;
            qcb->aging_activo = false;
            pthread_mutex_unlock(&mutex_aging);

            pthread_cancel(hilo);
            pthread_join(hilo, NULL);
            qcb->aging_activo = false;
        }else{
            pthread_mutex_unlock(&mutex_aging);
        }
    } */
}

/* 
void comenzar_aging_query(t_qcb* qcb){
    //qcb->tiempo_aging = temporal_create();
    // pthread_mutex_lock(&mutex_aging);
    if (qcb->aging_activo) {
        // pthread_mutex_unlock(&mutex_aging);
        return;
    }
    // pthread_mutex_unlock(&mutex_aging);

    pthread_create(&(qcb->hilo_aging_id), NULL, hilo_aging_individual, (void*)qcb);
    pthread_detach(qcb->hilo_aging_id);

    // pthread_mutex_lock(&mutex_aging);
    qcb->aging_activo = true;
    // pthread_mutex_unlock(&mutex_aging);
    return;
} */