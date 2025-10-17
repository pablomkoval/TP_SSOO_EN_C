#ifndef QCB_H_
#define QCB_H_

#include <master.h>

typedef struct{
    int socket;
    int qid;
    int pc;
    int prioridad;
    char* path;
    int id_worker_asociado;
    estados_query estado;
    t_temporal* tiempo_aging;
} t_qcb;

extern int qid_global;

t_qcb* crear_qcb (char* query_entrante, int socket);
void cambiar_estado(t_qcb* qcb, int nuevo_estado);

#endif