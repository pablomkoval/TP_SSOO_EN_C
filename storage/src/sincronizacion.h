#ifndef SINCRONIZACION_H
#define SINCRONIZACION_H

#include <utils/utils.h>

extern pthread_mutex_t mutex_bitmap;
extern pthread_mutex_t mutex_hash_index;

extern t_dictionary* mutex_por_metadata;
extern t_dictionary* mutex_por_bloque_fisico;

pthread_mutex_t* obtener_mutex_metadata(char* path);
pthread_mutex_t* obtener_mutex_bloque_fisico(char* path);

void inicializar_mutexes(void);

void lock_metadata(char* path);
void unlock_metadata(char* path);
void lock_bloque_fisico(char* path);
void unlock_bloque_fisico(char* path);


#endif