#include <query_control.h>

t_log* logger;
t_config* config;


int socket_master;

int main(int argc, char* argv[]) {
    // if(argc < 4){
    //     printf("Faltaron argumentos para iniciar el query control");
    //     return EXIT_FAILURE;
    // }
    // char* archivo_config = argv[1];
    // char* archivo_query = argv[2];
    // int prioridad = atoi(argv[3]);

    char* archivo_config = "query.config";//hardcodeado para manu
    int prioridad = 0;//hardcodeado para manu

    config = iniciar_config(archivo_config);
    logger = iniciar_logger();
       
    socket_master = conectar_master();

    if (socket_master == -1){
        log_trace(logger, "Falló conexión con master");
        return EXIT_FAILURE;
    } else{
        t_paquete* paquete_query = empaquetar_query(archivo_query, prioridad);
        enviar_paquete(paquete_query, socket_master, logger);
        borrar_paquete(paquete_query);
        log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %d", archivo_query, prioridad);
        recibir_mensajes_de_master(socket_master);
    }
    return 0;
}
