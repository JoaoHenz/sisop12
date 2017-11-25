#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"

int main()
{
	//teste open
	
	char *buffer;
	char buffer2[] = "/dir1/teste4-dir1";
	buffer = buffer2;
	int h = t2fs_open(buffer);

	if(h!=-1)
		printf("handle %d:, bloco - %d, offset - %d\n", listaHandles->fileHandle,listaHandles->bloco, listaHandles->offset);	

	printf("handle retornado: %d\n", h);

	
	//testes read e seek;
	
	char *texto = malloc(50);
	memset(texto, 0xff, 1);
	printf("texto dif: %x\n",(unsigned char)*texto);

	int lidos = t2fs_read (0, texto, 25);
	texto[20] = '\0';

	printf("\ntexto = %s\nlidos = %d\nHandle 0 offset = %d\n",texto,lidos,listaHandles->posCursor);

	t2fs_seek (0, 0);

	lidos = t2fs_read (0, texto, 10);
	texto[10] = '\0';

	printf("\ntexto = %s\nlidos = %d\nHandle 0 offset = %d\n",texto,lidos,listaHandles->posCursor);

	//tetes write e seek

//	char *bufferWrite = "eu sou bem melhor que isso"; //descomentar para testar write
//	t2fs_write (0, bufferWrite, 26);				//descomentar para testar write
	
	t2fs_seek (0, 0);

	lidos = t2fs_read (0, texto, 36);
	texto[36] = '\0';

	printf("\ntexto = %s\nlidos = %d\nHandle 0 offset = %d\n",texto,lidos,listaHandles->posCursor);
	
	//testes create

	//char arqDigits[] = "/dir2/teste6.txt";
	//char *arquivo = arqDigits;
	//t2fs_create(arquivo);

	//testes close

	t2fs_close(0);
	printf("handles number: %d\n", fileHandlerGenerator);
	
	//testes delete - descomenta-los para funcionar

//	char *bufferDelete;
//	char bufferTexto[] = "/dir1/teste4-dir1";

//	bufferDelete = bufferTexto;
//	t2fs_delete (bufferDelete);


	//free do main e outros
	free(texto);
	texto = NULL;

	return 0;
}
