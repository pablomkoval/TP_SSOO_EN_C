#ifndef BITMAP_H
#define BITMAP_H

#include <utils/utils.h>
#include <storage.h>

extern t_bitarray* bitmap;

void* inicializar_bitmap();
void asignar_bloque(int nro);
int buscar_bloque_libre();

#endif