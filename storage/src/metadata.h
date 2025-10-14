#ifndef METADATA_H
#define METADATA_H

#include <utils/utils.h>
#include <storage.h>
#include <sincronizacion.h>

extern t_config* metadata;

void crear_metadata_config(char* archivo);
int cambiar_tamanio_metadata(char* path, char* nuevo_tamanio);
void cambiar_estado_metadata(char* path, char* nuevo_estado);
char* estado_metadata(char* path);
int cant_bloques_logicos(char* path);
int commmit_file(int query_id, char* file, char* tag);
bool escritura_no_permitida(char* file_tag);
void cambiar_bloque_metadata(char* path, int bloque, int pos);
char* path_config_meta(char* file_tag);
void agregar_bloque_metadata(char* path, int bloque, int pos);
void quitar_ultimo_bloque_metadata(char *path);

#endif