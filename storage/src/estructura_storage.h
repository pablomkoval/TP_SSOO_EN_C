#ifndef ESTRUCTURA_STORAGE_H
#define ESTRUCTURA_STORAGE_H

#include <storage.h>

#include <utils/utils.h>

void crear_directorios_y_archivos();

void* inicializar_bitmap();

void crear_archivo_hash_bloques();

void crear_tag();

char* concatenar_path(char* path, char* suma);

void crear_directorio(const char* path);


extern t_bitarray* bitmap;

#endif