#include <estructura_storage.h>



t_config* hash;

int fd;

void crear_directorios_y_archivos()
{
    fd = open(punto_montaje, O_RDONLY | O_DIRECTORY);

    if(fresh_start)
    {
        formatear_volumen();
    }
    crear_directorio("files");
    
    crear_directorio("physical_blocks");
    crear_bloques_fisicos();
    crear_file("initial_file", "BASE");
    crear_bloque_logico("files/initial_file/BASE/logical_blocks", 0);

    void *mapeo = inicializar_bitmap();

    crear_archivo_hash_bloques();

    char* bloque_fisico = concatenar_path(punto_montaje,"physical_blocks/bloque0000.dat" );

    escribir_archivo(bloque_fisico, "hola mundo", 10);

    log_trace(logger, "se terminaron de crear todos los archivos y directorios necesarios");

  /// pruebas

    copiar_tag("initial_file", "BASE", "jose", "hola");

    free(bloque_fisico);
}



void crear_archivo_hash_bloques()
{
    char* config_block_hash = concatenar_path(punto_montaje,"blocks_hash_index.config");

    if(fresh_start)
    {
        FILE *archivo = fopen(config_block_hash, "w+"); //lo crea, pero si existe lo sobreescribe
        if(archivo) fclose(archivo);
    }else
    {
        FILE *archivo = fopen(config_block_hash, "a+"); //lo crea, pero si existe no lo sobreescribe
        if(archivo) fclose(archivo);
    }

    hash = config_create(config_block_hash);

    log_trace(logger, "Se inicializó correctamente el archvivo BLOCKS_HASH_INDEX");

    free(config_block_hash);

}

void formatear_volumen()
{
    char* dir_files  = concatenar_path(punto_montaje, "files");
    char* dir_blocks = concatenar_path(punto_montaje, "physical_blocks");

    char* borrar_files  = string_from_format("rm -rf '%s'", dir_files);
    char* borrar_blocks = string_from_format("rm -rf '%s'", dir_blocks);

    system(borrar_files);
    system(borrar_blocks);

    free(borrar_files);
    free(borrar_blocks);

    free(dir_files);
    free(dir_blocks);

    log_trace(logger, "Se formateó correctamente el volumen");
}

