#ifndef __CALLS__
#define __CALLS__

#include "system_calls.h"

/**
 * Constantes que identifican la validez de una operacion solicitada al sistema
 * */
#define OS_OK		0
#define OS_FAIL	1

/**
 * Tipos de dispositivos
 * */
#define CHARACTER_DEVICE	0
#define BLOCK_DEVICE			1

/**
 * Permisos validos en los dispotivos de caracteres
 * */
#define DEVICE_WRITABLE	1
#define DEVICE_READABLE	2

/**
 * Mensajes que pueden enviarse al servidor de archivos
 * */
#define DEVICE_INITIALITED		0
#define DEVICE_UNINITIALITED	1


/**
 * Mensajes que puede recibir un dispositivo de caracteres
 * */
#define READ_CHARACTERS		2
#define WRITE_CHARACTERS	3

/**
 * \brief Envia un mensaje de error
 */
void error(TID to, int code);

/**
 * \brief Informa a un hilo del exito de una peticion
 * \param to
 */
void success(TID to);

/**
 * \brief El hilo que realiza la llamada espera al menos la cantidad de
 * milisegundos indicada
 * \param milliseconds
 */
void sleep(unsigned milliseconds);


/**
 * \brief Escribe para un archivo un conjunto de caracteres
 * \param fd Descriptor del archivo
 * \param buff - buffer
 * \param length - longitud del buffer
 * \return 
 */
int write(int fd, const char* buff, unsigned length);

/**
 * \brief Lee una cantidad de bytes desde un archivo
 * \param fd Descriptor del archivo
 * \param buff Buffer donde se pondra lo leido
 * \param count	Cantidad de elmentos a leer
 * \return Cantidad de bytes leidos, negativo en caso de error
 */
int read(int fd, char* buff, unsigned count);


/**
 * \brief Registra al hilo que realiza la llamada como un dispositivo de caracteres
 * \param permissions Que puede hacer el dispositivo, DEVICE_WRITABLE | DEVICE_READABLE
 * \return OS_OK si todo fue bien, OS_FAIL en caso contrario
 */
int NewCharacterDeviceInSystem(unsigned permissions);


/**
 * \brief Envia un byte a un puerto I/O dado
 * \param port 
 * \param value
 */
void out_byte(int port, int value);

/**
 * \brief Envia un word (16 bits) a un puerto I/O dado
 * \param port
 * \param value
 */
void out_word(int port, int value);

/**
 * \brief Envia un dword (32 bits) a un puerto I/O dado
 * \param port
 * \param value
 */
void out_long(int port, int value);

/**
 * \brief Lee un byte de un puerto I/O dado
 * \param port
 * \return 
 */
int in_byte(int port);

/**
 * \brief Lee un word (16 bits) de un puerto I/O dado
 * \param port
 * \return 
 */
int in_word(int port);

/**
 * \brief Lee un dword (32 bits) de un puerto I/O dado
 * \param port
 * \return 
 */
int in_long(int port);

/**
 * \brief Supongo que no necesita presentacion
 */
void printf(const char* format,...);

#endif