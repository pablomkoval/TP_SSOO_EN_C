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
    int tiempo_aging_qcb;
    bool chequeo_desalojo_pendiente;
    //pthread_mutex_t semaforo_mutex;
    //t_temporal* tiempo_aging;
    //pthread_t hilo_aging_id;
    //bool aging_activo;
} t_qcb;

extern int qid_global;
extern pthread_mutex_t mutex_qid_global;
extern pthread_mutex_t mutex_aging;

t_qcb* crear_qcb (char* query_entrante, int prioridad_query, int socket);
void cambiar_estado(t_qcb* qcb, int nuevo_estado);
//void comenzar_aging_query(t_qcb* qcb);

#endif