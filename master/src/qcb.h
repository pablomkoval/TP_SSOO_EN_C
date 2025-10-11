#include <master.h>

typedef struct{
    int socket;
    int id;
    int prioridad;
    char* path;
    int id_worker_asociado;
} t_qcb;

t_qcb* crear_qcb (int qid);