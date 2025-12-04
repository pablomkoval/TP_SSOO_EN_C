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

    log_trace(logger, "Se creo la qcb con qid %d", qcb->qid);
    return qcb;
}

void cambiar_estado(t_qcb* qcb, int nuevo_estado){
    
    //int estado_actual = qcb->estado; //comentable si se saca el log_debug
    qcb->estado = nuevo_estado;
    
    //log_debug(logger, "Se cambia estado de query %d de %d -> %d", qcb->qid, estado_actual, nuevo_estado);
}
