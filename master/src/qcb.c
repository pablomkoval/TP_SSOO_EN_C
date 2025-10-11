#include <qcb.h>

static int qid_global = 0;

t_qcb* crear_qcb(char* query_entrante){
    t_qcb* qcb = malloc(sizeof(t_qcb));
    // qcb->socket = ; esto q poronga es
    qcb->qid = qid_global++;
    qcb->prioridad = 0 //despues con el de prioridades vamos viendo
    qcb->path = strdup(query_entrante);
    qcb->id_worker_asociado = -1;
    qcb->estado = READY;
}

//void cambiar_estado(query,nuevo estado) {
   // pcb->estado_actual = nuevo_estado;
   // pcb->metricas_estado[nuevo_estado]++;