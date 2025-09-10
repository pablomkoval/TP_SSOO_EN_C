#include <estructura_storage.h>

t_bitarray* bitmap;

int fd;

void crear_directorios_y_archivos()
{
    fd = open(punto_montaje, O_RDONLY | O_DIRECTORY);
    crear_directorio("files");
    crear_directorio("files/initial_file");
    crear_tag("files/initial_file", "/BASE");
    crear_bloque_logico("raiz/files/initial_file/BASE/logical_blocks", "/0000.dat");

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
        FILE *archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe lo sobreescribe
        if(archivo) fclose(archivo);
    }else
    {
        FILE *archivo = fopen(direccion_archivo, "w+"); //lo crea, pero si existe no lo sobreescribe
        if(archivo) fclose(archivo);
    }

}

void crear_bloque_logico(char* path, char* numero)
{

    char* path_bloque = concatenar_path(path, numero);

    FILE *archivo = fopen(path_bloque, "a+");
    if(archivo) fclose(archivo);
}

void crear_directorio(const char* path) {
    mkdirat(fd, path , 0777);
}

void crear_tag(char* file_path, char* tag)
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

void asociar_hash_block()
{
    
}