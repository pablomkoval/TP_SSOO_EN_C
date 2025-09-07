//seguro vaya en otro archivo, pero para ir haciendolo
#include <bitmap.h>

t_bitarray* bitmap;

void* inicializar_bitmap()
{
    char* direccion_archivo = string_from_format("%s%s", punto_montaje, "bitmap.bin");;
    int tamanio = ( cant_blocks + 7 )  /  8;

    FILE *archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    int fildes = fileno(archivo);
    ftruncate(fildes, tamanio) ;
    void *mapeo = mmap(0, tamanio, PROT_WRITE | PROT_READ, MAP_SHARED, fildes, 0);
    bitmap = bitarray_create_with_mode (mapeo, tamanio, LSB_FIRST);
    
    return mapeo;
}

