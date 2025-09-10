#ifndef CONEXIONES_MASTER_H_
#define CONEXIONES_MASTER_H_
#include <utils/utils.h>
#include <master.h>

typedef struct 
{
    int socket;
    int id;
} t_argumentos_worker;

void* funcion_main_escucha (void* args);

#endif