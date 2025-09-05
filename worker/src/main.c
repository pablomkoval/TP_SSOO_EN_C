#include <utils/hello.h>
#include <utils/utils.h>

int main(int argc, char* argv[]) {
    saludar("worker");
    t_config* config = config_create("worker.config");
    return 0;
}
