#include "funciones.h"

Sistema *info_sistema;

void *planificador_cola_bloqueados(){
    return NULL;
}

void *planificador(void*proce){
    Procesador *procesador = (Procesador*) proce;
    printf("hola soy %d\n", procesador->num_procesador);
    return NULL;
}

Proceso *leer_archivo_de_procesos(char *nombre_archivo){
    FILE* archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo");
        return NULL;
    }
    
    /*
        nose como seran los archivos, voy a suponer que los archivos son de la forma:
        _____________________________________
        |numero_de_procesos                 |
        |pid tiempo_llegada tiempo_servicio |
        |pid tiempo_llegada tiempo_servicio |
        |...                                |
        |...                                |
        |...                                |
        |pid tiempo_llegada tiempo_servicio |
        -------------------------------------

        ej:
        input.txt
        ________________
        |5             |
        |1 1000 100000 |
        |2 2000 200000 |  
        |3 3000 300000 |
        |4 4000 400000 |
        |5 5000 500000 |
        ----------------
    */

    int numero_de_procesos;
    fscanf(archivo, "%d", &numero_de_procesos);
    Proceso *cola_procesos = (Proceso*)malloc(sizeof(Proceso)*numero_de_procesos);

    int pid, tiempo_llegada, tiempo_servicio;
    int i = 0;
    while (fscanf(archivo, "%d %d %d", &pid, &tiempo_llegada, &tiempo_servicio) == 3) {
        cola_procesos[i].pid = pid;
        cola_procesos[i].tiempo_llegada = tiempo_llegada;
        cola_procesos[i].tiempo_servicio = tiempo_servicio;
        cola_procesos[i].tiempo_restante = tiempo_servicio;
        cola_procesos[i].esta_bloqueado = 0;
        i++;
    }

    for(int j = 0; j < numero_de_procesos; j++){
        printf("hola soy %d: %d, %d\n", cola_procesos[j].pid, cola_procesos[j].tiempo_llegada, cola_procesos[j].tiempo_servicio);
    }

    fclose(archivo);
    return cola_procesos;
}

void inicializacion_sistema(Proceso *cola_procesos){
    info_sistema = (Sistema*)malloc(sizeof(Sistema));
    info_sistema->cola_listos = cola_procesos;
    info_sistema->cola_bloqueados = NULL;
    pthread_mutex_init(&info_sistema->mutex_cola_listos, NULL);
    pthread_cond_init(&info_sistema->condicion_cola_listos, NULL);
    pthread_mutex_init(&info_sistema->mutex_cola_bloqueados, NULL);
}

void sistema(int procesadores, float quantum, float probabilidad){
    pthread_t tids[procesadores];
    for(int proc = 0; proc < procesadores; proc++){

        Procesador* procesador = (Procesador*)malloc(sizeof(Procesador));
        procesador->num_procesador = proc;
        procesador->procesos_ejecutados = 0;
        procesador->tiempo_bloqueado_promedio = 0;
        procesador->tiempo_espera_promedio = 0;
        procesador->utilizacion = 0;

        pthread_create(&tids[proc], NULL, planificador, (void*)procesador);
    }

    for(int proc = 0; proc < procesadores; proc++){
        pthread_join(tids[proc], NULL);
    }
    
    return;
}