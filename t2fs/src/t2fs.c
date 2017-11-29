#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LAA 10
#define REC_TAM 64
#define RECS_IN_DIR 1024/64 //16
#define CLUSTER_SIZE 1024

/*
	identify2 = FEITO
	create2 	= PARCIAL
	open2 		= PARCIAL
	delete2		= sendo feito
	close2		= FEITO
	read2			=
	write2		=
	truncate2	=
	seek2			=
	mkdir2		= sendo feito pelo JOÂO
	rmdir2		=
	chdir2		=
	getcwd2		=
	opendir2	=

	STATUS: 26% Concluido

*/

typedef struct fileHandler{
	int fileHandle;	//handle do arquivo
	int posFile;		//current position pointer do arquivo ou currententry de um dir
	struct t2fs_record *fileRecord;		//pointer para o record do arquivo
	struct t2fs_record *dir; //pointer para o dir onde está o arquivo
} Handler;

struct t2fs_superbloco *superbloco;
int initialized = 0, handlerCount = 0, handlerNarutoCount = 0;
char *current_path;
struct t2fs_record *rootDir;
struct t2fs_record *currentDir;

Handler* lista_arq_abertos[MAX_LAA] = { NULL,NULL,NULL,NULL,NULL,
												NULL,NULL,NULL,NULL,NULL };

Handler* lista_dir_abertos[MAX_LAA] = { NULL,NULL,NULL,NULL,NULL,
												NULL,NULL,NULL,NULL,NULL };


#define INIT {initialize();}


/*****************************************************************************
*******************FUNÇÃOS AUXILIARES******************************************
********************************************************nigger*****************
*/
int parsePath(char* path, char* subdir, char* remainder){
	int i, j;
	char* nullstring = "";
	strcpy(subdir,nullstring);
	strcpy(remainder,nullstring);

	if(path[0] == '/'){
		i = 1;
	}
	else{
		i = 0;
	}
	j = 0;
	while(path[i] != '/' && path[i] != '\0'){
		subdir[j] = path[i];
		subdir[j+1] = '\0';
		i++;
		j++;
	}
	j = 0;
	while(path[i] != '\0'){
		remainder[j] = *(path + i);
		j++;
		i++;
	}
	remainder[j] = '\0';
	return 0;
}

int findDir(char* filename){
	return -1;
}

DWORD findFile(char* filename){
	unsigned int i;
	for(i = 0; i < 10; i++){
		if (strcmp(filename, lista_arq_abertos[i]->fileRecord->name) == 0){
			return i;
		}
	}
	return -1;
}

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

int write_cluster (DWORD cluster, char *buffer){
	int offset = superbloco->DataSectorStart;
	write_sector(offset + (cluster*4) + 0, buffer);
	write_sector(offset + (cluster*4) + 1, buffer+256);
	write_sector(offset + (cluster*4) + 2, buffer+512);
	write_sector(offset + (cluster*4) + 3, buffer+768);
	return 0;
}

int read_cluster (DWORD cluster, char *buffer){
	int offset = superbloco->DataSectorStart;
	read_sector(offset + (cluster*4) + 0, buffer);
	read_sector(offset + (cluster*4) + 1, buffer+256);
	read_sector(offset + (cluster*4) + 2, buffer+512);
	read_sector(offset + (cluster*4) + 3, buffer+768);
	return 0;
}

void initialize(){
	int i, *curr_pos, sector_aux, cluster_pos;
	BYTE *buffer = (char*) malloc(SECTOR_SIZE);

	if (!initialized){
		superbloco = (struct t2fs_superbloco *) malloc(256);
		read_sector(0,(char *) superbloco);

		rootDir = (struct t2fs_record*) malloc(sizeof(struct t2fs_record));
		rootDir->TypeVal = 0x02;
		strcpy(rootDir->name, "root");
		//rootDir->name = rootName;
		rootDir->bytesFileSize = (superbloco->SectorsPerCluster) * (SECTOR_SIZE);
		rootDir->firstCluster = superbloco->RootDirCluster;

		currentDir = rootDir;
		current_path = malloc(5);
		strcpy(current_path, "/");
		initialized = 1;
	}
}
int insereListaDirAbertos(struct t2fs_record* novo_record, struct t2fs_record *dir){
	int i=0, j;

	while((i<MAX_LAA) && (lista_dir_abertos[i] != NULL)){
		i++;
	}

	if (lista_dir_abertos[i] == NULL){
		Handler *handler = malloc(sizeof(Handler));

		handler->fileHandle = i;
		handler->posFile = 0;
		handler->fileRecord = novo_record;
		handler->dir = dir;

		lista_dir_abertos[i] = handler;

		handlerNarutoCount++;
		return i; //execução terminou com sucesso, devolve o handler
	}

	return -1; //execução acabou com erros

}


int insereListaArqAbertos(struct t2fs_record* novo_record, struct t2fs_record *dir){
	int i=0, j;

	while((i<MAX_LAA) && (lista_arq_abertos[i] != NULL)){
		i++;
	}


	if (lista_arq_abertos[i] == NULL){
		Handler *handler = malloc(sizeof(Handler));

		handler->fileHandle = i;
		handler->posFile = 0;
		handler->fileRecord = novo_record;
		handler->dir = dir;

		lista_arq_abertos[i] = handler;
		//(*lista_arq_abertos[i]) = (t2fs_record) malloc(sizeof(t2fs_record));

		//(*lista_arq_abertos[i])->TypeVal = novo_record->TypeVal;
		//strcpy((*lista_arq_abertos[i])->name,novo_record->name )
		//(*lista_arq_abertos[i])->bytesFileSize = novo_record->bytesFileSize;
		//(*lista_arq_abertos[i])->firstCluster = novo_record->firstCluster;

		handlerCount++;
		return i; //execução terminou com sucesso, devolve o handler
	}

	return -1; //execução acabou com erros

}

DWORD procuraClusterVazio(){
	int i = superbloco->pFATSectorStart, index = 0, cluster_index=-1;
	int flagAchou =0;
	char *buffer = malloc(SECTOR_SIZE);
	DWORD cluster;
	while((i<superbloco->DataSectorStart) && (read_sector(i, buffer)==0) && (flagAchou!=1)){
		int j;
		i++;
		index = 0;
		for(j=0; j<64; j++){
			cluster = dWordConvert(&index, buffer);
			cluster_index++;
			//printf("%u\n", cluster);
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

int getFileRecord(struct t2fs_record* directory, char* filename, struct t2fs_record* file){
	BYTE *buffer = malloc(superbloco->SectorsPerCluster * SECTOR_SIZE);
	int i, max_records;

	max_records = superbloco->SectorsPerCluster * 4;

	read_cluster(directory->firstCluster,buffer);
	for(i = 0; i < max_records; i++){
		memcpy(file, buffer + (i*REC_TAM), sizeof(struct t2fs_record));
		if(strcmp(file->name, filename) == 0){
			return 0;
		}
	}
	return -1;
}

int write_rec_to_disk(struct t2fs_record *rec){
	//writes given file record in the current dir
	char *buffer = malloc(superbloco->SectorsPerCluster * SECTOR_SIZE);
	read_cluster(currentDir->firstCluster, buffer);
	int free_rec = find_free_rec_in_dir(buffer);
	insert_rec_in_dir(buffer, free_rec, rec);

	write_cluster(currentDir->firstCluster, buffer);
	free(buffer);

	return 0;
}

int insert_rec_in_dir(char *dirCluster, int rec_index, struct t2fs_record *rec){
	memcpy(dirCluster + (rec_index*REC_TAM), rec, sizeof(struct t2fs_record));
	return 0;
}

int find_free_rec_in_dir(char *dirCluster){
	int i = 0;
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));

	for(i=0; i<RECS_IN_DIR; i++){
		memcpy(record, dirCluster + (i*REC_TAM), sizeof(struct t2fs_record));
		//printf("%s\n",record->name );
		if(record->TypeVal == 0x00){
			//free record
			free(record);
			return i;
		}
	}
	free(record);
	return -1;
}

int print_dir(struct t2fs_record *dir){
	int i = 0;
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	char *dirCluster = malloc(CLUSTER_SIZE);
	read_cluster(dir->firstCluster, dirCluster);

	for(i=0; i<RECS_IN_DIR; i++){
		memcpy(record, dirCluster + (i*REC_TAM), sizeof(struct t2fs_record));
		printf("%s\n",record->name );
	}
	free(record);
	free(dirCluster);
	return 0;
}

int find_name_rec_in_dir(char *dirCluster, char *filename){
	int i = 0;
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));

	for(i=0; i<RECS_IN_DIR; i++){
		memcpy(record, dirCluster + (i*REC_TAM), sizeof(struct t2fs_record));
		//printf("%s\n",record->name );
		if(strcmp(record->name, filename) == 0){
			//found record
			free(record);
			return i;
		}
	}
	free(record);
	return -1;
}

int mark_EOF(DWORD cluster_index){
	char *buffer = malloc(SECTOR_SIZE);
	DWORD sector_index = superbloco->pFATSectorStart + ((DWORD)(cluster_index/64));
	read_sector(sector_index,buffer);
	unsigned int eof = 0xFFFFFFFF;
	int offset = (cluster_index - ((sector_index-1)*64))*4;
	memcpy(buffer+offset, &eof, sizeof(DWORD));
	write_sector(sector_index, buffer);
	free(buffer);

	return 0;
}


int mark_next(DWORD cluster_index, unsigned int bird){
	char *buffer = malloc(SECTOR_SIZE);
	DWORD sector_index = superbloco->pFATSectorStart + ((DWORD)(cluster_index/64));
	read_sector(sector_index,buffer);
	int offset = (cluster_index - ((sector_index-1)*64))*4;
	memcpy(buffer+offset, &bird, sizeof(DWORD));
	write_sector(sector_index, buffer);
	free(buffer);

	return 0;
}

int mark_free(DWORD cluster_index){
	char *buffer = malloc(SECTOR_SIZE);
	DWORD sector_index = superbloco->pFATSectorStart + ((DWORD)(cluster_index/64));
	read_sector(sector_index,buffer);
	unsigned int fr = 0x00000000;
	int offset = (cluster_index - ((sector_index-1)*64))*4;
	memcpy(buffer+offset,&fr, sizeof(DWORD));
	write_sector(sector_index, buffer);
	free(buffer);

	return 0;
}

DWORD get_next_cluster(DWORD cluster_index){
	char *buffer = malloc(SECTOR_SIZE);
	DWORD sector_index = superbloco->pFATSectorStart + ((DWORD)(cluster_index/64));
	read_sector(sector_index,buffer);
	DWORD point_value;
	int offset = (cluster_index - ((sector_index-1)*64))*4;
	memcpy(&point_value,buffer+offset, sizeof(DWORD));
	free(buffer);

	return point_value;
}




// FUN��ES PRINCIPAIS DA BILBIOTECA

int identify2 (char *name, int size){ INIT;
	int i;
	char *ident = "Fernando Garcia Bock 242255\nLeonardo Wellausen 261571\nJoao Batista Manique Henz 242251\n\0";

	if (size >= sizeof(ident)){
		strcpy(name, ident);
		return 0;
	}
	else{
		strncpy(name, ident, size);
		return 0;
	}

	return -1;
	// PLACEHOLDER DE FUN��O TBQH
	/*-----------------------------------------------------------------------------
	Fun��o: Usada para identificar os desenvolvedores do T2FS.
		Essa fun��o copia um string de identifica��o para o ponteiro indicado por "name".
		Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".
		O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por �\0�.
		O string deve conter o nome e n�mero do cart�o dos participantes do grupo.

	Entra:	name -> buffer onde colocar o string de identifica��o.
		size -> tamanho do buffer "name" (n�mero m�ximo de bytes a serem copiados).

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

FILE2 create2 (char *filename){ INIT;
	char *aux_path = malloc(64);
	char *subdir = malloc(64);
	char *remainder = malloc(64);
	char *nullstring = "\0";

	struct t2fs_record* novo_record = malloc(sizeof(struct t2fs_record));
	struct t2fs_record* record_aux = malloc(sizeof(struct t2fs_record));
	struct t2fs_record* old_dir = malloc(sizeof(struct t2fs_record));
	novo_record->TypeVal = 0x01;//tipo arquivo simples
	strcpy(novo_record->name,filename);
	novo_record->bytesFileSize = 0;
	novo_record->firstCluster = procuraClusterVazio() ;

	if(novo_record->firstCluster == -1){
		//TODO tratamento limpar o que foi feito antes
		return -1;
	}
	int handle = insereListaArqAbertos(novo_record, currentDir);
	mark_EOF(novo_record->firstCluster);

	parsePath(filename, subdir, remainder);
	while(strcmp(remainder, nullstring) != 0){
		strcpy(subdir, nullstring);
		strcpy(aux_path, remainder);
		strcpy(remainder, nullstring);
		parsePath(aux_path, subdir, remainder);
		//printf("%s\n%s\n\n", subdir, remainder);
		if (getFileRecord(record_aux, subdir, record_aux) == -1) return -1;
	}
	memcpy(old_dir,currentDir,sizeof(struct t2fs_record));
	memcpy(currentDir,record_aux,sizeof(struct t2fs_record));
	//parsePath(char* path, char* subdir, char* remainder)
	write_rec_to_disk(novo_record);
	memcpy(currentDir,old_dir,sizeof(struct t2fs_record));

	if(handle)
		return handle;
	else
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o: Criar um novo arquivo.
		O nome desse novo arquivo � aquele informado pelo par�metro "filename".
		O contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
		Caso j� exista um arquivo ou diret�rio com o mesmo nome, a fun��o dever� retornar um erro de cria��o.
		A fun��o deve retornar o identificador (handle) do arquivo.
		Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.

	Entra:	filename -> path absoluto para o arquivo a ser criado. Todo o "path" deve existir.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo).
		Em caso de erro, deve ser retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}

int delete2 (char *filename){ INIT;
	//TODO PATH

	DWORD isOpen = findFile(filename);
	if(isOpen != -1){
		close2(isOpen);
	}


	struct t2fs_record* novo_record = (struct t2fs_record *) malloc(sizeof(struct t2fs_record));
	struct t2fs_record *deleted_record = (struct t2fs_record *) malloc(sizeof(struct t2fs_record));
	struct t2fs_record *old_dir= (struct t2fs_record *) malloc(sizeof(struct t2fs_record));
	memcpy(old_dir,currentDir,sizeof(struct t2fs_record));
	chdir2(filename);

	DWORD firstCluster = deleted_record->firstCluster;
	free(deleted_record);
	novo_record->TypeVal = 0x00;//tiporecord livre
	strcpy(novo_record->name,"");
	novo_record->bytesFileSize = 0;
	novo_record->firstCluster = 0;

	write_rec_to_disk(novo_record);
	free(novo_record);
	//deletado da entrada do diretório

	//limpando na FAT
	DWORD next_cluster ;


	do {
		next_cluster = get_next_cluster(firstCluster);
		mark_free(firstCluster);
		firstCluster = next_cluster;
	} while(next_cluster != 0xFFFFFFFF);

	memcpy(currentDir,old_dir,sizeof(struct t2fs_record));

	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Apagar um arquivo do disco.
		O nome do arquivo a ser apagado � aquele informado pelo par�metro "filename".

	Entra:	filename -> nome do arquivo a ser apagado.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

FILE2 open2 (char *filename){ INIT;
	char *aux_path = malloc(64);
	char *subdir = malloc(64);
	char *remainder = malloc(64);
	char *nullstring = "\0";
	struct t2fs_record* new_record = malloc(sizeof(struct t2fs_record));
	struct t2fs_record* record_aux = malloc(sizeof(struct t2fs_record));

	//FERNANDO VÊ ISSO PRA EU
	struct t2fs_record *this_dir;

	parsePath(filename, subdir, remainder);
	//printf("%s\n%s\n\n", subdir,remainder);
	if(filename[0] == '.'){ //
		this_dir = currentDir;
		if (getFileRecord(currentDir, subdir, record_aux) == -1) return -1;
	}
	else{
		this_dir = rootDir;
		if (getFileRecord(rootDir, subdir, record_aux) == -1) return -1;
	}
	while(strcmp(remainder, nullstring) != 0){
		strcpy(subdir, nullstring);
		strcpy(aux_path, remainder);
		strcpy(remainder, nullstring);
		parsePath(aux_path, subdir, remainder);
		//printf("%s\n%s\n\n", subdir, remainder);
		this_dir = record_aux;
		if (getFileRecord(record_aux, subdir, record_aux) == -1) return -1;
	}
	memcpy(new_record, record_aux, sizeof(struct t2fs_record));
	//int getFileRecord(struct t2fs_record* directory, char* filename, struct t2fs_record* file){

	int handle = insereListaArqAbertos(new_record, this_dir);
	free(record_aux);
	return handle;
	/*-----------------------------------------------------------------------------
	Fun��o:	Abre um arquivo existente no disco.
		O nome desse novo arquivo � aquele informado pelo par�metro "filename".
		Ao abrir um arquivo, o contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
		A fun��o deve retornar o identificador (handle) do arquivo.
		Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.
		Todos os arquivos abertos por esta chamada s�o abertos em leitura e em escrita.
		O ponto em que a leitura, ou escrita, ser� realizada � fornecido pelo valor current_pointer (ver fun��o seek2).

	Entra:	filename -> nome do arquivo a ser apagado.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo)
		Em caso de erro, deve ser retornado um valor negativo
	-----------------------------------------------------------------------------*/
}

int close2 (FILE2 handle){ INIT;
	if (lista_arq_abertos[handle]->fileRecord->TypeVal == 0x01){
		free(lista_arq_abertos[handle]->fileRecord);
		lista_arq_abertos[handle] = NULL;
		handlerCount--;
		return 0;
	}

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o arquivo identificado pelo par�metro "handle".

	Entra:	handle -> identificador do arquivo a ser fechado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int read2 (FILE2 handle, char *buffer, int size){ INIT;
	DWORD currentCluster = lista_arq_abertos[handle]->fileRecord->firstCluster;
	DWORD next_cluster;
	int FATClusterIndex, i = 0;
	int currentPointer = lista_arq_abertos[handle]->posFile;
	FATClusterIndex = currentPointer / (256 * superbloco->SectorsPerCluster);
	char *bufferAux = malloc(CLUSTER_SIZE);
	int clusterPointer = currentPointer % CLUSTER_SIZE;
	int currentSize = 0;

	while(i < FATClusterIndex){
		next_cluster = get_next_cluster(currentCluster);
		currentCluster = next_cluster;

		i++;
	}
	read_cluster(currentCluster, bufferAux);
	memcpy(buffer, bufferAux + clusterPointer, CLUSTER_SIZE - clusterPointer);
	next_cluster = get_next_cluster(currentCluster);
	currentCluster = next_cluster;
	currentSize += CLUSTER_SIZE - clusterPointer;
	while(next_cluster != 0xFFFFFFFF || currentSize + CLUSTER_SIZE < size){
		read_cluster(currentCluster, bufferAux);
		memcpy(buffer, bufferAux, CLUSTER_SIZE);
		next_cluster = get_next_cluster(currentCluster);
		currentCluster = next_cluster;
		currentSize += CLUSTER_SIZE;
	}
	if (currentSize + CLUSTER_SIZE < size){
		read_cluster(currentCluster, bufferAux);
		memcpy(buffer, bufferAux, size - currentSize);
	}
	else if (next_cluster != 0xFFFFFFFF){
		read_cluster(currentCluster, bufferAux);
		memcpy(buffer, bufferAux, CLUSTER_SIZE);
	}

	return 0;
	/*
	do {
		next_cluster = get_next_cluster(firstCluster);
		firstCluster = next_cluster;
		i++;
	} while(next_cluster != 0xFFFFFFFF);*/

	/*int FATClusterIndex, FATSectorIndex, fileRead;
	int currentPointer = lista_arq_abertos[handle]->posFile;
	int firstCluster = lista_arq_abertos[handle]->fileRecord->firstCluster;
	char *bufferAux = malloc(256);
	char

	if (handle >= MAX_LAA || handle < 0){ // handler out of bounds error
		return -1;
	}

	FATClusterIndex = firstCluster + currentPointer / (256 * superbloco->SectorsPerCluster);
	FATSectorIndex = superbloco->pFATSectorStart + (FATClusterIndex / 64);

	while(!fileRead){
		read_sector(FATSectorIndex,bufferAux);

	}*/


	/*
	Handler *handler = lista_arq_abertos[handle];
	struct t2fs_record *this_record = handler->fileRecord;

	DWORD clusters_toread = (size+(handler->posFile%CLUSTER_SIZE) + (CLUSTER_SIZE-1))/CLUSTER_SIZE; //ROUND UP
	DWORD current_cluster = handler->posFile/CLUSTER_SIZE;
	DWORD cluster_index = -1;
	DWORD firstCluster = this_record->firstCluster;
	int i=0;
	int bytes_read = 0;
	char *cluster = malloc(CLUSTER_SIZE);

	for(i=0; i<current_cluster; i++){
		cluster_index = get_next_cluster(firstCluster);
		firstCluster = cluster_index;
	}
	i=0;

	int offset = handler->posFile%CLUSTER_SIZE;

	if(clusters_toread == 1){
		read_cluster(cluster_index, cluster);
		memcpy(buffer, cluster + offset, size);
		free(cluster);
	}else if (clusters_toread == 2){
		read_cluster(cluster_index, cluster);
		memcpy(buffer, cluster + (offset), CLUSTER_SIZE - offset );
		read_cluster(get_next_cluster(cluster_index), cluster);
		memcpy(buffer + CLUSTER_SIZE - offset, cluster, size - (CLUSTER_SIZE - offset) );
		free(buffer);
	}
	else{
		read_cluster(cluster_index, cluster);

		memcpy(buffer, cluster + (offset), CLUSTER_SIZE - offset );

		//for(i=1;) finish this for here

	}


	return -1;*/
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
		Os bytes lidos s�o colocados na �rea apontada por "buffer".
		Ap�s a leitura, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo lido.

	Entra:	handle -> identificador do arquivo a ser lido
		buffer -> buffer onde colocar os bytes lidos do arquivo
		size -> n�mero de bytes a serem lidos

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes lidos.
		Se o valor retornado for menor do que "size", ent�o o contador de posi��o atingiu o final do arquivo.
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}

int write2 (FILE2 handle, char *buffer, int size){ INIT;
	int i =0, j=0, l=0;
	int size_sectors = size/SECTOR_SIZE;
	int clusters_ajeitados =lista_arq_abertos[handle]->posFile/(SECTOR_SIZE*superbloco->SectorsPerCluster)-1;
	DWORD firstCluster, next_cluster;

	firstCluster = lista_arq_abertos[handle]->fileRecord->firstCluster;
	do {
		next_cluster = get_next_cluster(firstCluster);
		if(next_cluster != 0xFFFFFFFF){
			break;
		}
		firstCluster = next_cluster;
	} while(next_cluster != 0xFFFFFFFF);

	int clusternoqualestouescrevendo = firstCluster;


	DWORD clusterVazio;

	while(i<size_sectors){ //enquanto nao terminar de gravar
		while(lista_arq_abertos[handle]->posFile%(CLUSTER_SIZE*clusters_ajeitados)<CLUSTER_SIZE){ //enquanto ele ainda nao tiver ocupado o cluster atual
			write_sector(clusternoqualestouescrevendo*superbloco->SectorsPerCluster+j, buffer+i*SECTOR_SIZE);

			lista_arq_abertos[handle]->posFile += SECTOR_SIZE;
			i++; // diz o quanto falta para terminar de gravar o buffer
			j++;//diz em qual sector deste cluster nós estamos
		}
		j=0;
		clusters_ajeitados++;

		clusterVazio = procuraClusterVazio();
		if (clusterVazio == -1)
			return -1;
		mark_next(clusternoqualestouescrevendo,clusterVazio);
		mark_EOF(clusterVazio);

		clusternoqualestouescrevendo = clusterVazio;
		//faz o processamento para alocar um novo cluster
	}
	lista_arq_abertos[handle]->fileRecord->bytesFileSize += size;
	// WRITE UPDATED RECORD
	return 0;
	//atualizar negócio do arquivo no handler disco

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
		Os bytes a serem escritos est�o na �rea apontada por "buffer".
		Ap�s a escrita, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo escrito.

	Entra:	handle -> identificador do arquivo a ser escrito
		buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
		size -> n�mero de bytes a serem escritos

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes efetivamente escritos.
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}

int truncate2 (FILE2 handle){ INIT;

	struct t2fs_record *record = lista_arq_abertos[handle]->fileRecord;
	Handler *handler = lista_arq_abertos[handle];

	DWORD curr_cluster = handler->posFile/CLUSTER_SIZE;
	DWORD firstCluster = record->firstCluster;
	DWORD next_cluster = -1;
	record->bytesFileSize = handler->posFile;

	struct t2fs_record *currentDirAntigo = malloc(REC_TAM);
	memcpy (currentDirAntigo, currentDir, REC_TAM);
	currentDir = handler->dir;
	write_rec_to_disk(record);
	memcpy(currentDir, currentDirAntigo, REC_TAM);
	free(currentDirAntigo);

	int i = 0;

	do {
		next_cluster = get_next_cluster(firstCluster);
		if(i>curr_cluster){
			mark_free(firstCluster);
		}
		firstCluster = next_cluster;
		i++;
	} while(next_cluster != 0xFFFFFFFF);



	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fun��o usada para truncar um arquivo.
		Remove do arquivo todos os bytes a partir da posi��o atual do contador de posi��o (CP)
		Todos os bytes a partir da posi��o CP (inclusive) ser�o removidos do arquivo.
		Ap�s a opera��o, o arquivo dever� contar com CP bytes e o ponteiro estar� no final do arquivo

	Entra:	handle -> identificador do arquivo a ser truncado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int seek2 (FILE2 handle, unsigned int offset){ INIT;

	if((handle<MAX_LAA)&&(handle>=0)){
		if(offset!=-1){
			lista_arq_abertos[handle]->posFile = offset;
			return 0;
		}
		else{
			lista_arq_abertos[handle]->posFile = lista_arq_abertos[handle]->fileRecord->bytesFileSize+1;
			return 0;
		}

	}

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Reposiciona o contador de posi��es (current pointer) do arquivo identificado por "handle".
		A nova posi��o � determinada pelo par�metro "offset".
		O par�metro "offset" corresponde ao deslocamento, em bytes, contados a partir do in�cio do arquivo.
		Se o valor de "offset" for "-1", o current_pointer dever� ser posicionado no byte seguinte ao final do arquivo,
			Isso � �til para permitir que novos dados sejam adicionados no final de um arquivo j� existente.

	Entra:	handle -> identificador do arquivo a ser escrito
		offset -> deslocamento, em bytes, onde posicionar o "current pointer".

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int mkdir2 (char *pathname){ INIT;
	char *aux_path = malloc(64);
	char *subdir = malloc(64);
	char *remainder = malloc(64);
	char *nullstring = "\0";
	struct t2fs_record* new_record = malloc(sizeof(struct t2fs_record));
	struct t2fs_record* record_aux = malloc(sizeof(struct t2fs_record));

	//FERNANDO VÊ ISSO PRA EU
	struct t2fs_record *this_dir;

	parsePath(pathname, subdir, remainder);
	//printf("%s\n%s\n\n", subdir,remainder);
	if(pathname[0] == '.'){ //
		this_dir = currentDir;
		if (getFileRecord(currentDir, subdir, record_aux) == -1) return -1;
	}
	else{
		this_dir = rootDir;
		getFileRecord(rootDir, subdir, record_aux) ;
	}
	while(strcmp(remainder, nullstring) != 0){
		strcpy(subdir, nullstring);
		strcpy(aux_path, remainder);
		strcpy(remainder, nullstring);
		parsePath(aux_path, subdir, remainder);
		//printf("%s\n%s\n\n", subdir, remainder);
		this_dir = record_aux;
		getFileRecord(record_aux, subdir, record_aux);
	}

	struct t2fs_record *currentDirAntigo = malloc(REC_TAM);
	memcpy(currentDirAntigo, currentDir, REC_TAM);
	memcpy(currentDir, this_dir, REC_TAM);


	struct t2fs_record* novo_record = malloc(sizeof(struct t2fs_record));

	novo_record->TypeVal = 0x02; //tipo arquivo simples
	novo_record->bytesFileSize = CLUSTER_SIZE;
	strcpy(novo_record->name ,subdir);
	novo_record->firstCluster = procuraClusterVazio() ;

	if(novo_record->firstCluster == -1){
		//TODO tratamento limpar o que foi feito antes
		return -1;
	}

	mark_EOF(novo_record->firstCluster);
	DWORD firstCluster = novo_record->firstCluster;
	write_rec_to_disk(novo_record);

	char *cluster_buffer = malloc(CLUSTER_SIZE);


	strcpy(novo_record->name ,".");

	memcpy(cluster_buffer + (0*REC_TAM),novo_record, REC_TAM);
	strcpy(novo_record->name ,"..");
	novo_record->firstCluster = currentDir->firstCluster;
	memcpy(cluster_buffer + (1*REC_TAM),novo_record, REC_TAM);

	int i;

	novo_record->TypeVal = 0x00;//tipo arquivo simples
	strcpy(novo_record->name ,"");
	novo_record->bytesFileSize = 0;
	novo_record->firstCluster = 0 ;
	for(i=2;i<RECS_IN_DIR;i++){
		memcpy(cluster_buffer + (i*REC_TAM),novo_record, REC_TAM);
	}

	write_cluster(firstCluster, cluster_buffer);

	memcpy(currentDir, currentDirAntigo, REC_TAM);

	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Criar um novo diret�rio.
		O caminho desse novo diret�rio � aquele informado pelo par�metro "pathname".
			O caminho pode ser ser absoluto ou relativo.
		S�o considerados erros de cria��o quaisquer situa��es em que o diret�rio n�o possa ser criado.
			Isso inclui a exist�ncia de um arquivo ou diret�rio com o mesmo "pathname".

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int rmdir2 (char *pathname){ INIT;
	//TODO malandragem para ler o dir com o pathname e colocá-lo em dir
	//setar o currDir como o pai do dir a ser removido

	struct t2fs_record *currentDirAntigo = malloc(REC_TAM);
	memcpy(currentDirAntigo, currentDir, REC_TAM);
	struct t2fs_record *dir = malloc(REC_TAM);
	struct t2fs_record *entry = malloc(REC_TAM);
	int i;
	char *buffer = malloc(CLUSTER_SIZE);
	read_cluster(dir->firstCluster, buffer);
	//check if there are files

	memcpy(currentDir, buffer + (1*REC_TAM), REC_TAM); //passa o pai para o currentDir
	for(i=2; i<RECS_IN_DIR; i++){  //entries 0 and 1 are . and .. and are always there
		memcpy(entry, buffer + (i*REC_TAM), REC_TAM);
		//printf("%s\n",record->name );
		if(entry->TypeVal != 0x00){
			free(entry);
			free(buffer);
			free(dir);
			//retorna o currDir de verdade
			memcpy(currentDir, currentDirAntigo, REC_TAM);
			free(currentDirAntigo);

			return -1; //dir is not empty
		}
	}
	free(buffer);

	//move on to deleting the dir
	mark_free(dir->firstCluster);

	dir->TypeVal = 0x00;//tiporecord livre
	strcpy(dir->name,"");
	dir->bytesFileSize = 0;
	dir->firstCluster = 0;

	write_rec_to_disk(dir);
	free(dir);

	//retorna o currDir de verdade
	memcpy(currentDir, currentDirAntigo, REC_TAM);
	free(currentDirAntigo);


	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Apagar um subdiret�rio do disco.
		O caminho do diret�rio a ser apagado � aquele informado pelo par�metro "pathname".
		S�o considerados erros quaisquer situa��es que impe�am a opera��o.
			Isso inclui:
				(a) o diret�rio a ser removido n�o est� vazio;
				(b) "pathname" n�o existente;
				(c) algum dos componentes do "pathname" n�o existe (caminho inv�lido);
				(d) o "pathname" indicado n�o � um arquivo;

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int chdir2 (char *pathname){ INIT;
	/*int i=0,j=0,profundidade_cont=0, path_is_valid=1;
	struct t2fs_record* buffer_record = malloc(sizeof(struct t2fs_record));
	char parsed_path[100][100];

	while(pathname[i]!='\0'){
		while((pathname[i]!='/') && (pathname[i]!='\0')){
			parsed_path[profundidade_cont][j] = pathname[i];
			i++; j++;
		}
		parsed_path[profundidade_cont][j] = '\0';
		j=0;
		if(pathname[i]!='/') profundidade_cont++;
		i++;
	}

	i=0;

	while(i<profundidade_cont && path_is_valid ==0){ //enquanto nao tiver chegado no path e enquanto estiver seguindo paths válidos
		path_is_valid = getFileRecord(currentDir,parsed_path[i],buffer_record);
		if(path_is_valid==0){
			memcpy(currentDir,buffer_record, REC_TAM);
		}
	}

	if(path_is_valid==0)
		return 0;*/

		char *aux_path = malloc(64);
		char *subdir = malloc(64);
		char *remainder = malloc(64);
		char *nullstring = "\0";
		//struct t2fs_record* new_record = malloc(sizeof(struct t2fs_record));
		struct t2fs_record* record_aux = malloc(sizeof(struct t2fs_record));


		parsePath(pathname, subdir, remainder);
		//printf("%s\n%s\n\n", subdir,remainder);
		if(pathname[0] == '.'){ //
			if (getFileRecord(currentDir, subdir, record_aux) == -1) return -1;
		}
		else{
			if (getFileRecord(rootDir, subdir, record_aux) == -1) return -1;
		}
		while(strcmp(remainder, nullstring) != 0){
			strcpy(subdir, nullstring);
			strcpy(aux_path, remainder);
			strcpy(remainder, nullstring);
			parsePath(aux_path, subdir, remainder);
			//printf("%s\n%s\n\n", subdir, remainder);
			if (getFileRecord(record_aux, subdir, record_aux) == -1) return -1;
		}

		memcpy(currentDir, record_aux, REC_TAM);
		free(record_aux);
		free(aux_path);
		free(subdir);
		free(remainder);

		free(current_path);
		current_path = malloc(strlen(pathname));
		strcpy(current_path, pathname);


	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Altera o current path
		O novo caminho do diret�rio a ser usado como current path � aquele informado pelo par�metro "pathname".
		S�o considerados erros quaisquer situa��es que impe�am a opera��o.
			Isso inclui:
				(a) "pathname" n�o existente;
				(b) algum dos componentes do "pathname" n�o existe (caminho inv�lido);
				(c) o "pathname" indicado n�o � um diret�rio;

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int getcwd2 (char *pathname, int size){ INIT;
	//TODO ESTAMOS MANTENDO O current_path ATUALIZADO FOR REAL BOYZ?

	if(size < strlen(current_path)){
		return -1;  //size é muito pequeno
	}

	memcpy(pathname, current_path, strlen(current_path));
	return 0;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fun��o que informa o diret�rio atual de trabalho.
		O T2FS deve copiar o pathname do diret�rio de trabalho, incluindo o �\0� do final do string, para o buffer indicado por "pathname".
		Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".

	Entra:	pathname -> ponteiro para buffer onde copiar o pathname
		size -> tamanho do buffer "pathname" (n�mero m�ximo de bytes a serem copiados).

	Sa�da:
		Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Caso ocorra algum erro, a fun��o retorna um valor diferente de zero.
		S�o considerados erros quaisquer situa��es que impe�am a c�pia do pathname do diret�rio de trabalho para "pathname",
		incluindo espa�o insuficiente, conforme informado por "size".
	-----------------------------------------------------------------------------*/
}

DIR2 opendir2 (char *pathname){ INIT;
	struct t2fs_record* currentDirAntigo = malloc(sizeof(struct t2fs_record));
	int flag;
	int handle = -1;

	memcpy(currentDirAntigo,currentDir, REC_TAM);
	flag =chdir2(pathname);

	if(flag==0){
		struct t2fs_record* new_record = malloc(sizeof(struct t2fs_record));
		struct t2fs_record* daddy_record = malloc(REC_TAM);
		memcpy(new_record, currentDir, REC_TAM);

		char *buffer = malloc(CLUSTER_SIZE);
		read_cluster(new_record->firstCluster, buffer);
		memcpy(daddy_record, buffer + (1*REC_TAM), REC_TAM); //passa o pai para o currentDir
		free(buffer);

		handle = insereListaDirAbertos(new_record,daddy_record);
	}

	memcpy(currentDir,currentDirAntigo, REC_TAM);
	return handle;
	/*-----------------------------------------------------------------------------
	Fun��o:	Abre um diret�rio existente no disco.
		O caminho desse diret�rio � aquele informado pelo par�metro "pathname".
		Se a opera��o foi realizada com sucesso, a fun��o:
			(a) deve retornar o identificador (handle) do diret�rio
			(b) deve posicionar o ponteiro de entradas (current entry) na primeira posi��o v�lida do diret�rio "pathname".
		O handle retornado ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do diret�rio.

	Entra:	pathname -> caminho do diret�rio a ser aberto

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o identificador do diret�rio (handle).
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){ INIT;
	char cluster_buffer[CLUSTER_SIZE];
	int flag;
	struct t2fs_record *novacoisa = malloc(sizeof(struct t2fs_record));
	if (lista_arq_abertos[handle]->fileRecord->TypeVal == 0x02){
		flag = read_cluster(lista_arq_abertos[handle]->fileRecord->firstCluster,cluster_buffer);
		if ((!flag)&&(lista_arq_abertos[handle]->posFile<RECS_IN_DIR)){
			novacoisa = malloc(REC_TAM);
			memcpy(novacoisa,cluster_buffer+REC_TAM*lista_arq_abertos[handle]->posFile, REC_TAM);

			strcpy(dentry->name,novacoisa->name);
			dentry->fileType = novacoisa->TypeVal;
			dentry->fileSize = novacoisa->bytesFileSize;

			lista_arq_abertos[handle]->posFile++;
			return 0;
		}
	}

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a leitura das entradas do diret�rio identificado por "handle".
		A cada chamada da fun��o � lida a entrada seguinte do diret�rio representado pelo identificador "handle".
		Algumas das informa��es dessas entradas devem ser colocadas no par�metro "dentry".
		Ap�s realizada a leitura de uma entrada, o ponteiro de entradas (current entry) � ajustado para a pr�xima entrada v�lida
		S�o considerados erros:
			(a) t�rmino das entradas v�lidas do diret�rio identificado por "handle".
			(b) qualquer situa��o que impe�a a realiza��o da opera��o

	Entra:	handle -> identificador do diret�rio cujas entradas deseja-se ler.
		dentry -> estrutura de dados onde a fun��o coloca as informa��es da entrada lida.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor negativo
			Se o diret�rio chegou ao final, retorna "-END_OF_DIR" (-1)
			Outros erros, ser� retornado um outro valor negativo


		#define	END_OF_DIR	1
	-----------------------------------------------------------------------------*/
}

int closedir2 (DIR2 handle){ INIT;
	if (lista_dir_abertos[handle]->fileRecord->TypeVal == 0x02){
		free(lista_dir_abertos[handle]->fileRecord);
		lista_dir_abertos[handle] = NULL;
		handlerNarutoCount--;
		return 0;
	}

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o diret�rio identificado pelo par�metro "handle".

	Entra:	handle -> identificador do diret�rio que se deseja fechar (encerrar a opera��o).

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int main(int argc, char const *argv[]) {
	/* temp main for testin */
	INIT;

	int i;


	i = open2("/file1.txt");

	printf("File Code: %d\n",i);
	printf("Filename of new file is: %s\n", lista_arq_abertos[i]->fileRecord->name);
	printf("File Size: %u\n\n", lista_arq_abertos[i]->fileRecord->bytesFileSize);
	write2(i,"nigger",6);
	printf("File Size: %u\n\n", lista_arq_abertos[i]->fileRecord->bytesFileSize);
	char *buffer = malloc(61);
	read2(i,buffer,61);
	printf("SHIT I READ: %s\n", buffer);

	/*
	char *name = malloc(128);
	int i, j = 0;

	printf("Id:");
	for (i = 0; i < 4; i++){
		printf("%c",(unsigned char) superbloco->id[i]);
	}
	printf("\n");
	printf("Version: %x\n",(unsigned short int) superbloco->version);
	printf("Size of Superblock: %x\n",(unsigned short int) superbloco->SuperBlockSize);
	printf("Disk Size: %u\n",(unsigned short int) superbloco->DiskSize);
	printf("Number of Sectors: %u\n",(unsigned short int) superbloco->NofSectors);
	printf("Sectors Per Cluster: %u\n",(unsigned short int) superbloco->SectorsPerCluster);
	printf("FAT Start Sector: %u\n",(unsigned short int) superbloco->pFATSectorStart);
	printf("Root Directory Start Cluster: %u\n",(unsigned short int) superbloco->RootDirCluster);
	printf("Data Start Sector: %u\n",(unsigned short int) superbloco->DataSectorStart);

	printf("\nOpen2 Tests:\n");
	i = open2("/dir1/dir2/dir3/dir4/dir5/file.txt");
	printf("File Code: %d\n\n",i);

	i = open2("/file1.txt");
	printf("File Code: %d\n",i);
	printf("Filename of new file is: %s\n\n", lista_arq_abertos[i]->fileRecord->name);


	i = open2("/file2.txt");
	printf("File Code: %d\n",i);
	printf("Filename of new file is: %s\n\n", lista_arq_abertos[i]->fileRecord->name);
	close2(i);

	i = open2("/file2.txt");
	printf("File Code: %d\n",i);
	printf("Filename of new file is: %s\n\n", lista_arq_abertos[i]->fileRecord->name);

 	i = open2("/file1.txt");
	printf("File Code: %d\n\n",i);*/
	/*
	struct t2fs_record* testrec1 = malloc(sizeof(struct t2fs_record));
	struct t2fs_record* testrec2 = malloc(sizeof(struct t2fs_record));
	getFileRecord(rootDir, "file1.txt", testrec1);
	getFileRecord(rootDir, "file2.txt", testrec2);
	printf("Teste 1 Name: %s\n", testrec1->name);
	printf("Teste 1 Name: %s\n", testrec2->name);
	*/

	/*char *s = malloc(256);

	print_dir(currentDir);

	//chdir2("./root");
	getcwd2(s, 256);
	printf("aaa:%s\n", s);
	//chdir2("../dir1");
	chdir2("./dir1");
	print_dir(currentDir);
<<<<<<< HEAD
	getcwd2(s, 256);
	printf("aaa:%s\n", s);
	printf("\n");*/

	/*
	i = opendir2("/dir1");
	printf("Dir val of new Dir is: %x\n",lista_dir_abertos[i]->fileRecord->TypeVal);
	printf("Dir name of new Dir is: %s\n",lista_dir_abertos[i]->fileRecord->name);
	printf("Dir size of new Dir is: %u\n",lista_dir_abertos[i]->fileRecord->bytesFileSize);
	printf("Dir first cluster of new Dir is: %u\n",lista_dir_abertos[i]->fileRecord->firstCluster);
	printf("Dir Code: %d\n\n",i);
	closedir2(0);
	printf("Dir val of new Dir is: %x\n",lista_dir_abertos[i]->fileRecord->TypeVal);
	printf("Dir name of new Dir is: %s\n",lista_dir_abertos[i]->fileRecord->name);
	printf("Dir size of new Dir is: %u\n",lista_dir_abertos[i]->fileRecord->bytesFileSize);
	printf("Dir first cluster of new Dir is: %u\n",lista_dir_abertos[i]->fileRecord->firstCluster);
	printf("Dir Code: %d\n\n",i);
	*/

	/*
	i = opendir2("./");
	DIRENT2 *dentry;
	lista_dir_abertos[i]->posFile = 0;
	readdir2(i, dentry);

	printf("Name of nigga: %s\n", dentry->name);


	DIR2 dir = opendir2("dir1");
	Handler *handler = lista_arq_abertos[dir];
	printf("%d\n",(handler->posFile) );


	mark_free(0);
	printf("%x\n", get_next_cluster(0) );

	mkdir2("./dir");
	print_dir(currentDir);
	chdir2("./dir1");
	print_dir(currentDir);
	*/

	/*mkdir2("dir3");
	print_dir(currentDir);
	chdir2("./dir3");
	print_dir(currentDir);*/

	return 0;
}

//END OF FILE
