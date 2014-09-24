
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int filtro(const struct dirent * d)
{
	
		int n = strlen(d->d_name);
		return (d->d_name[n-1] == 'X' && d->d_name[n-2]=='.');
}

typedef struct
{
	int offset;
	int len;
	char name[100];
} Entrada;


/**
 * \brief 
 * \param argc
 * \param argv  El argumento 0 es la imagen de destino, el resto son archivos que se pondran dentro de esa imagen
 * \return 
 */
int main(int argc, char **argv)
{
	
	if (argc != 3)
	{
		fprintf(stderr,"Usage: ramdisksCreator imageFile Dir\n");
		return 1;
	}
	
	
	struct dirent ** arr; 
	
	int n = scandir(argv[2],&arr,filtro,alphasort);
	
	int fo = open(argv[1],O_WRONLY | O_CREAT, S_IRWXU);
	
	if (!fo)
	{
		fprintf(stderr,"No pudo crearse el archivo de salida %s \n",argv[1]);
		return 1;
	}
	
	int l = strlen(argv[2]);
	char buffer[4096];
	int c;
	
	int offset = 0;
	// escribo al princio del archivo la cantidad de entradas
	write(fo,&n,4);
	offset += 4;
	
	// reservo espacio para cada entrada
	Entrada * entry = (Entrada*)malloc(sizeof(Entrada) * n); 
	write(fo, entry, sizeof(Entrada)*n);
	offset += sizeof(Entrada)*n;
	
	// le copio todos los archivos
	for (int i = 0 ; i < n ; i++)
	{
		printf("Archivo : %s \n",arr[i]->d_name);
		
		char* s = (char*)malloc(l + strlen(arr[i]->d_name) + 2);
		strcpy(s,argv[2]);
		s[l] = '/';
		strcpy(&s[l+1],arr[i]->d_name);
		
		
		int fin = open(s,O_RDONLY);
		int sum = 0;
		do
		{
			c = read(fin,buffer,4096);
			sum += c;
			write(fo,buffer,c);
		} while (c == 4096);
		
		entry[i].len = sum;
		entry[i].offset = offset;
		strcpy(entry[i].name, arr[i]->d_name);
		offset += sum;
		
		
		close(fin);
		
		free(s);
	}
	
	// le copio los valores de las entradas
	lseek(fo,4,SEEK_SET);
	write(fo, entry, sizeof(Entrada)*n);
	
	close(fo);
	
	return 0;
}
