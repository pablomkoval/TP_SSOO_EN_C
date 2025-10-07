#ifndef METADATA_H
#define METADATA_H

#include <utils/utils.h>
#include <storage.h>

extern t_config* metadata;

void crear_metadata_config(char* archivo);
int cambiar_tamanio_metadata(char* path, char* nuevo_tamanio);
void cambiar_estado_metadata(char* path, char* nuevo_estado);
void agregar_bloque_metadata(char* path, int bloque);
char* estado_metadata(char* path);
int cant_bloques_logicos(char* path);
int commmit_file(char* path);


#endif