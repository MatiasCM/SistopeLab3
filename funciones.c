#include "funciones.h"

Sistema *info_sistema;

Proceso *desencolar(Proceso *cola, int num_procesos){
    Proceso *cola_modificada = (Proceso*)malloc(sizeof(Proceso)*(num_procesos-1));
    for(int proc = 0; proc < (num_procesos-1); proc++){
        cola_modificada[proc] = cola[proc];
    }
    return cola_modificada;
}

Proceso *encolar(Proceso *cola, int num_procesos, Proceso proceso_entrante){
    Proceso *cola_modificada = (Proceso*)malloc(sizeof(Proceso)*(num_procesos+1));
    cola_modificada[0] = proceso_entrante;

    for(int proc = 1; proc < (num_procesos+1); proc++){
        cola_modificada[proc] = cola[proc-1];
    }
    return cola_modificada;
}

void *planificador_cola_bloqueados(){

    srand(time(NULL));

    while(info_sistema->procesos_en_cola_listos > 0 || info_sistema->procesos_en_cola_bloqueados > 0){
        pthread_mutex_lock(&info_sistema->mutex_cola_bloqueados);


        int *duraciones_RIO_x_proceso = (int*)malloc(sizeof(int)*info_sistema->procesos_en_cola_bloqueados);
        for(int proc = 0; proc < info_sistema->procesos_en_cola_bloqueados; proc++){
            duraciones_RIO_x_proceso[proc] = rand() % 401 + 100;
        }
        

        while(info_sistema->procesos_en_cola_bloqueados > 0){
            usleep(10 * 1000);

            int procesos_cola_bloqueados = info_sistema->procesos_en_cola_bloqueados;
            for(int proc = (procesos_cola_bloqueados-1); proc >= 0; proc++){
                usleep(10 * 1000);
                duraciones_RIO_x_proceso[proc] -= 10;
                if(duraciones_RIO_x_proceso[proc] <= 0){
                    pthread_mutex_lock(&info_sistema->mutex_cola_listos);
                    
                    Proceso proceso_frente = info_sistema->cola_bloqueados[procesos_cola_bloqueados-1];
                    info_sistema->cola_bloqueados = desencolar(info_sistema->cola_bloqueados, procesos_cola_bloqueados);
                    info_sistema->cola_listos = encolar(info_sistema->cola_listos, info_sistema->procesos_en_cola_listos, proceso_frente);

                    info_sistema->procesos_en_cola_bloqueados -= 1;
                    info_sistema->procesos_en_cola_listos += 1;
                    
                    pthread_mutex_unlock(&info_sistema->mutex_cola_listos);
                }
                else{
                    Proceso proceso_frente = info_sistema->cola_bloqueados[procesos_cola_bloqueados-1];
                    info_sistema->cola_bloqueados = desencolar(info_sistema->cola_bloqueados, procesos_cola_bloqueados);
                    info_sistema->cola_bloqueados = encolar(info_sistema->cola_bloqueados, procesos_cola_bloqueados, proceso_frente);
                }
            }
        }
        free(duraciones_RIO_x_proceso);

        pthread_mutex_unlock(&info_sistema->mutex_cola_bloqueados);
    }

    return NULL;
}

void *planificador(void*proce){

    srand(time(NULL));

    Procesador *procesador = (Procesador*) proce;

    while(info_sistema->procesos_en_cola_listos > 0 || info_sistema->procesos_en_cola_bloqueados > 0){

        // seccion critica
        //
        pthread_mutex_lock(&info_sistema->mutex_cola_listos);

        int num_procesos_cola_listos = info_sistema->procesos_en_cola_listos;

        Proceso proceso;
        
        if(num_procesos_cola_listos == 0 && info_sistema->procesos_en_cola_bloqueados > 0){
            // se debe quedar esperando a que agreguen procesos a la cola de listos
            pthread_cond_wait(&info_sistema->condicion_cola_listos, &info_sistema->mutex_cola_listos);
            num_procesos_cola_listos = info_sistema->procesos_en_cola_listos;
        }

        if(num_procesos_cola_listos > 0){
            proceso = info_sistema->cola_listos[num_procesos_cola_listos-1];

            info_sistema->cola_listos = desencolar(info_sistema->cola_listos, num_procesos_cola_listos);
            info_sistema->procesos_en_cola_listos -= 1;
        }
        
        pthread_mutex_unlock(&info_sistema->mutex_cola_listos);
        //
        // seccion critica
        
        if(num_procesos_cola_listos > 0){
            // genera un numero entre 1 y 100
            int hace_IO = rand() % 100 + 1;

            // si el numero creado es menor o igual que la probabilidad por 100 entonces hace IO
            // ej: si la probabilidad_RIO es 0.31 entonces desde 1 a 31 se hace RIO y desde 32 hasta 100 no
            //     eso daria una probabilidad de 31% 
            if(hace_IO <= procesador->probabilidad_RIO*100){
                int duracion_RP = rand() % ((int)procesador->quantum);
                usleep(duracion_RP *1000);
                proceso.tiempo_restante -= duracion_RP;

                // seccion critica
                //
                pthread_mutex_lock(&info_sistema->mutex_cola_bloqueados);

                int num_procesos_cola_bloqueados = info_sistema->procesos_en_cola_bloqueados;

                info_sistema->cola_bloqueados = encolar(info_sistema->cola_bloqueados, num_procesos_cola_bloqueados, proceso);
                info_sistema->procesos_en_cola_bloqueados += 1;

                pthread_mutex_unlock(&info_sistema->mutex_cola_bloqueados);
                //
                // seccion critica
            }
            else{
                if(proceso.tiempo_restante <= procesador->quantum){
                    usleep(proceso.tiempo_restante *1000);
                    proceso.tiempo_restante = 0;
                    printf("proceso con pid %d termino\n", proceso.pid);
                }
                else{
                    usleep(procesador->quantum *1000);
                    proceso.tiempo_restante -= procesador->quantum;

                    pthread_mutex_lock(&info_sistema->mutex_cola_listos);

                    info_sistema->cola_listos = encolar(info_sistema->cola_listos, info_sistema->procesos_en_cola_listos, proceso);
                    info_sistema->procesos_en_cola_listos += 1;
                    pthread_cond_signal(&info_sistema->condicion_cola_listos);

                    pthread_mutex_unlock(&info_sistema->mutex_cola_listos);

                }
            }
            printf("quedan %d\n", info_sistema->procesos_en_cola_listos);
        }

    }
    printf("termine mi ejecucion %d\n", procesador->num_procesador);
    return NULL;
}

Proceso *leer_archivo_de_procesos(char *nombre_archivo, int*numero_de_procesos){
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

    fscanf(archivo, "%d", numero_de_procesos);

    Proceso *cola_procesos = (Proceso*)malloc(sizeof(Proceso)*(*numero_de_procesos));

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
    
    fclose(archivo);
    return cola_procesos;
}

void inicializacion_sistema(Proceso *cola_procesos, int numero_procesos){
    info_sistema = (Sistema*)malloc(sizeof(Sistema));
    info_sistema->cola_listos = cola_procesos;
    info_sistema->cola_bloqueados = NULL;
    info_sistema->procesos_en_cola_listos = numero_procesos;
    info_sistema->procesos_en_cola_bloqueados = 0;
    pthread_mutex_init(&info_sistema->mutex_cola_listos, NULL);
    pthread_cond_init(&info_sistema->condicion_cola_listos, NULL);
    pthread_mutex_init(&info_sistema->mutex_cola_bloqueados, NULL);
}

void sistema(int procesadores, float quantum, float probabilidad){
    pthread_t tids[procesadores+1];
    for(int proc = 0; proc < procesadores; proc++){

        Procesador* procesador = (Procesador*)malloc(sizeof(Procesador));
        procesador->num_procesador = proc;
        procesador->procesos_ejecutados = 0;
        procesador->tiempo_bloqueado_promedio = 0;
        procesador->tiempo_espera_promedio = 0;
        procesador->utilizacion = 0;
        procesador->quantum = quantum;
        procesador->probabilidad_RIO = probabilidad;

        pthread_create(&tids[proc], NULL, planificador, (void*)procesador);
    }

    // la ultima hebra que se crea es la planificadora de la cola de bloqueados
    pthread_create(&tids[procesadores], NULL, planificador_cola_bloqueados, NULL);


    for(int proc = 0; proc < (procesadores+1); proc++){
        pthread_join(tids[proc], NULL);
    }
    
    return;
}