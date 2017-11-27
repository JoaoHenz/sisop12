#ifndef MISC_HEADER
#define MISC_HEADER

#include "../include/apidisk.h"
#include <stdio.h>
#include <stdlib.h>

#include "t2fs.h"

#define MAX_LAA 10

typedef struct fileHandler{
	int fileHandle;	//handle do arquivo
	int posFile;		//current position pointer do arquivo
	struct t2fs_record *fileRecord;		//pointer para o record do arquivo
} Handler;

WORD wordConvert(int *pos, BYTE *buffer);

DWORD dWordConvert(int *pos, BYTE *buffer);

void initialize(int* initialized,struct t2fs_superbloco* superbloco,struct t2fs_record* rootDir);

int insereListaArqAbertos(struct t2fs_record* novo_record,Handler* lista_arq_abertos[MAX_LAA]);

DWORD procuraClusterVazio(DWORD pFATSectorStart,DWORD DataSectorStart);


#endif
//END OF FILE
