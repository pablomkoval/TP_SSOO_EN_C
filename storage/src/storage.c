#include <storage.h>

t_log *logger;
t_config *config;
t_config *config_superblock;

int main(int argc, char *argv[])
{

    config = iniciar_config();
    logger = iniciar_logger();
    config_superblock = iniciar_config_superblock();

    // despues se irá a una funcion general
    void *mapeo = inicializar_bitmap();
    //

    // pruebas
    bitarray_set_bit(bitmap, 16);
    bitarray_set_bit(bitmap, 15);
    bitarray_set_bit(bitmap, 1);
    bitarray_set_bit(bitmap, 31);
    //

    return 0;
}