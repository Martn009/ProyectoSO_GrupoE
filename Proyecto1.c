#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// --- CONFIGURACIÓN DE SERVIDOR (ALTA CARGA) ---
#define NUM_CUENTAS 1000        // Tamaño de la Base de Datos
#define NUM_WORKERS 50          // 50 Hilos simultáneos
#define TRANSACCIONES_POR_WORKER 200 // Operaciones por hilo
#define RETARDO_IO 2000         // Latencia simulada (microsegundos)

// --- ESTRUCTURAS ---
typedef struct {
    int id;
    double saldo;
} Cuenta;

// --- RECURSOS COMPARTIDOS ---
Cuenta base_datos[NUM_CUENTAS]; 
sem_t sem_db_lock;              // Semáforo (Mutex)
long transacciones_exitosas = 0;
long transacciones_fallidas = 0;

// --- INICIALIZACIÓN ---
void inicializar_bd() {
    for (int i = 0; i < NUM_CUENTAS; i++) {
        base_datos[i].id = i;
        base_datos[i].saldo = 1000.0 + (rand() % 5000); 
    }
    printf("[INFO] Base de Datos montada. Registros: %d\n", NUM_CUENTAS);
}

// --- WORKER (Lógica de Transacción) ---
void *worker_db(void *arg) {
    // int id_worker = *(int *)arg; // No se usa en el log limpio, pero se mantiene la estructura
    
    for (int i = 0; i < TRANSACCIONES_POR_WORKER; i++) {
        
        // Simular latencia de red/disco
        usleep(rand() % RETARDO_IO);

        int tipo_ops = rand() % 3; // 0: Depósito, 1: Retiro, 2: Transferencia
        int cuenta_a = rand() % NUM_CUENTAS;
        int cuenta_b = rand() % NUM_CUENTAS; 
        double monto = 10 + (rand() % 500);

        // --- INICIO ZONA CRÍTICA ---
        sem_wait(&sem_db_lock);

        if (tipo_ops == 0) { 
            // Depósito
            base_datos[cuenta_a].saldo += monto;
            transacciones_exitosas++;
        } 
        else if (tipo_ops == 1) { 
            // Retiro (con validación)
            if (base_datos[cuenta_a].saldo >= monto) {
                base_datos[cuenta_a].saldo -= monto;
                transacciones_exitosas++;
            } else {
                transacciones_fallidas++;
            }
        } 
        else { 
            // Transferencia
            if (cuenta_a != cuenta_b && base_datos[cuenta_a].saldo >= monto) {
                base_datos[cuenta_a].saldo -= monto;
                base_datos[cuenta_b].saldo += monto;
                transacciones_exitosas++;
            } else {
                transacciones_fallidas++;
            }
        }
        
        // Log de progreso simplificado (cada 100 ops)
        long total = transacciones_exitosas + transacciones_fallidas;
        if (total % 100 == 0) {
             printf("[PROCESANDO] Total: %ld | OK: %ld | Error: %ld\r", total, transacciones_exitosas, transacciones_fallidas);
             fflush(stdout);
        }

        // --- FIN ZONA CRÍTICA ---
        sem_post(&sem_db_lock);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t workers[NUM_WORKERS];
    int ids[NUM_WORKERS];
    struct timespec start, end;

    srand(time(NULL));

    printf("--- SERVIDOR DE TRANSACCIONES BANCARIAS v3.0 ---\n");
    printf("Configuracion: %d Hilos | %d Cuentas\n", NUM_WORKERS, NUM_CUENTAS);
    
    inicializar_bd();
    sem_init(&sem_db_lock, 0, 1); // Inicializar Semáforo Binario

    printf("[INFO] Iniciando workers... Espere finalizacion.\n");
    
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Crear Hilos
    for (int i = 0; i < NUM_WORKERS; i++) {
        ids[i] = i;
        if (pthread_create(&workers[i], NULL, worker_db, &ids[i]) != 0) {
            perror("Error fatal creando hilo");
            exit(1);
        }
    }

    // Esperar Hilos (Join)
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Métricas
    double tiempo_total = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double tps = (transacciones_exitosas + transacciones_fallidas) / tiempo_total;

    printf("\n\n--- REPORTE DE RENDIMIENTO ---\n");
    printf("Estado:                FINALIZADO\n");
    printf("Tiempo Total:          %.4f segundos\n", tiempo_total);
    printf("Transacciones Totales: %ld\n", transacciones_exitosas + transacciones_fallidas);
    printf(" > Exitosas:           %ld\n", transacciones_exitosas);
    printf(" > Fallidas (Fondos):  %ld\n", transacciones_fallidas);
    printf("Rendimiento (TPS):     %.2f ops/seg\n", tps);
    printf("------------------------------\n");

    sem_destroy(&sem_db_lock);
    return 0;
}
