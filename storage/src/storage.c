#include <storage.h>


t_log* logger;
t_config* config;


int main(int argc, char* argv[]) {
    
    config = iniciar_config();
    logger = iniciar_logger();
    void* mapeo = inicializar_bitmap();

    bitarray_set_bit(bitmap, 16);
    bitarray_set_bit(bitmap, 15);
    bitarray_set_bit(bitmap, 1);
    


    return 0;
}