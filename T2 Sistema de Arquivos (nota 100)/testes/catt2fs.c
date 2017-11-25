#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"

int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		printf("ERRO - Falta o path do arquivo\n");
		return -1;
	}
	if(argc > 2)
	{
		printf("ERRO - Muitos parametros\n");
		return -1;
	}
	char *arquivoT2FS = malloc(strlen(argv[1])+1);

	int handle, bytesLidos;
	char c;

	strcpy(arquivoT2FS, argv[1]);
	//printf("%s\n",arquivoT2FS);

	handle = t2fs_open(arquivoT2FS);
	//printf("handle: %d\n", handle);

	//printf("T2FS:\n");
	t2fs_seek (handle, 0);	
	while(1)
	{
		//arrumar read para handle -1;
		bytesLidos = t2fs_read (handle,&c,1);
		if( bytesLidos == 0 || bytesLidos == -1)
		{ 
			if(bytesLidos == -1)
			{
				printf("ERRO - Não conseguiu ler o arquivo\n");
			}
			break;
		}
		printf("%c", c);
	}
	t2fs_close(handle); 
	return 0;
}
