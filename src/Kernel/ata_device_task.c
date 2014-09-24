#include <system_calls.h>
#include <calls.h>

#include "kerneltasks.h"

// Pila del este hilo
static char pila[1024];
// Area utcb de este hilo
static char utcb[UTCB_AREA_SIZE];

// Este es el buffer de lectura para el modo PIO
static unsigned char buffer[1024];



// offset de los registros
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D
// dispositivo ATA o ATAPI
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
// master o esclavo seleccionado
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01
// mask para los registros de estado
#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01
// mask para el registro de error
#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01
// algunos comandos
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC


typedef struct {
	unsigned base; // direccion base del archivo de tareas
	unsigned controlPort; // puerto control/AlternateStatus
	unsigned existe; // indica si realmente existe
	unsigned slave; // es el master o el esclavo
	
	unsigned size; // tamaño del disco en bloques (por ahora de 512 bytes) o e creo eso
	char LBA48; // 1 si el disco tiene soporte LBA 48, 0 en caso contrario
	
	char model[41];	// indica vendedor o fabricante
	char serial[21]; // indica un numero de serie de ese vendedor
} AT_DEVICE;

#define NDEVICES 4

static AT_DEVICE devices[NDEVICES];


unsigned ide_read(int deviceIndex, unsigned port)
{
	
	if (port == ATA_REG_ALTSTATUS)
		return in_byte(devices[deviceIndex].controlPort);
	else 
		return in_byte(devices[deviceIndex].base + port);
}

void ide_write(int deviceIndex, unsigned port, unsigned char data)
{
	if (port == ATA_REG_CONTROL)
		out_byte(devices[deviceIndex].controlPort,data);
	else
	{
		out_byte(devices[deviceIndex].base + port ,data);
	}
}

void ide_read_data(int deviceIndex, unsigned char* buffer)
{
	int index = 0;
	unsigned short *b = (unsigned short *)buffer;
	unsigned short d;
	while (index < 256)
	{
		d = in_word(devices[deviceIndex].base + ATA_REG_DATA);
		b[index] = d;
		index++;
	}
} 

/**
 * \brief Inicializa los dispositovs
 */
void InitDevices()
{
	unsigned status,err;
 
	// 1- Detect I/O Ports which interface IDE Controller:
	devices[0].base = 0x1F0;
	devices[0].controlPort = 0x3F6;
	devices[0].slave = 0;
	devices[0].existe = 1; // asumo que existe

	devices[1].base = 0x1F0;
	devices[1].controlPort = 0x3F6;
	devices[1].slave = 1;
	devices[1].existe = 1; // asumo que existe

	devices[2].base = 0x170;
	devices[2].controlPort = 0x376;
	devices[2].slave = 0;
	devices[2].existe = 1; // asumo que existe

	devices[3].base = 0x170;
	devices[3].controlPort = 0x376;
	devices[3].slave = 1;
	devices[3].existe = 1; // asumo que existe 

	// 2- Disable IRQs: Vamos a usar PIO Mode por ahora, hasta entender Ultra DMA
	out_byte(0x3f6, 0x80 | 0x02);
	out_byte(0x3f6, 0x02);
	out_byte(0x376, 0x80 | 0x02);
	out_byte(0x376, 0x02);
	printf("ATA Interrupts Disabled Done\n");
	// configuramos cada dispositivo
	for (int i = 0 ; i < 4 ; i++)
	{
		err = 0;
	 // seleccionamos el device adecuado
	 ide_write(i,ATA_REG_HDDEVSEL,0xA0 | (devices[i].slave << 4));
	 sleep(100);
	 
	 // ahora mandamos el comando IDENTIFY
	 ide_write(i,ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	 sleep(100);
	 
	 status = ide_read(i,ATA_REG_STATUS);
	 if (status == 0)
	 {
		 // no existe el device, lo marco asi y voy al siguiente
		 devices[i].existe = 0;
		 continue;
	 }
	
	 while(1) {
            status = ide_read(i, ATA_REG_STATUS);
            if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY)) break; // Everything is right.
	 }

	 if (err)
	 {
		 devices[i].existe = 0;
		 continue;
	 }
	 
	 // leo los datos devueltos por el comando de identificacion
	 ide_read_data(i,&buffer[0]);
	 
	 unsigned short *b = (unsigned short *)buffer;
	 
	 /*
	 if (b[2] == 0x738c || b[2] == 0xc837)
	 {
		 print ("ESTE TIPO EXISTE\n\n");
	 }
	 else
	 {
		 devices[i].existe = 0;
		 //continue;
	 }
	 */
	 
	 // tomo el SIZE
	 if (b[83] & 0x400)
	 {
		 // compruebo soporte LBA 48
		 devices[i].LBA48 = 1;
		 devices[i].size = b[100] + (b[101] << 16);
	 }
	 else
	 {
		 devices[i].LBA48 = 0;
		 devices[i].size = b[61];
		 devices[i].size = devices[i].size << 16;
		 devices[i].size += b[60];
	 }
	 
	 // copio el modelo
	 for (int j = 0 ; j < 40 ; j++)
		 devices[i].model[j] = buffer[27*2 + j];
	 devices[i].model[40] = 0;	
	 
	 // copio el numero de serie
	 for (int j = 0 ; j < 20 ; j++)
		 devices[i].serial[j] = buffer[10*2 + j];
	 devices[i].serial[20] = 0;
	 
	 printf("Modelo : %s\n",devices[i].model);
	 printf("Serie  : %s\n",devices[i].serial);
	 printf("El size del disco es : %d\n",devices[i].size);
	 
	 //printNumber(i,10);
	}
}


/**
 * \brief Lee 512 bytes de un dispositivo
 * \param indexDevice
 * \param block
 * \param buffer
 * \return 1, si todo Ok, 0 en caso contrario
 */
int ReadBlock(int indexDevice, unsigned block, unsigned char* buffer)
{
	unsigned status,err;
	if (!devices[indexDevice].existe) return 0;
	
	// voy a leer con lba28
	unsigned lba0 = block & 0xFF;
	unsigned lba1 = (block & 0xFF00) >> 8;
	unsigned lba2 = (block & 0xFF0000) >> 16;
	unsigned devSelected = (block & 0xFF000000) >> 24;
	devSelected = (devices[indexDevice].slave << 4) | devSelected | 0x40;
	unsigned sectorCount = 1;
	
	// espero a que este desocupado el dispositivo
	while (ide_read(indexDevice, ATA_REG_STATUS) & ATA_SR_BSY);
	
	// ahora a leer el sector en cuestion
	ide_write(indexDevice,ATA_REG_SECCOUNT0, sectorCount);
	ide_write(indexDevice,ATA_REG_LBA0, lba0);
	ide_write(indexDevice,ATA_REG_LBA1, lba1);
	ide_write(indexDevice,ATA_REG_LBA2, lba2);
	ide_write(indexDevice,ATA_REG_HDDEVSEL, devSelected);
	ide_write(indexDevice,ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	
	
	// ahora esperamos la disponibilidad de datos
	err = 0;
	while(!err) {
            status = ide_read(indexDevice, ATA_REG_STATUS);
            if (status & ATA_SR_ERR) 
							err = 1;
            if (!err && !(status & ATA_SR_BSY) && (status & ATA_SR_DRDY)) 
							// Everything is right.
							err = 2;
	 }
	 if (err == 1)
	 {
		 // ocurrio un error, debo analizarlo
		 return 0;
	 }
	 
	 // todo bien leamos los datos
	 ide_read_data(indexDevice,buffer);
	 return 1;
}

/**
 * \brief Este es el hilo de acceso a los dispositvos de bloques AT
 * \param utcb
 */
void at_thread(void * utcb)
{
	// Configuro inicialmente el dispositivo
	InitDevices();
	
	// leo unos cuantos bloques
	for (int i = 0 ; i < 1000 ; i++)
	{
		if (ReadBlock(0,i,buffer))
		{
			printf("Leido %d \n\n\n",i);
			
		}
		else
			printf("Bloque no leido\n");
	}
	
	while (1)
	{
		// aqui espero por peticiones de bloques
		printf("Soy el hilo del ATA haraganeando\n");
		//PassToKernelMode();
	}
}

/**
 * \brief Crea un hilo para acceder a los dispositivos de bloque AT
 */
void Init_AT_Control_Thread()
{
	ThreadControl(&_at_thread,&pila[1024],&utcb[0]);
}
