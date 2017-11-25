#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"
#include "funcExtras.h"

int main(int argc,char *argv[])
{
	if(argc < 3)
	{
		printf("ERRO - Falta algum parametro\n");
		return -1;
	}
	if(argc > 3)
	{
		printf("ERRO - Muitos parametros\n");
		return -1;
	}
	char *arquivoOriginal = malloc(strlen(argv[1])+1);
	char *arquivoT2FS = malloc(strlen(argv[2])+1);
	char *temp = malloc(strlen(argv[2])+1);

	FILE *fp;
	int handle, i, UltimaBarra, foundDir;
	char c;

//	int bytesLidos;

	strcpy(arquivoOriginal, argv[1]);
	strcpy(arquivoT2FS, argv[2]);

	fp = fopen(arquivoOriginal,"r");
	if(!fp)
	{
		printf("ERRO - Arquivo original não pode ser aberto para cópia\n");
		return -1;
	}
	/**
		Verifica se diretório existe
	*/
	strcpy(temp,arquivoT2FS);
	//printf("temp: %s\n", temp);
	for(i=strlen(arquivoT2FS); i>0;i--)
	{
		if(*(temp+i) == '/')
		{
			UltimaBarra = i;
			//printf("temp Atual: %s\n",temp+i);
			break;
		}
	}
	*(temp+i) = '\0';
	//printf("temp: %s\n", temp);
	foundDir = findDiretorio(temp);
	//printf("flag : %d\n", foundDir);
	if(foundDir == -1)
	{
		//Não achou o arquivo especificado
		printf("ERRO - Não achou algum diretorio do path\n");
		return -1;
	}

	/**
		cria arquivo
	*/
	handle = t2fs_create(arquivoT2FS);
	if(handle==-1)
	{
		printf("ERRO - Não conseguiu criar novo arquivo no t2fs\n");
		return -1;
	}
	/**
		copia dados do arquivo para o novo arquivo criado
	*/
	while(1)
	{
		c = (char)fgetc(fp);
		if( feof(fp) )
		{ 
			break ;
		}
		t2fs_write (handle,&c,1);
//		printf("%c", c);
	}

	t2fs_close(handle);   
	fclose(fp);

	printf("COPIADO\n");

	free(temp);
	temp = NULL;
	free(arquivoOriginal);
	arquivoOriginal = NULL;	
	free(arquivoT2FS);
	arquivoT2FS = NULL;

	return 0;
}
