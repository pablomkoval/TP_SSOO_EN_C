#ifndef QCB_H_
#define QCB_H_

#include<utils/utils.h>
#include<master_config.h>

typedef enum{
    READY,
    EXEC,
    EXIT
} estados_query;

typedef struct{
    int socket;
    int qid;
    int pc;
    int prioridad;
    char* path;
    int id_worker_asociado;
    estados_query estado;
    t_temporal* tiempo_aging;
    pthread_t hilo_aging_id;
} t_qcb;

extern int qid_global;

t_qcb* crear_qcb (char* query_entrante, int prioridad_query, int socket);
void cambiar_estado(t_qcb* qcb, int nuevo_estado);
void comenzar_aging_query(t_qcb* qcb);

#endif