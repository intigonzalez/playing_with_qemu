#ifndef __SYSTEM_CALLS__
#define __SYSTEM_CALLS__

/**
 * Tipo que representa un identificador de hilo
 * */
typedef unsigned TID;

/**
 * Llamadas al microkernel
 * */
#define THREAD_CONTROL		1
#define INTERRUPT_CONTROL	2
#define IPC								3

/**
 * Tipos de ipc
 * */
#define THREAD_SEND				1
#define THREAD_RECEIVE		2
#define	THREAD_SND_RCV		3

/**
 * Fuentes de mensajes, de donde puede venir un mensaje
 * */
#define HARDWARE_SOURCE			1
#define ANY_SOURCE					2

/**
 * Tamaño del area UTCB
 * */
#define UTCB_AREA_SIZE 64*4


/*
 * Esta estructura contiene informacion util para el hilo raiz del sistema
 * EL hilo raiz la recibe en su pila al inicio y es responsable de analizarla
 * */
typedef struct 
{
	void * videoMemory; // indica el lugar donde esta la memoria de video
	void * hardwareMemory; // indica donde estan mapeado los primeros 4mb de memoria fisica
	unsigned memory; // indica la cantidad de memoria en paginas de 4kb
	
	void* ramdisk1;
	void* ramdisk2; 
} BootInfo,*pBootInfo;



int do_call(unsigned type);

typedef void *TIPO();
/**
 * \brief Esta llamada crea un hilo nuevo en el mismo espacio de direcciones
 * del hilo que realiza la llamada, el nuevo hilo comienza la ejecucion
 * en la posicion dada por eip y con pila esp.
 * El hilo que realiza la llamada es responsable de proveer direcciones
 * correctas reservando espacio para ellas
 * \param eip Es la direccion donde se comienza a ejecutar el hilo
 * \param esp Es la direccion donde comienza el apuntador de la pila
 * \return El id del hilo creado si no existio ningun problema, 0 o negativo
 * en caso contrario
 * */
int ThreadControl(TIPO eip, void* esp, void* utcb);

/**
 * \brief Con esta llamada un hilo informa que desea manejar una interrupcion
 * El hilo debera esperar por mensajes
 * \param interrupt Es el numero de la interrupcion que deseamos manejar
 * \return Un codigo de error
 */
int InterruptControl(unsigned interrupt);

/**
 * \brief Realiza una ipc (interprocess comunication) de forma sincrona
 * \param ACTION - Send, Receive o Send and Receive
 * \param id - a quien le envio, de quien recibo
 * \return Un codigo de error, 0 en caso contrario
 */
int ipc(unsigned ACTION, TID id);


/**
 * \brief Establece el valor de un registro
 * \param index - Indice del registro (0 - 63)
 * \param value
 */
void LoadRegistre(unsigned index, int value);

/**
 * \brief Establece el valor de un registro de 8 bits
 * \param index Existen 256 registros de 8 bits, de 0 - 255
 * \param value Valor a poner en el registro, solo se usan los 8 bits menos significativos
 */
void LoadByteRegistre(unsigned index, unsigned value);

/**
 * \brief Devuelve el valor de un registro
 * \param index - Indice del registro (0 - 63)
 * \return 
 */
int StoreRegistre(unsigned index);

/**
 * \brief Devuelve el valor de un registro de 8 bits
 * \param index Existen 256 registros de 8 bits, de 0 - 255
 * \return  El valor de ese registro
 */
int StoreByteRegistre(unsigned index);

/**
 * @brief Entra a modo kernel un ratico, es de prueba pero sera real muy pronto
 */
void PassToKernelMode(unsigned addrs);

/**
 * Es la direccion de retorno
 */
void EtiquetaDeRetorno();

#endif
