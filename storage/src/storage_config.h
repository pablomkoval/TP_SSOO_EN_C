#ifndef STORAGE_CONFIG_H_
#define STORAGE_CONFIG_H_
#include <utils/utils.h>

t_config* iniciar_config();
t_log* iniciar_logger(void);
t_config* iniciar_config_superblock();

extern char* puerto_escucha;
extern bool fresh_start;
extern char* punto_montaje;
extern int retardo_operacion;
extern int retard_acceso_bloque;
extern char* log_level;

extern int fs_size;
extern int block_size;
extern int cant_blocks;



#endif