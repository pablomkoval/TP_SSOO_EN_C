#ifndef ESTRUCTURA_STORAGE_H
#define ESTRUCTURA_STORAGE_H

#include <storage.h>

#include <utils/utils.h>

void crear_directorios_y_archivos();

void* inicializar_bitmap();

void crear_archivo_hash_bloques();


extern t_bitarray* bitmap;

#endif