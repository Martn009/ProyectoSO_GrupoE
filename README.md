# Proyecto I Bimestre - Sistemas Operativos


# Requisitos y Compilacióm
## Requisitos del Sistema

Para compilar y ejecutar este programa, se requieren las siguientes herramientas:

- **Sistema operativo**: Linux o MacOS (Windows puede necesitar un entorno compatible con POSIX como Cygwin o WSL).
- **Compilador de C**: GCC (GNU Compiler Collection).
- **Librerías**: 
  - `pthread`: Para manejo de hilos.
  - `semaphore.h`: Para manejo de semáforos POSIX.

## Instalación

### 1. Instalar Dependencias

En sistemas basados en Debian (como Ubuntu), las dependencias necesarias se pueden instalar con el siguiente comando:

```bash
sudo apt-get update
sudo apt-get install build-essential
```


# Inicialización y Creación de Hilos en el Sistema Bancario de Alta Concurrencia

Aqui se explica cómo se inicializan los recursos del sistema y cómo se crean y sincronizan los hilos usando POSIX Threads (`pthread`) en el programa bancario. El objetivo es entender claramente el proceso de preparación, ejecución y finalización de los hilos que simulan cajeros operando sobre una base de datos compartida.

---

## Inicialización de Recursos

Antes de crear los hilos que realizarán las transacciones, el programa debe preparar varios elementos clave.

### **1. Inicialización de la Base de Datos**

La función `inicializar_bd()` construye un arreglo con 100 cuentas bancarias. A cada una se le asigna:

- Un número de identificación (`id`)
- Un saldo inicial de **1000 dólares**

```c
inicializar_bd();
```

Esto permite que todos los hilos trabajen sobre un estado inicial uniforme y coherente.

---
## **2. Inicialización del Semáforo**

```c
sem_init(&sem_transaccion, 0, 1);
```

Este semáforo POSIX (`sem_t`) se utiliza para proteger la **zona crítica**, es decir, las partes del código donde se modifica el saldo de las cuentas.  
Con un valor inicial de `1`, el semáforo actúa como un **candado de exclusión mutua (mutex)**:

- Solo un hilo puede entrar a la zona crítica a la vez.
- Se garantiza que las transacciones no se mezclen ni generen corrupción de datos.

---

## Creación de los Hilos (`pthread_create`)

Luego de la inicialización, el programa crea múltiples hilos, cada uno representando un cajero que realiza operaciones sobre las cuentas.

```c
for (int i = 0; i < NUM_HILOS; i++) {
    ids[i] = i;
    pthread_create(&hilos[i], NULL, cajero_thread, &ids[i]);
}
```

### **Proceso**

1. **Bucle `for`**
   - Se repite tantas veces como hilos se desean crear (`NUM_HILOS = 20`).
   - Cada vuelta crea un nuevo hilo.

2. **Asignación del ID del Hilo**
   ```c
   ids[i] = i;
   ```
   Cada hilo recibe un ID único para identificarlo en los registros.

3. **Uso del `pthread_create()`**
   ```c
   pthread_create(&hilos[i], NULL, cajero_thread, &ids[i]);
   ```
   Parámetros:
   - `&hilos[i]`: lugar donde se almacenará el identificador del hilo creado.
   - `NULL`: atributos por defecto.
   - `cajero_thread`: función que ejecutará el hilo.
   - `&ids[i]`: puntero al ID del hilo (su parámetro de entrada).

Cada hilo comienza a ejecutarse en paralelo, generando operaciones bancarias mientras compite por entrar a la zona crítica protegida por el semáforo.

---

## Finalización (`pthread_join`)

Una vez creados los hilos, el programa principal **no puede finalizar** hasta que todos los hilos hayan terminado su trabajo. Para ello se usa:

```c
for (int i = 0; i < NUM_HILOS; i++) {
    pthread_join(hilos[i], NULL);
}
```

### **Funcion del  `pthread_join`?**

- Bloquea la ejecución del hilo principal.
- Espera a que el hilo especificado termine.
- Garantiza que:
  - Ningún hilo quede ejecutándose después del cierre del programa.
  - Todas las transacciones se completen.
  - Los datos finales sean correctos.

### **Importancia del Orden**

El `join` se hace en un bucle secuencial porque:

- Cada llamada mantiene al programa principal detenido hasta que el hilo correspondiente finalice.
- El programa no pasa al reporte final hasta que **todos** los hilos hayan concluido.
---

