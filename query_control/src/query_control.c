#include <query_control.h>

t_log* logger;
t_config* config;

int socket_master;

int main(int argc, char* argv[]) {
    saludar("query_control");

    config = iniciar_config();
    logger = iniciar_logger();

    socket_master = conectar_master();

    

    return 0;
}
