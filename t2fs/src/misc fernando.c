#ifndef MISC_C
#define MISC_C


#include "../include/misc.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_LAA 10

typedef struct fileHandler{
	int fileHandle;	//handle do arquivo
	int posFile;		//current position do arquivo
	t2fs_record *fileRecord;		//pointer para o record do arquivo
} Handler;

// FUNÇÔES AUXILIARES

WORD wordConvert(int *pos, BYTE *buffer){
	int i;
	WORD bird, auxbuffer[2] = {0};
	for (i = 0; i < 2; i++){
		auxbuffer[i] = buffer[i+(*pos)];
	}
	bird = (unsigned short int) (auxbuffer[0] | (auxbuffer[1] << 8));
	(*pos) += 2;
	return bird;
}

DWORD dWordConvert(int pos, BYTE *buffer){
	int i;
	DWORD birdbird, auxbuffer[4] = {0};
	for (i = 0; i < 4; i++){
		theword = buffer[i+(*pos)];
	}
	birdbird = (unsigned int) (auxbuffer[0] | (auxbuffer[1] << 8) | (auxbuffer[2] << 16) | (auxbuffer[3] << 24));
	(*pos) += 4;
	return birdbird;
}

void initialize(int* initialized,t2fs_superbloco* superbloco,t2fs_record* rootDir){
	int i, *curr_pos, sector_aux;
	BYTE *buffer = (char*) malloc(SECTOR_SIZE);

	if (!(*initialized)){
		read_sector(0, buffer);
		for (i = 0; i < 4; i++){
			superbloco->id[i] = buffer[3-i];
		}
		(*curr_pos) = i + 1;
		superbloco->version = wordConvert(curr_pos, buffer);
		superbloco->SuperBlockSize = wordConvert(curr_pos, buffer);
		superbloco->DiskSize = dWordConvert(curr_pos, buffer);
		superbloco->NofSectors  = dWordConvert(curr_pos, buffer);
		superbloco->SectorsPerCluster = dWordConvert(curr_pos, buffer);
		superbloco->pFATSectorStart = dWordConvert(curr_pos, buffer);
		superbloco->RootDirCluster = dWordConvert(curr_pos, buffer);
		superbloco->DataSectorStart = dWordConvert(curr_pos, buffer);
		sector_aux = RootDirCluster * SectorsPerCluster;
		read_sector(sector_aux, buffer);
		rootDir->TypeVal = buffer[0];
		for(i = 0; i < MAX_FILE_NAME_SIZE; i++){
			rootDir->name[i] = buffer[MAX_FILE_NAME_SIZE-i];
		}
		(*curr_pos) = MAX_FILE_NAME_SIZE;
		rootDir->bytesFileSize = dWordConvert(curr_pos, buffer);
		rootDir->firstCluster = dWordConvert(curr_pos, buffer);
		(*initialized) = 1;
	}
}

int insereListaArqAbertos(t2fs_record* novo_record, t2fs_record* lista_arq_abertos){
	int i=0;

	while(i>MAX_LAA && lista_arq_abertos[i])
		i++;

	if (!lista_arq_abertos[i]){
		lista_arq_abertos[i] = malloc(sizeof(t2fs_record));

		lista_arq_abertos[i]->TypeVal = novo_record->TypeVal;
		strcpy(lista_arq_abertos[i]->name,novo_record->name )
		lista_arq_abertos[i]->bytesFileSize = novo_record->bytesFileSize;
		lista_arq_abertos[i]->firstCluster = novo_record->firstCluster;


		return i; //execução terminou com sucesso, devolve o handler
	}

	return NULL; //execução acabou com erros

}






#endif
//END OF FILE
