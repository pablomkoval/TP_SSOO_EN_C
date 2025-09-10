#ifndef ESTRUCTURA_STORAGE_H
#define ESTRUCTURA_STORAGE_H

#include <storage.h>

#include <utils/utils.h>

extern int fd;

void crear_directorios_y_archivos();

void* inicializar_bitmap();

void crear_archivo_hash_bloques();

void crear_tag(char* file_path, char* tag);

char* concatenar_path(char* path, char* suma);

void crear_directorio(const char* path);

void crear_bloque_logico(char* path, char* numero);




extern t_bitarray* bitmap;

#endif