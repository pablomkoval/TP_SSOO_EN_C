#include <storage.h>


t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    saludar("storage");

    logger = iniciar_logger;
    config = iniciar_config;


    return 0;
}