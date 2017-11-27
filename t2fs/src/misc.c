#include "../include/misc.h"
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include <stdio.h>
#include <stdlib.h>

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

DWORD dWordConvert(int *pos, BYTE *buffer){
	int i;
	DWORD birdbird, auxbuffer[4] = {0};
	for (i = 0; i < 4; i++){
		//theword = buffer[i+(*pos)];  //whatever the fuck is going on here
		auxbuffer[i] = buffer[i+(*pos)];
	}
	birdbird = (unsigned int) (auxbuffer[0] | (auxbuffer[1] << 8) | (auxbuffer[2] << 16) | (auxbuffer[3] << 24));
	(*pos) += 4;
	return birdbird;
}

void initialize(int* initialized,struct t2fs_superbloco* superbloco,struct t2fs_record* rootDir){
	int i, *curr_pos, sector_aux, cluster_pos;
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
		sector_aux = superbloco->RootDirCluster / 4;
		read_sector(sector_aux, buffer);
		cluster_pos = 64 * (superbloco->RootDirCluster % 4);
		rootDir->TypeVal = buffer[cluster_pos];
		for(i = cluster_pos; i < (MAX_FILE_NAME_SIZE + cluster_pos); i++){
			rootDir->name[i] = buffer[MAX_FILE_NAME_SIZE + cluster_pos - i];
		}
		(*curr_pos) = MAX_FILE_NAME_SIZE + cluster_pos;
		rootDir->bytesFileSize = dWordConvert(curr_pos, buffer);
		rootDir->firstCluster = dWordConvert(curr_pos, buffer);
		(*initialized) = 1;
	}
}


int insereListaArqAbertos(struct t2fs_record* novo_record,Handler* lista_arq_abertos[MAX_LAA]){
	int i=0;

	while((i<MAX_LAA) && (lista_arq_abertos[i] != NULL)){
		i++;
	}

	if (lista_arq_abertos[i] == NULL){
		Handler *handler = malloc(sizeof(Handler));

		handler->fileHandle = i;
		handler->posFile;
		handler->fileRecord = novo_record;

		lista_arq_abertos[i] = handler;
		//(*lista_arq_abertos[i]) = (t2fs_record) malloc(sizeof(t2fs_record));

		//(*lista_arq_abertos[i])->TypeVal = novo_record->TypeVal;
		//strcpy((*lista_arq_abertos[i])->name,novo_record->name )
		//(*lista_arq_abertos[i])->bytesFileSize = novo_record->bytesFileSize;
		//(*lista_arq_abertos[i])->firstCluster = novo_record->firstCluster;


		return i; //execução terminou com sucesso, devolve o handler
	}

	return -1; //execução acabou com erros

}



DWORD procuraClusterVazio(DWORD pFATSectorStart,DWORD DataSectorStart){
	int i = pFATSectorStart, index = 0, cluster_index=-1;
	int flagAchou =0;
	char *buffer = malloc(SECTOR_SIZE);
	DWORD cluster;
	while((i<DataSectorStart) && (read_sector(i, buffer)==0) && (flagAchou!=1)){
		int j;
		i++;
		index = 0;
		for(j=0; j<64; j++){
			cluster = dWordConvert(&index, buffer);
			cluster_index++;
			printf("%u\n", cluster);
			if (cluster == 0x00000000){
				flagAchou=1;
				break;
			}
		}
	}
	if (flagAchou==1)
		return cluster_index;

	return -1;
}

int getFileRecord(struct t2fs_record* directory, char* filename, struct t2fs_record* file, struct t2fs_superbloco* superbloco){
	struct t2fs_record record[4];
	int found, cluster, cluster_aux, i, j;
	unsigned int sector, data_sector;
	BYTE *buffer, *buffer_aux, *buffer_record;
	DWORD next_cluster;

	// Read starter cluster from FAT
	sector = 1 + (directory->firstCluster / 64);
	cluster = directory->firstCluster % 64;
	while(!found){
		read_sector(sector, buffer);
		cluster_aux = cluster;
		for (i = 0; i < 4; i++){
			next_cluster = dWordConvert(&cluster_aux, buffer);
		}
		// Read the cluster (N sectors per cluster, so N*4 records per cluster)
		data_sector = superbloco->DataSectorStart + (cluster * (superbloco->SectorsPerCluster));
		for (i = 0; i < superbloco->SectorsPerCluster; i++){
			read_sector(data_sector + i, buffer);
			for (j = 0; j < 4; j++){
				//record[i] = (struct t2fs_record*) buffer[64*i];
				/*if(strcmp(record[i]->name, filename)){
					found = 1;
					file = record[i];
				}*/
			}
		}
		// Set up the next FAT SECTOR TO CHECK!
		sector = 1 + (next_cluster/64);
		cluster = next_cluster % 64;
	}
	return -1;
}


