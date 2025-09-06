//seguro vaya en otro archivo, pero para ir haciendolo
#include <bitmap.h>

t_bitarray* bitmap;

void* inicializar_bitmap()
{
    //char* direccion_archivo = "algun_campo_config";
    char* direccion_archivo = "bitmap.bin";
    //int tamanio = tamanio fs / cant bloques;
    int tamanio = 32;

    FILE *archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    int fildes = fileno(archivo);
    ftruncate(fildes, tamanio) ;
    void *mapeo = mmap(0, tamanio, PROT_WRITE | PROT_READ, MAP_SHARED, fildes, 0);
    bitmap = bitarray_create_with_mode (mapeo, tamanio, LSB_FIRST);
    
    return mapeo;
}

