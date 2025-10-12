#include <qcb.h>

int qid_global = 0;

t_qcb* crear_qcb(char* query_entrante, int socket){
    t_qcb* qcb = malloc(sizeof(t_qcb));
    qcb->socket = socket; 
    qcb->qid = qid_global++;
    qcb->prioridad = 0;
    qcb->path = strdup(query_entrante);
    qcb->id_worker_asociado = -1;
    qcb->estado = READY;
}

//void cambiar_estado(query,nuevo estado) {
   // pcb->estado_actual = nuevo_estado;
   // pcb->metricas_estado[nuevo_estado]++;