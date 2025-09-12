#ifndef QUERY_CONFIG_H
#define QUERY_CONFIG_H

#include <utils/utils.h>


extern char* ip_master;
extern char* puerto_master;
extern char* log_level;


t_log* iniciar_logger(void);
t_config* iniciar_config(char* nombre_archivo);

#endif