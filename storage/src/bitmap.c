#include <bitmap.h>

t_bitarray *bitmap;

void *inicializar_bitmap()
{

    char *direccion_archivo = concatenar_path(punto_montaje, "bitmap.bin");
    int tamanio = (cant_blocks + 7) / 8;

    FILE *archivo;

    if (fresh_start)
    {
        archivo = fopen(direccion_archivo, "w+"); // lo crea, pero si existe lo sobreescribe
    }
    else
    {
        archivo = fopen(direccion_archivo, "a+"); // lo crea, pero si existe no lo sobreescribe
    }

    int fildes = fileno(archivo);
    ftruncate(fildes, tamanio);
    void *mapeo = mmap(0, tamanio, PROT_WRITE | PROT_READ, MAP_SHARED, fildes, 0);
    bitmap = bitarray_create_with_mode(mapeo, tamanio, MSB_FIRST);

    log_trace(logger, "Se inicializó correctamente el BITMAP");

    free(direccion_archivo);

    return mapeo;
}

int buscar_bloque_libre()
{
    for (int i = 0; i < cant_blocks; i++)
    {
        if (!bitarray_test_bit(bitmap, i))
        { 
            log_error(logger, "lo esta");
            return i;
        }
    }
    log_info(logger, "No se encontraron bloques libres para asignar");
    return -3;
}

void asignar_bloque(int nro)
{
    bitarray_set_bit(bitmap, nro);
}