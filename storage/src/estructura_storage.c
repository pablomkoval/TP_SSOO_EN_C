#include <estructura_storage.h>

t_bitarray* bitmap;

void crear_directorios_y_archivos()
{
    crear_directorio("files");
    crear_directorio("files/initial_file");
    crear_tag("files/initial_file", "/BASE");

    crear_directorio("physical_blocks");

    void *mapeo = inicializar_bitmap();

    crear_archivo_hash_bloques();
}

void* inicializar_bitmap()
{
    
    char* direccion_archivo = concatenar_path(punto_montaje,"bitmap.bin");
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

    if(fresh_start)
    {
        
    }else
    {
        fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    }

}

void crear_directorio(const char* path) {
    mkdir(path, 0777);
}

void crear_tag(const char* file_path, const char* tag)
{
    char* path = concatenar_path(file_path, tag);
    crear_directorio(path);

    char* path_blocks = concatenar_path(path, "/logical_blocks");
    crear_directorio(path_blocks);

    char* path_metadata = concatenar_path(path, "/metadata.config");
    fopen(path_metadata, "w+");

}

char* concatenar_path(char* path, char* suma)
{
    char* direccion_archivo = string_from_format("%s%s", path, suma);
    return direccion_archivo;
}