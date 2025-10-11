#include <master.h>

typedef struct{
    int socket;
    int qid;
    int prioridad;
    char* path;
    int id_worker_asociado;
    estados_query estado;
} t_qcb;

t_qcb* crear_qcb (char* query_entrante);