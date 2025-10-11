#ifndef QUERY_INTERPRETER_H_
#define QUERY_INTERPRETER_H_

#include <utils/utils.h>
#include <worker.h>
#include <queries.h>
typedef enum {
    CREATE_Q,
    TRUNCATE_Q,
    WRITE_Q,
    READ_Q,
    TAG_Q,
    COMMIT_Q,
    FLUSH_Q,
    DELETE_Q,
    END_Q
} id_query_t;

typedef struct {
    id_query_t identificador;
    char* file_tag;
    char* param1;
    char* param2;
} query_t;


query_t* parsear_query(char* query_raw);
query_t* leer_query();
void ejecutar_query(query_t* query);
void* ciclo_query_interpreter();

#endif