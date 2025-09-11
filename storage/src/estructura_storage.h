#ifndef ESTRUCTURA_STORAGE_H
#define ESTRUCTURA_STORAGE_H

#include <storage.h>
#include <utils/utils.h>
#include <dirs.h>
#include <blocks.h>
#include <metadata.h>
#include <bitmap.h>

extern int fd;
extern t_config* metadata;
extern t_bitarray* bitmap;
extern t_config* hash;



void formatear_volumen();
void crear_directorios_y_archivos();

void crear_archivo_hash_bloques();





#endif