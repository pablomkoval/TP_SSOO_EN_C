#include <storage.h>


t_log* logger;


int main(int argc, char* argv[]) {
    saludar("storage");

    logger = iniciar_logger();


    return 0;
}