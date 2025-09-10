#include <estructura_storage.h>

t_bitarray* bitmap;

void crear_directorios_y_archivos()
{
    mkdir("files", 0777);
    mkdir("physical_blocks", 0777);
    void *mapeo = inicializar_bitmap();
    crear_archivo_hash_bloques();
}

void* inicializar_bitmap()
{
    char* direccion_archivo = string_from_format("%s%s", punto_montaje, "bitmap.bin");
    int tamanio = ( cant_blocks + 7 )  /  8;

    FILE *archivo;
    
    if(fresh_start)
    {
        archivo = fopen(direccion_archivo, "w+"); //lo crea, pero si existe lo sobreescribe
    }else
    {
        archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    }
    
    int fildes = fileno(archivo);
    ftruncate(fildes, tamanio) ;
    void *mapeo = mmap(0, tamanio, PROT_WRITE | PROT_READ, MAP_SHARED, fildes, 0);
    bitmap = bitarray_create_with_mode (mapeo, tamanio, LSB_FIRST);
    
    return mapeo;
}

void crear_archivo_hash_bloques()
{
    char* direccion_archivo = string_from_format("%s%s", punto_montaje, "blocks_hash_index.config");

    FILE *archivo;

    if(fresh_start)
    {
        archivo = fopen(direccion_archivo, "w+"); //lo crea, pero si existe lo sobreescribe
    }else
    {
        archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    }

}
