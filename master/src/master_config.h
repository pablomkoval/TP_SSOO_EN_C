#ifndef MASTER_CONFIG_H
#define MASTER_CONFIG_H

#include <utils/utils.h>

extern char* puerto_escucha;
extern char* algoritmo_planificacion;
extern int tiempo_aging;
extern char* log_level;

t_log* iniciar_logger(void);
t_config* iniciar_config(char* nombre_config);

#endif