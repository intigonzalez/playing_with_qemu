#ifndef __KERNEL_TASKS__
#define __KERNEL_TASKS__

// informacion general de inicio
extern BootInfo _bootInfo;

// Las entradas inicialiales para los hilos, estan definidas en OsThread.s
void _HiloDelReloj();
void _at_thread();
void _fs_thread();
void _console_thread();
void _test_thread();

void ENTRADA();

// Crea un hilo para los discos duros y CD, definido en ata_device_task.c
void Init_AT_Control_Thread();
// Crea un hilo para el VGA y el display definido en console.c
void Init_Console_Thread();

/**
 * \brief Crea un hilo para controlar el sistema de archivos, definido en fs_task.c
 */
void Init_FS_Control_Thread();

/**
 * \brief Comenzamos el hilo de pruebas, definido en tests.c
 */
void Init_Test_Thread();

#endif