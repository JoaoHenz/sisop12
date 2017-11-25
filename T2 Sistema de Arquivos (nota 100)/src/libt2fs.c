#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "t2fs.h"
#include "funcExtras.h"
#include "apidisk.h"
#define SECTOR_SIZE 256
#define BYTES_ON_EACH_RECORD sizeof(struct t2fs_record)
#define ERRO -1 
#define ENDOFFILE 0

struct t2fs_superbloco *superbloco;
struct t2fs_record *DirAtual;
int BlocoDirAtual, OffsetDirAtual;
struct t2fs_record *bitmap;

typedef struct listaHandles{
	int fileHandle;	//handle do arquivo
	int posCursor;		//current position do arquivo
	int bloco;		//bloco onde o record do arquivo se encontra
	int offset;		//posição onde encontra-se o record no bloco acima
	struct listaHandles *next;	//proximo
} HandleList;

char nomeSubDir[39];
HandleList *listaHandles = NULL;
int rootSize = -1, fileEntrySize = -1, diskSize = -1, fileHandlerGenerator = 0, SuperblocoLido = 0;

//-----------------------------------------------------------------------------------//
//close
int t2fs_close (t2fs_file handle);
//open
t2fs_file t2fs_open (char *nome);
int ReadDiretorio(char *nome);
char* DivideString(char* pathname);
int findBlock(int blocoV,struct t2fs_record *record);
int FindNamePos(int bloco, char *nome, struct t2fs_record *record);
void trocaDirAtual(int bloco, int offset);
int getBlockSize();
int LeSuperbloco();
int read_block (unsigned int bloco, char *buffer);
void putHandleOnList(int handle, int bloco, int offset);
//read
int t2fs_read (t2fs_file handle, char *buffer, int size);
void AtualizaOffset(int deslocamento, int size);
int leArquivo(int blocoInicial, int offset, char *buffer, int size, struct t2fs_record *record);
int GetCurrentBloco(t2fs_file handle, struct t2fs_record *record);
int GetOffsetOnBloco(t2fs_file handle, struct t2fs_record *record);
struct t2fs_record *LoadRecord(int handle,  struct t2fs_record *record);
//write
int t2fs_write (t2fs_file handle, char *buffer, int size);
int escreveArquivo(int blocoInicial, int offset, char *buffer, int size, struct t2fs_record *record);
int addBloco(struct t2fs_record *record);
int alocaBlocoIndiceInd(struct t2fs_record *record, int bloco);
int alocaBlocoIndiceDupInd(struct t2fs_record *record, int bloco);
int searchBlocoLivre();
int carregaBitMap();
int write_block (unsigned int bloco, char *buffer);
//seek
int t2fs_seek (t2fs_file handle, unsigned int offset);
//delete
int t2fs_delete (char *nome);
int deleteFile(char *nome);
int deletaBlocoDados(int bloco, int offset);
int removeBitMap(int blocoR);
int removeRecord(int bloco, int offset);
//create
t2fs_file t2fs_create (char *nome);
t2fs_file createFile (char *nome);
int limpaArquivo(struct t2fs_record *record);
t2fs_file criaBlocoDados(int bloco, int offset);
int criaDiretorio(int bloco, int offset);
//funções extras
int findDiretorio(char *nome);
int searchDiretorio(char *nome);
int createDiretorio(char *nome);
int criaDirFile(char *nome);
int RemoveDiretorio(char *nome);
int DeleteDirFile(char *nome);
int isDirFree(struct t2fs_record *record);
int PrintaDiretorio(char *nome);
int PrintConteudoDir(char *nome);
int PrintFunc(struct t2fs_record *record);
//identify
char *t2fs_identify (void);

//-----------------------------------------------------------------------------------//
//--------------------------------Função IDENTIFY------------------------------------//
//-----------------------------------------------------------------------------------//
char *t2fs_identify(void){
	char *identification = (char*) malloc(94);
	identification = "Vinicius Amaro Cechin 159984\nGiuseppe Moroni Ramella 208781\nDouglas Cavalli Rachevsky 205686\n"; //94 caracteres com o '\0' incluso
	//printf("%d\n",strlen(identification)+1);
	return identification;
}
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Funções Extras------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
int createDiretorio(char *nome)
{
	LeSuperbloco();
	carregaBitMap();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),sizeof(struct t2fs_record));
	
	int retorno = criaDirFile(nome);
	
	if(retorno == -1)
	{
		free(DirAtual);
		DirAtual = NULL;
		free(bitmap);
		bitmap = NULL;
		free(superbloco);
		superbloco = NULL;
		SuperblocoLido = 0;
		return -1;

	}
	memcpy(&(superbloco->BitMapReg), bitmap, sizeof(struct t2fs_record));
	write_sector(0,(char *)superbloco);
	
	free(DirAtual);
	DirAtual = NULL;
	free(bitmap);
	bitmap = NULL;
	free(superbloco);
	superbloco = NULL;
	SuperblocoLido = 0;
	return 0;
}
//-----------------------------------------------------------------------------------//
int criaDirFile(char *nome)
{
	char *name = malloc(39);
	strcpy(name,nome);
	//printf("string: %s,  original: %s\n", name, nome);
	int blocoSize = getBlockSize();
	char* temp = malloc(blocoSize);
	char* buffer = malloc(blocoSize);
	char* resto;
	int i = 0, j, bloco, offset = -1, foundFreeRecord = 0, freeRecordOffset, freeRecordBlock, handle = -1;
	
	resto = DivideString(name);

	//printf("resto: %s, nomeSubDir %s", resto, nomeSubDir);
	int numBlocos = DirAtual->blocksFileSize;
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));

	if(*nomeSubDir == '\0')
	{
		return -1;	
	}	

	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		if(!foundFreeRecord)
		{
			read_block(bloco, temp);
			for(j=0;j<blocoSize;j+=sizeof(struct t2fs_record))
			{
				memcpy(record, temp+j, sizeof(struct t2fs_record));
				if(record->TypeVal == 0x00)
				{
					freeRecordOffset = j;
					freeRecordBlock = bloco;
					foundFreeRecord = 1;
					break;
				}
			}
		}
		//printf("record - bloco %d, offset %d",freeRecordBlock, freeRecordOffset);
		//printf("dir: [%s]\n", nomeSubDir);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return criaDirFile(resto);
			}
			
			free(buffer);
			buffer = NULL;
			free(temp);
			temp = NULL;
			free(record);
			record = NULL;
			free(name);
			name = NULL;
			return ERRO;			
		}
		i++;
	}
	if(!foundFreeRecord)
	{
		//Não existe record livre nos blocos, aumentar numero de blocos
		int livre = addBloco(DirAtual);
		if(livre == -1)
		{
			free(buffer);
			buffer = NULL;
			free(temp);
			temp = NULL;
			free(record);
			record = NULL;
			free(name);
			name = NULL;
			return -1;
		}
		return createFile(nome);
	}
	
	if(*resto!='\0')
	{
		free(buffer);
		buffer = NULL;
		free(temp);
		temp = NULL;
		free(record);
		record = NULL;
		free(name);
		name = NULL;
		return -1;
	}

	//printf("\nCBD DirAtual = %s\n", DirAtual->name);
	handle = criaDiretorio(freeRecordBlock, freeRecordOffset);
	if(handle == -1)
	{
		free(buffer);
		buffer = NULL;
		free(temp);
		temp = NULL;
		free(record);
		record = NULL;
		free(name);
		name = NULL;
		return -1;
	}

	DirAtual->bytesFileSize +=64;
	if(DirAtual->name[0] == '/')
	{
		//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
		memcpy(&(superbloco->RootDirReg), DirAtual, sizeof(struct t2fs_record));
	}
	else
	{
		//se não -> DirAtual é colocado no BlocoDirAtual na posição OffsetDirAtual
		read_block(BlocoDirAtual,buffer);
		memcpy(buffer+OffsetDirAtual, DirAtual, sizeof(struct t2fs_record));
		write_block(BlocoDirAtual,buffer);
	}	

	free(buffer);
	buffer = NULL;
	free(temp);
	temp = NULL;
	free(record);
	record = NULL;
	free(name);
	name = NULL;	
	return handle;
}
//-----------------------------------------------------------------------------------//
int findDiretorio(char *nome)
{
	LeSuperbloco();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),BYTES_ON_EACH_RECORD);
	int saida = searchDiretorio(nome);
	free(DirAtual);
	DirAtual = NULL;
	return saida;
	//return 0;
}
//-----------------------------------------------------------------------------------//
//Função que procura o Diretorio passado por nome, se existe retorna 0, senão -1
int searchDiretorio(char *nome)
{
	char* resto;
	int i = 0, bloco, offset = -1;
	resto = DivideString(nome);
	//printf("resto: %s\n", resto);
	int numBlocos = DirAtual->blocksFileSize;
	//printf("nb=%d,bfs=%d\n", numBlocos, DirAtual->blocksFileSize);

	if(*nomeSubDir == '\0')
	{
		return 0;	
	}	

	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		//printf("dir: [%s]\n", nomeSubDir);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return ReadDiretorio(resto);
			}
			return 0;			
		}
		i++;
	}

	return -1;
}
//-----------------------------------------------------------------------------------//
int RemoveDiretorio(char *nome)
{
	LeSuperbloco();
	carregaBitMap();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),sizeof(struct t2fs_record));
	
	int retorno = DeleteDirFile(nome);

	if(retorno == -1)
	{
		free(DirAtual);
		DirAtual = NULL;
		free(bitmap);
		bitmap = NULL;
		free(superbloco);
		superbloco = NULL;
		SuperblocoLido = 0;
		return -1;
	}

	free(DirAtual);
	DirAtual = NULL;
	free(bitmap);
	bitmap = NULL;
	free(superbloco);
	superbloco = NULL;
	SuperblocoLido = 0;
	return 0;
}
//-----------------------------------------------------------------------------------//
int DeleteDirFile(char *nome)
{
	char* resto;
	int i = 0, bloco, offset = -1;
	resto = DivideString(nome);
	int numBlocos = DirAtual->blocksFileSize;
	struct t2fs_record *record;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	
	if(*nomeSubDir == '\0')
	{
		return -1;	
	}	

	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return DeleteDirFile(resto);
			}

			read_block(bloco, temp);
			if(*(temp+offset) != 0x02)
			{
				return -1;	
			}
			
			//verifica se o diretorio está vazio
			
			record = (struct t2fs_record*)(temp+offset);
			//printf("Record Name: %s\n", record->name);
			int livre = isDirFree(record);
			//printf("livre: %d\n", livre);
			if(livre == -1)
			{
				return -1;
			}
			
			//deleta arquivo

			//printf("Bloco: %d; Offset: %d\n", bloco, offset);
			int retorno = deletaBlocoDados(bloco, offset);

			if(retorno == -1)
			{
				free(temp);
				temp = NULL;
				return -1;
			}

			DirAtual->bytesFileSize -=64;
			if(DirAtual->name[0] == '/')
			{
				//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
				memcpy(&(superbloco->RootDirReg), DirAtual, sizeof(struct t2fs_record));
			}
			else
			{
				//se não -> DirAtual é colocado no BlocoDirAtual na posição OffsetDirAtual
				read_block(BlocoDirAtual,temp);
				memcpy(temp+OffsetDirAtual, DirAtual, sizeof(struct t2fs_record));
				write_block(BlocoDirAtual,temp);
			}
			free(temp);
			temp = NULL;
			return 0;		
		}
		i++;
	}
	//printf("chega aqui? ret -1\n");
	free(temp);
	temp = NULL;
	return -1;
}
//-----------------------------------------------------------------------------------//
int isDirFree(struct t2fs_record *record)
{
	int i, j, numBlocos, bloco;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	//struct t2fs_record *blocoAtual;
	
	//printf("tamanho: %d\n", record->blocksFileSize);
	numBlocos = record->blocksFileSize;
	for(i=0;i<numBlocos;i++)
	{
		bloco = findBlock(i,record);
		read_block(bloco, temp);
		for(j=0; j<blocoSize; j+=64)
		{
			//blocoAtual = temp+j;
			//printf("%x\n, ", *(temp+j));
			if((*(temp+j) == 0x01) || (*(temp+j) == 0x02))
			{
				free(temp);
				temp = NULL;
				printf("Diretório com arquivos dentro...\n");
				return -1;
			}
		}
	}
	free(temp);
	temp = NULL;
	return 0;
}
//-----------------------------------------------------------------------------------//
int PrintaDiretorio(char *nome)
{
	LeSuperbloco();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),BYTES_ON_EACH_RECORD);
	
	int handle = PrintConteudoDir(nome);
	free(DirAtual);
	DirAtual = NULL;
	return handle;
	//return 0;
}
//-----------------------------------------------------------------------------------//
int PrintConteudoDir(char *nome)
{
	char* resto;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	int i = 0, bloco, offset = -1;
	resto = DivideString(nome);
	int numBlocos = DirAtual->blocksFileSize;
	struct t2fs_record *record;

	if(*nomeSubDir == '\0')
	{
		int printado = PrintFunc(DirAtual);
		if(printado == -1)
		{
			free(temp);
			temp = NULL;
			return -1;
		}
		free(temp);
		temp = NULL;
		return 0;	
	}	

	//printf("OutNumBlocos: %d\n", numBlocos);
	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		printf("dir: [%s]\n", nomeSubDir);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);

		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return ReadDiretorio(resto);
			}
			read_block(bloco, temp);
			//printf("valid byte: %x\n", *(temp+offset));
			if(*(temp+offset) == 0x02)
			{
				record = (struct t2fs_record*)(temp+offset);
				int printado = PrintFunc(record);
				if(printado == -1)
				{
					free(temp);
					temp = NULL;
					return -1;
				}
				free(temp);
				temp = NULL;
				return 0;	
			}
			free(temp);
			temp = NULL;
			return -1;			
		}
		i++;
	}

	//printf("chega aqui\n");
	free(temp);
	temp = NULL;
	return -1;
}

int PrintFunc(struct t2fs_record *record)
{
	int i, j, numBlocos, bloco;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	struct t2fs_record *tempRecord; 

	numBlocos = record->blocksFileSize;
	//printf("NumBlocos %d\n", numBlocos);
	for(i=0;i<numBlocos;i++)
	{
		bloco = findBlock(i,record);
		read_block(bloco, temp);
		for(j=0; j<blocoSize; j+=64)
		{
			tempRecord = (struct t2fs_record*)(temp+j);
			switch(tempRecord->TypeVal)
			{
				case 0x01:
				case 0x02:
					printf("%-20s %c     %-6d  %10d bytes\n", tempRecord->name, (tempRecord->TypeVal==0x01?'r':'d'), tempRecord->blocksFileSize, tempRecord->bytesFileSize);
					break;
				default:
					break;
			}
		}
	}
	free(temp);
	temp = NULL;
	return 0;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Função CREATE-------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
t2fs_file t2fs_create (char *nome)
{
	if(!(fileHandlerGenerator < 20))
	{
		//não pode abrir mais arquivos
		return -1;
	}
	LeSuperbloco();
	carregaBitMap();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),sizeof(struct t2fs_record));
	int retorno = createFile(nome);
	if(retorno == -1)
	{
		free(DirAtual);
		DirAtual = NULL;
		//free(bitmap);
		//bitmap = NULL;
		//free(superbloco);
		//superbloco = NULL;
		//SuperblocoLido = 0;
		return -1;
	}
	memcpy(&(superbloco->BitMapReg), bitmap, sizeof(struct t2fs_record));
	write_sector(0,(char *)superbloco);
	
	free(DirAtual);
	DirAtual = NULL;
	//free(bitmap);
	//bitmap = NULL;
	//free(superbloco);
	//superbloco = NULL;
	//SuperblocoLido = 0;
	return 0;
}
//-----------------------------------------------------------------------------------//
t2fs_file createFile (char *nome)
{
	char *name = malloc(39);
	strcpy(name,nome);
	//printf("string: %s,  original: %s\n", name, nome);
	int blocoSize = getBlockSize();
	char* temp = malloc(blocoSize);
	char* buffer = malloc(blocoSize);
	char* resto;
	int i = 0, j, bloco, offset = -1, foundFreeRecord = 0, freeRecordOffset, freeRecordBlock, flag = -1, handle = -1;
	resto = DivideString(name);
	//printf("resto: %s, nomeSubDir %s", resto, nomeSubDir);
	int numBlocos = DirAtual->blocksFileSize;
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));

	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		if(!foundFreeRecord)
		{
			read_block(bloco, temp);
			for(j=0;j<blocoSize;j+=sizeof(struct t2fs_record))
			{
				memcpy(record, temp+j, sizeof(struct t2fs_record));
				if(record->TypeVal == 0x00)
				{
					freeRecordOffset = j;
					freeRecordBlock = bloco;
					foundFreeRecord = 1;
					break;
				}
			}
		}
		//printf("record - bloco %d, offset %d",freeRecordBlock, freeRecordOffset);
		//printf("dir: [%s]\n", nomeSubDir);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return createFile(resto);
			}

			//printf("valid byte: %x\n", *(temp+offset));
			read_block(bloco, temp);
			if(*(temp+offset) != 0x01)
			{
				free(buffer);
				buffer = NULL;
				free(temp);
				temp = NULL;
				free(record);
				record = NULL;
				free(name);
				name = NULL;
				return -1;	
			}
			//cria handle e limpa bloco de dados
			memcpy(record, temp+offset, sizeof(struct t2fs_record));
			limpaArquivo(record);
			removeRecord(bloco, offset);
			handle = criaBlocoDados(bloco, offset);
			
			free(buffer);
			buffer = NULL;
			free(temp);
			temp = NULL;
			free(record);
			record = NULL;
			free(name);
			name = NULL;
			return handle;			
		}
		i++;
	}
	if(!foundFreeRecord)
	{
		//Não existe record livre nos blocos, aumentar numero de blocos
		int livre = addBloco(DirAtual);
		if(livre == -1)
		{
			free(buffer);
			buffer = NULL;
			free(temp);
			temp = NULL;
			free(record);
			record = NULL;
			free(name);
			name = NULL;
			return -1;
		}
		return createFile(nome);
	}
	if(*resto!='\0')
	{
		//printf("\nCDIR DirAtual = %s\n", DirAtual->name);
		flag = criaDiretorio(freeRecordBlock,freeRecordOffset);
		if(flag == -1)
		{
			free(buffer);
			buffer = NULL;
			free(temp);
			temp = NULL;
			free(record);
			record = NULL;
			free(name);
			name = NULL;
			return -1;
		}

		DirAtual->bytesFileSize +=64;
		//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
		if(DirAtual->name[0] == '/')
		{
			//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
			memcpy(&(superbloco->RootDirReg), DirAtual, sizeof(struct t2fs_record));
		}
		else
		{
			//se não -> DirAtual é colocado no BlocoDirAtual na posição OffsetDirAtual
			read_block(BlocoDirAtual,buffer);
			memcpy(buffer+OffsetDirAtual, DirAtual, sizeof(struct t2fs_record));
			write_block(BlocoDirAtual,buffer);
		}	

		trocaDirAtual(freeRecordBlock, freeRecordOffset);
		return createFile(resto);
	}
	//printf("\nCBD DirAtual = %s\n", DirAtual->name);
	handle = criaBlocoDados(freeRecordBlock, freeRecordOffset);

	DirAtual->bytesFileSize +=64;
	if(DirAtual->name[0] == '/')
	{
		//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
		memcpy(&(superbloco->RootDirReg), DirAtual, sizeof(struct t2fs_record));
	}
	else
	{
		//se não -> DirAtual é colocado no BlocoDirAtual na posição OffsetDirAtual
		read_block(BlocoDirAtual,buffer);
		memcpy(buffer+OffsetDirAtual, DirAtual, sizeof(struct t2fs_record));
		write_block(BlocoDirAtual,buffer);
	}	

	free(buffer);
	buffer = NULL;
	free(temp);
	temp = NULL;
	free(record);
	record = NULL;
	free(name);
	name = NULL;	
	return handle;
}
//-----------------------------------------------------------------------------------//
int limpaArquivo(struct t2fs_record *record)
{
	int NumBlocos = record->blocksFileSize;
	int i;
	int blocoSize = getBlockSize();	
	char* temp = malloc(blocoSize);

	for(i=0;i<NumBlocos;i++)
	{
		//printf("chega aqui\n");
		int blocoR = findBlock(i,record);
		if(blocoR == -1)
		{
			free(temp);
			temp = NULL;
			return -1;
		}
		memset(temp, 0x00, blocoSize);
		write_block(blocoR,temp);
		removeBitMap(blocoR);
	}
	
	record->bytesFileSize = 0;
	free(temp);
	temp = NULL;
	return 0;
}
//-----------------------------------------------------------------------------------//
t2fs_file criaBlocoDados(int bloco, int offset)
{
	int blocoDados;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	
	//printf("chega aqui\n");
	read_block(bloco, temp);

	blocoDados = searchBlocoLivre();
	if(blocoDados == -1)
	{
		//não há bloco livre;
		free(temp);
		temp = NULL;
		free(record);
		record = NULL;
		return -1;
	}
	memset(record, 0x00, sizeof(struct t2fs_record));
	record->dataPtr[0] = blocoDados;
	record->dataPtr[1] = 0xffffffff;
	record->singleIndPtr = 0xffffffff;
	record->doubleIndPtr = 0xffffffff;
	record->blocksFileSize = 1;
	record->bytesFileSize = 0;
	record->TypeVal = 0x01;
	strcpy(record->name,nomeSubDir);
	memcpy(temp+offset, record, sizeof(struct t2fs_record));
	write_block(bloco, temp);
	
	read_block(blocoDados, temp);
	memset(temp, 0x00, blocoSize);
	write_block(blocoDados, temp);

	putHandleOnList(fileHandlerGenerator, bloco, offset);
	fileHandlerGenerator++;
	free(temp);
	temp = NULL;
	free(record);
	record = NULL;
	return fileHandlerGenerator-1;
}
//-----------------------------------------------------------------------------------//
int criaDiretorio(int bloco, int offset)
{
	int blocoDir;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	
	read_block(bloco, temp);
	memcpy(record, temp+offset, sizeof(struct t2fs_record));
	blocoDir = searchBlocoLivre();
	if(blocoDir == -1)
	{
		//não há bloco livre;
		free(temp);
		temp = NULL;
		free(record);
		record = NULL;
		return -1;
	}
	memset(record, 0x00, sizeof(struct t2fs_record));
	record->dataPtr[0] = blocoDir;
	record->dataPtr[1] = 0xffffffff;
	record->singleIndPtr = 0xffffffff;
	record->doubleIndPtr = 0xffffffff;
	record->blocksFileSize = 1;
	record->bytesFileSize = 0;
	record->TypeVal = 0x02;
	strcpy(record->name,nomeSubDir);
	memcpy(temp+offset, record, sizeof(struct t2fs_record));
	write_block(bloco, temp);
	
	read_block(blocoDir, temp);
	memset(temp, 0x00, blocoSize);
	write_block(blocoDir, temp);

	free(temp);
	temp = NULL;
	free(record);
	record = NULL;
	return 0;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Função DELETE-------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
int t2fs_delete (char *nome)
{
	LeSuperbloco();
	carregaBitMap();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),sizeof(struct t2fs_record));
	int retorno = deleteFile(nome);
	if(retorno == -1)
	{
		return -1;
	}
	free(DirAtual);
	DirAtual = NULL;
	free(bitmap);
	bitmap = NULL;
	free(superbloco);
	superbloco = NULL;
	SuperblocoLido = 0;
	return 0;
}
//-----------------------------------------------------------------------------------//
int deleteFile(char *nome)
{
	char* resto;
	//printf("chega aqui?\n");
	int i = 0, bloco, offset = -1;
	resto = DivideString(nome);
	int numBlocos = DirAtual->blocksFileSize;	
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);

	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return deleteFile(resto);
			}

			read_block(bloco, temp);
			if(*(temp+offset) != 0x01)
			{
				return -1;	
			}

			//deleta arquivo
			//printf("Bloco: %d; Offset: %d\n", bloco, offset);
			int retorno = deletaBlocoDados(bloco, offset);

			if(retorno == -1)
			{
				return -1;
			}

			DirAtual->bytesFileSize -=64;
			if(DirAtual->name[0] == '/')
			{
				//se DirAtual.nome = "/" -> DirAtual é colocado no superbloco
				memcpy(&(superbloco->RootDirReg), DirAtual, sizeof(struct t2fs_record));
			}
			else
			{
				//se não -> DirAtual é colocado no BlocoDirAtual na posição OffsetDirAtual
				read_block(BlocoDirAtual,temp);
				memcpy(temp+OffsetDirAtual, DirAtual, sizeof(struct t2fs_record));
				write_block(BlocoDirAtual,temp);
			}
			free(temp);
			temp = NULL;
			return 0;			
		}
		i++;
	}
	//printf("chega aqui? ret -1\n");
	free(temp);
	temp = NULL;
	return -1;
}
//-----------------------------------------------------------------------------------//
int deletaBlocoDados(int bloco, int offset)
{
	int i=0;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	
	read_block(bloco, temp);
	memcpy(record, temp+offset, sizeof(struct t2fs_record));
	int numBlocos = record->blocksFileSize;

	while(i<numBlocos)
	{
		//printf("chega aqui\n");
		int blocoR = findBlock(i,record);
		if(blocoR == -1)
		{
			return -1;
		}
		read_block(blocoR,temp);
		memset(temp, 0x00, blocoSize);
		write_block(blocoR,temp);
		removeBitMap(blocoR);
		i++;
	}
	removeRecord(bloco, offset);

	free(record);
	record = NULL;
	free(temp);
	temp = NULL;
	return 0;
}
//-----------------------------------------------------------------------------------//
int removeBitMap(int blocoR)
{
	int i;
	int blocoSize = getBlockSize();
	char *buffer = malloc(blocoSize);
	read_block(bitmap->dataPtr[0],buffer);
	int cont = 0;
	char *p = buffer;
	int byteToChange = blocoR/8;
	int bitToChange = blocoR%8;
	bitToChange = pow(2,bitToChange);
	//printf("conteudo p: %x\n", (BYTE)*p);
	for(i=0;i<byteToChange;i++)
	{
		p++;
		cont++;
	}
	BYTE B = *(p);
	//printf("conteudo p: %x\n", B);
	BYTE mascara = 0xff - bitToChange;
	//printf("mascara para p: %x\n", mascara);
	B = B & mascara;
	//printf("novo: %x\n", B);
	memcpy(p,&(B),sizeof(unsigned char));
	write_block(bitmap->dataPtr[0],buffer);			//descomentar para funcionar
	//printf("conteudo p: %x\n", (BYTE)*(p));

	free(buffer);
	buffer = NULL;
	return 0;
}
//-----------------------------------------------------------------------------------//
int removeRecord(int bloco, int offset)
{
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	
	read_block(bloco, temp);
	char *p = temp;
	int i;
	for(i=0; i<offset; i+=sizeof(struct t2fs_record))
	{
		p +=sizeof(struct t2fs_record);
	}
	//printf("\nrecord positions: %x\n", (BYTE)*(p+1));
	memset(p, 0x00, sizeof(struct t2fs_record));
	//printf("\nrecord positions: %x\n\n", (BYTE)*(p+1));
	write_block(bloco, temp);					//descomentar para funcionar
	free(temp);
	temp = NULL;
	return 0;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Função SEEK---------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
int t2fs_seek (t2fs_file handle, unsigned int offset)
{
	LeSuperbloco();
	carregaBitMap();
	struct t2fs_record *record = malloc(BYTES_ON_EACH_RECORD);
	HandleList *HandleTemp = listaHandles;
	record = LoadRecord(handle, record);

	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			//current pointer novo aponta para o fim do arquivo?
			if(offset == -1)
			{
				HandleTemp->posCursor = record->bytesFileSize;
				free(record);
				record = NULL;
				return 0;
			}
			//current pointer novo existe no arquivo?
			if(offset < record->bytesFileSize)
			{
				HandleTemp->posCursor = offset;
				free(record);
				record = NULL;
				return 0;
			}
			else
			{
				free(record);
				record = NULL;
				return -1;
			}
		}
		HandleTemp = HandleTemp->next;
	}
	free(record);
	record = NULL;
	return -1;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Função WRITE--------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
int t2fs_write (t2fs_file handle, char *buffer, int size)
{
	LeSuperbloco();
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	HandleList *HandleTemp = listaHandles;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	//bitmap = malloc(sizeof(struct t2fs_record));
	carregaBitMap();
	record = LoadRecord(handle, record);
	int currentBloco = GetCurrentBloco(handle, record);
	int currentOffset = GetOffsetOnBloco(handle, record);
	
	if((currentBloco==-1)||(currentOffset==-1)||(record==NULL))
	{
		//Não achou algum dado, handle com problemas
		free(temp);
		temp = NULL;
		free(record);
		record = NULL;
		return -1;
	}

	int BytesEscritos = escreveArquivo(currentBloco, currentOffset, buffer, size, record);

	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			//current pointer não aponta para o fim do arquivo?
			if(((HandleTemp->posCursor)+BytesEscritos) < record->bytesFileSize)
				HandleTemp->posCursor+=BytesEscritos;
			else
				HandleTemp->posCursor = record->bytesFileSize;

			read_block(HandleTemp->bloco, temp);
			memcpy( &(temp[HandleTemp->offset]), (char *)record, BYTES_ON_EACH_RECORD);
			write_block(HandleTemp->bloco, temp);
			free(temp);
			temp = NULL;
			break;
		}
		HandleTemp = HandleTemp->next;
	}
	
	memcpy(&(superbloco->BitMapReg), bitmap, sizeof(struct t2fs_record));
	free(temp);
	temp = NULL;
	free(record);
	record = NULL;
	return BytesEscritos;
}
//-----------------------------------------------------------------------------------//
//Escreve dados do buffer para o disco
int escreveArquivo(int blocoInicial, int offset, char *buffer, int size, struct t2fs_record *record)
{
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);

	int i = blocoInicial;
	int contPosBloco = offset;
	int cont = 0;
	int blocoR;

	int CP = (blocoInicial*blocoSize)+offset;
	int BlocosNaoCriados = 0;
	int BlocosToAdd = (CP+size)/blocoSize;
	if(BlocosToAdd >= record->blocksFileSize)
	{
		//escrita a mais que a capacidade do arquivo, adiciona novos blocos ao arquivo
		int i = BlocosToAdd;		
		while(i > 0)
		{
			int flag = addBloco(record);
			i--;
			if(flag == ERRO)
			{
				BlocosNaoCriados++;
			}
//			bitmap->bytesFileSize+= (BlocosToAdd - BlocosNaoCriados)*64;
		}	
	}

	int tamanhoFile = contPosBloco + size;
	if(tamanhoFile > record->bytesFileSize)
		record->bytesFileSize = tamanhoFile;
	if(contPosBloco+size < blocoSize)
	{
		blocoR = findBlock(i, record);
		read_block(blocoR, temp);

		memcpy(&(temp[contPosBloco]), buffer, size);		
		write_block (blocoR, temp);
		free(temp);
		temp=NULL;

		return size;
	}
	else
	{
		while(i < record->blocksFileSize)
		{
			blocoR = findBlock(i, record);
			read_block(blocoR, temp);
			if(size < blocoSize)
			{
				memcpy(temp+contPosBloco, buffer+cont, size);
				cont+=size;
				write_block(blocoR, temp);
				free(temp);
				temp=NULL;
				return cont;
			}
			memcpy(temp+contPosBloco, buffer+cont, blocoSize-contPosBloco);
			cont+=(blocoSize-contPosBloco);
			write_block(blocoR, temp);
			size-=(blocoSize-contPosBloco);
			contPosBloco = 0;	
			i++;
		}
		free(temp);
		temp=NULL;
		return cont;
	}
	free(temp);
	temp=NULL;
	return -1;
}
//-----------------------------------------------------------------------------------//
//Adiciona mais um bloco ao arquivo
int addBloco(struct t2fs_record *record)
{
	int bloco = searchBlocoLivre();
	//printf("bloco livre: %d\n", bloco);
	if(bloco == -1)
	{
		return -1;
	}
	record->blocksFileSize++;
	if(record->dataPtr[0] == 0xffffffff)
	{
		record->dataPtr[0] = bloco;
		return 0;
	}
	if(record->dataPtr[1] == 0xffffffff)
	{
		record->dataPtr[1] = bloco;
		return 0;
	}
	if(alocaBlocoIndiceInd(record, bloco) != -1)
	{
		return 0;
	}
	if(alocaBlocoIndiceDupInd(record, bloco) != -1)
	{
		return 0;
	}
	return -1;
}
//-----------------------------------------------------------------------------------//
//Aloca um bloco de Indices Indireto
int alocaBlocoIndiceInd(struct t2fs_record *record, int bloco)
{
	int blocoSize = getBlockSize();
	char *buffer = malloc(blocoSize);
	int cont = 0;
	char *p;
	if(record->singleIndPtr == 0xffffffff)
	{
		record->singleIndPtr = bloco;
		read_block(bloco, buffer);
		memset(buffer, 0xff, blocoSize);
		int bloco2 = searchBlocoLivre();
		//memcpy(buffer,(unsigned char *)bloco2,sizeof(DWORD));
		*((DWORD*)buffer)=bloco2;
		write_block(bloco, buffer);
		return 0;
	}
	else
	{
		read_block(record->singleIndPtr, buffer);
		p = buffer;
		while((*(DWORD *)p != 0xffffffff)&&(cont<blocoSize))
		{
			p += sizeof(DWORD);
			cont+=sizeof(DWORD);
			if(*(DWORD *)p == 0xffffffff)
			{
				*((DWORD*)buffer)=bloco;
				write_block(record->singleIndPtr, buffer);
				return 0;
			}
		}
	}
	return -1;
}
//-----------------------------------------------------------------------------------//
//Aloca um bloco de Indices Duplamente Indireto
int alocaBlocoIndiceDupInd(struct t2fs_record *record, int bloco)
{
	int blocoSize = getBlockSize();
	char *buffer = malloc(blocoSize);
	char *buffer2 = malloc(blocoSize);
	int cont = 0, cont2 = 0, bloco2 = -1, bloco3 = -1;
	char *p, *q;
	if(record->doubleIndPtr == 0xffffffff)
	{
		record->doubleIndPtr = bloco;
		
		read_block(bloco, buffer);
		memset(buffer, 0xff, blocoSize);
		bloco2 = searchBlocoLivre();
		*((DWORD*)buffer)=bloco2;
		write_block(bloco, buffer);
		
		read_block(bloco2, buffer);
		memset(buffer, 0xff, blocoSize);
		bloco3 = searchBlocoLivre();
		*((DWORD*)buffer)=bloco3;
		write_block(bloco2, buffer);
		
		return 0;
	}
	read_block(record->doubleIndPtr, buffer);
	p = buffer;
	while(( *((DWORD*)p) != 0xffffffff)&&(cont<blocoSize))
	{
		read_block(*((DWORD *)p),buffer2);
		cont2 = 0;
		q = buffer2;
		while(cont2<blocoSize)
		{
			if(*(DWORD *)q == 0xffffffff)
			{
				*((DWORD*)q)=bloco;
				write_block(*((DWORD *)p), buffer2);
				return 0;
			}
			q = q+sizeof(DWORD);
			cont2+=sizeof(DWORD);
		}
		p = p+sizeof(DWORD);
		cont+=sizeof(DWORD);
		if(*(DWORD *)p == 0xffffffff)
		{
			*((DWORD*)p)=bloco;
			write_block(record->doubleIndPtr, buffer);
			read_block(bloco,buffer);
			memset(buffer, 0xff, blocoSize);
			bloco2 = searchBlocoLivre();
			*((DWORD*)buffer)=bloco2;
			write_block(bloco, buffer);
	
			return 0;
		}
	}
	return -1;
}
//-----------------------------------------------------------------------------------//
//Procura no bitmap por um bloco livre e ja acrescenta-o no bitmap. Retorna o numero do bloco encontrado
int searchBlocoLivre()
{
	int blocoSize = getBlockSize();
	char *buffer = malloc(blocoSize);
	read_block(bitmap->dataPtr[0],buffer);
	int cont = 0;
	char *p = buffer;
	while(((unsigned int)*p == 0xffffffff) && (cont<blocoSize))
	{
		p++;
		cont++;
	}
	if(cont<blocoSize)
	{
		unsigned char B = *(p);
	//	printf("bitmap: %x\n",(unsigned int)B);
		int i, j, numero = 2;
		for(i=0;i<8;i++)
		{
			int logic = B & 1;
			if(logic == 0)
			{
				for(j=0;j<i;j++)
					numero = numero*2;
				if(i!=0)
					numero -= 1;
				B = (unsigned char) numero;
				memcpy(p,&(B),sizeof(unsigned char));
				write_block(bitmap->dataPtr[0],buffer);		//comentar p/ não escrever			
				free(buffer);
				buffer = NULL;
				return cont*8+i;
			}
			//printf("bit %d do byte lido: %d\n", i, logic);
			B>>=1;
		}

	}
	free(buffer);
	buffer = NULL;
	return -1;
}
//-----------------------------------------------------------------------------------//
//Carrega o bitmap na memória
int carregaBitMap()
{
	bitmap = malloc(sizeof(struct t2fs_record));
	memcpy(bitmap, &(superbloco->BitMapReg),sizeof(struct t2fs_record));
	return 0;
}
//-----------------------------------------------------------------------------------//
//Escreve de um buffer para um bloco do disco
int write_block (unsigned int bloco, char *buffer)
{
	write_sector(bloco*4+1, buffer);
	write_sector(bloco*4+2, buffer+256);
	write_sector(bloco*4+3, buffer+512);
	write_sector(bloco*4+4, buffer+768);
	return 0;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//---------------------------------Função READ---------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//

int t2fs_read (t2fs_file handle, char *buffer, int size)
{
	if(handle < 0 || handle > 20)
	{
		return -1;
	}
	LeSuperbloco();
	struct t2fs_record *record = malloc(sizeof(struct t2fs_record));
	HandleList *HandleTemp = listaHandles;
	
	int blocoSize = getBlockSize();
	record = LoadRecord(handle, record);
	int currentBloco = GetCurrentBloco(handle, record);
	int currentOffset = GetOffsetOnBloco(handle, record);
	
	if((currentBloco==-1)||(currentOffset==-1)||(record==NULL))
	{
		free(record);
		record = NULL;
		//Não achou algum dado, handle com problemas
		return ERRO;
	}

	if((currentBloco*blocoSize)+currentOffset >= record->bytesFileSize)
	{
		free(record);
		record = NULL;
		//offset apontando para o fim do arquivo
		return ENDOFFILE;
	}
	//printf("\noffset: %d", currentOffset);
	int BytesLidos = leArquivo(currentBloco, currentOffset, buffer, size, record);	
	//printf("depois: %s\n",buffer);	
	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			//current pointer aponta para o fim do arquivo?
			if(((HandleTemp->posCursor)+BytesLidos) < record->bytesFileSize)
				HandleTemp->posCursor+=BytesLidos;
			else
				HandleTemp->posCursor = record->bytesFileSize;
		}
		HandleTemp = HandleTemp->next;
	}

	free(record);
	record = NULL;
	return BytesLidos;
}
//-----------------------------------------------------------------------------------//
//Função que lê os dados de um bloco e retorna o numero de bytes lidos.
int leArquivo(int blocoInicial, int offset, char *buffer, int size, struct t2fs_record *record)
{
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);

	int i = blocoInicial;
	int contPosBloco = offset;
	int cont = 0;
	int blocoR;

	//printf("filesize: %d\n", record->bytesFileSize);
	if((blocoInicial*blocoSize)+offset+size > record->bytesFileSize)
	{
		size -= ((blocoInicial*blocoSize)+offset+size-(record->bytesFileSize));
	}

	if(contPosBloco+size < blocoSize)
	{
		blocoR = findBlock(i, record);
		read_block(blocoR, temp);
		//printf("\noffset: %d, ", contPosBloco);
		memcpy(buffer, &(temp[contPosBloco]), size);
		//printf("buffer: %s\n", buffer);
		free(temp);
		temp=NULL;
		return size;
	}
	else
	{
		while(i < record->blocksFileSize)
		{
			blocoR = findBlock(i, record);
			read_block(blocoR, temp);
			if(size < blocoSize)
			{
				memcpy(buffer+cont, temp+contPosBloco, size);
				cont+=size;
				free(temp);
				temp=NULL;
				return cont;
			}
			memcpy(buffer+cont, temp+contPosBloco, blocoSize-contPosBloco);
			cont+=(blocoSize-contPosBloco);
			size-=(blocoSize-contPosBloco);
			contPosBloco = 0;	
			i++;
		}
		free(temp);
		temp=NULL;
		return cont;
	}
}
//-----------------------------------------------------------------------------------//
//Função que retorna o bloco que deve ser lido/escrito na chamada atual de READ/WRITE
int GetCurrentBloco(t2fs_file handle, struct t2fs_record *record)
{
	HandleList *HandleTemp = listaHandles;
	int blocoSize, current, offsetInsideBloco;
	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			blocoSize = getBlockSize();
			current = HandleTemp->posCursor;
			offsetInsideBloco = current/blocoSize;
			return offsetInsideBloco;
		}
		HandleTemp = HandleTemp->next;
	}
	return -1;
}
//-----------------------------------------------------------------------------------//
//Função que retorna o offset do bloco que deve ser lido/escrito na chamada de READ/WRITE
int GetOffsetOnBloco(t2fs_file handle, struct t2fs_record *record)
{
	HandleList *HandleTemp = listaHandles;
	int blocoSize, current, offsetInsideBloco;
	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			blocoSize = getBlockSize();
			current = HandleTemp->posCursor;
			//printf("\ncurrent = %d\n",current);
			offsetInsideBloco = current%blocoSize;
			return offsetInsideBloco;
		}
		HandleTemp = HandleTemp->next;
	}
	return -1;
}
//-----------------------------------------------------------------------------------//
//Carrega o record indicado por handle na memória
struct t2fs_record *LoadRecord(int handle,  struct t2fs_record *record)
{
	int blocoSize = getBlockSize();	
	HandleList *HandleTemp = listaHandles;
	char *temp = malloc(blocoSize);

	while(HandleTemp != NULL)
	{
		if(HandleTemp->fileHandle == handle)
		{
			read_block(HandleTemp->bloco, temp);
			memcpy((char *)record, &(temp[HandleTemp->offset]), sizeof(struct t2fs_record));
			free(temp);
			temp = NULL;
			return record;
		}
		HandleTemp = HandleTemp->next;
	}
	return NULL;
}
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//--------------------------------funçao OPEN----------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
t2fs_file t2fs_open (char *nome)
{
	if(!(fileHandlerGenerator < 20))
	{
		//não pode abrir mais arquivos
		return -1;
	}
	LeSuperbloco();
	DirAtual = malloc(BYTES_ON_EACH_RECORD);
	memcpy(DirAtual, &(superbloco->RootDirReg),BYTES_ON_EACH_RECORD);
	
	int handle = ReadDiretorio(nome);
	free(DirAtual);
	DirAtual = NULL;
	return handle;
	//return 0;
}
//-----------------------------------------------------------------------------------//
//Função que abre o arquivo e gera um handle para o mesmo. Retorna o valor do handle
int ReadDiretorio(char *nome)
{
	char* resto;
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	int i = 0, bloco, offset = -1;
	resto = DivideString(nome);
	//printf("resto: %s\n", resto);
	int numBlocos = DirAtual->blocksFileSize;
	//printf("nb=%d,bfs=%d\n", numBlocos, DirAtual->blocksFileSize);
	while(i<numBlocos)
	{
		bloco = findBlock(i,DirAtual);
		//printf("dir: [%s]\n", nomeSubDir);
		offset = FindNamePos(bloco, nomeSubDir, DirAtual);
		if(offset != -1)
		{
			if(*resto!='\0')
			{
				//troca diretorio atual
				trocaDirAtual(bloco, offset);
				return ReadDiretorio(resto);
			}
			//cria handle
			//printf("chega aqui\n");
			read_block(bloco, temp);
			//printf("valid byte: %x\n", *(temp+offset));
			if(*(temp+offset) == 0x01)
			{
				putHandleOnList(fileHandlerGenerator, bloco, offset);
				fileHandlerGenerator++;
				free(temp);
				temp = NULL;
				return fileHandlerGenerator-1;	
			}
			free(temp);
			temp = NULL;
			return -1;			
		}
		i++;
	}

	printf("chega aqui\n");
	free(temp);
	temp = NULL;
	return -1;
}
//-----------------------------------------------------------------------------------//
//Função que divide o caminho do arquivo em partes (a cada barra encontrada)
//Salva a primeira parte em "nomeSubDir" e retorna o resto do caminho
char* DivideString(char* pathname)
{
	char *s;
	char *d;
	s = pathname;
//printf("%s\n",s);
	if(*s=='/')	s++;
	d = s;
//printf("%s, %s\n",s,d);
	while(*d != '/' && *d != '\0') {
		++d;
//		printf("%s, %s\n",s,d);
    }
//	printf("%s, %c\n",s,*d);
    if(*d != '\0')
	{
		*d = '\0';
		d++;
	}
	strcpy(nomeSubDir,s);
	return d;

}
//-----------------------------------------------------------------------------------//
//Entra um bloco virtual do arquivo (0 a tamnho em blocos do arquivo) e retorna um bloco real (endereço do ponteiro que aponta o bloco de dados correspondente)
int findBlock(int blocoV, struct t2fs_record *record)
{
	int blocoSize = getBlockSize();
	int pos, blocoR;
	DWORD *temp = malloc(blocoSize);
	DWORD *temp2 = malloc(blocoSize);

	if(blocoV < superbloco->NofBlocks)
	{
		if(blocoV==0)
		{
			return record->dataPtr[0];
		}
		else{
            if(blocoV==1)
            {
                return record->dataPtr[1];
            }
            else{
                if(blocoV>=3 && blocoV<=257)
                {
                    read_block(record->singleIndPtr, (char *)temp);
				blocoR = temp[blocoV-2];
				free(temp);
				temp = NULL;
                    return blocoR;
                }
                else
                {
                    read_block(record->doubleIndPtr, (char *)temp2);
                    pos = (blocoV-258)/256;
                    read_block(temp2[pos], (char *)temp);
                    pos = (blocoV-2)%256;
				blocoR = temp[pos];
				free(temp2);
				temp2 = NULL;
				free(temp);
				temp = NULL;
                    return blocoR;
                }
            }
		}
	}
	else
		return -1;
}
//-----------------------------------------------------------------------------------//
//Procura o record com o nome passado como parametro para a função
int FindNamePos(int bloco, char *nome, struct t2fs_record *record)
{
	int blocoSize = getBlockSize();
	int cont = 0;
	char *subrecord = malloc(BYTES_ON_EACH_RECORD);
	char nameFile[39], name[39];
	char *temp = malloc(blocoSize);

	memcpy(name, nome, 39);
	read_block(bloco, (char *)temp);
	
	for(cont = 0; cont < blocoSize; cont+=BYTES_ON_EACH_RECORD)
	{
		memcpy(subrecord, &(temp[cont]), BYTES_ON_EACH_RECORD);
		memcpy(nameFile, ((struct t2fs_record *)subrecord)->name, 39);
		//printf("%s\n", nameFile);
		//printf("%d\n", cont);
		if(strcmp(nameFile, name) == 0)
		{
			free(temp);
			temp = NULL;
			free(subrecord);
			subrecord = NULL;
			return cont; //achou
		}
	}
	free(temp);
	temp = NULL;
	free(subrecord);
	subrecord = NULL;
	return -1; //não achou
}
//-----------------------------------------------------------------------------------//
//Troca o record apontado pela variavél global "DirAtual"
void trocaDirAtual(int bloco, int offset)
{
	int blocoSize = getBlockSize();
	char *temp = malloc(blocoSize);
	//struct t2fs_record *NovoDir;
	
	read_block(bloco, (char *)temp);

	BlocoDirAtual = bloco;
	OffsetDirAtual = offset;
	//printf("\nBlocoDirAtual = %d, OffSetDirAtual = %d\n", BlocoDirAtual, OffsetDirAtual);
	
	memcpy(DirAtual, temp+offset, sizeof(struct t2fs_record));
	
	//printf("NovoDir:fn=%s\n", DirAtual->name);
	return;
}
//-----------------------------------------------------------------------------------//
//retorna o tamanho de um bloco
int getBlockSize(){
	LeSuperbloco();
	return superbloco->BlockSize;
}
//-----------------------------------------------------------------------------------//
//Lê o superbloco para a memória
int LeSuperbloco()
{
	if(!SuperblocoLido)
	{
		superbloco = (struct t2fs_superbloco *) malloc(SECTOR_SIZE);
		read_sector(0, (char *)superbloco);
		SuperblocoLido = 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------//
//Lê um bloco do disco para o buffer
int read_block (unsigned int bloco, char *buffer)
{
	read_sector(bloco*4+1, buffer);
	read_sector(bloco*4+2, buffer+256);
	read_sector(bloco*4+3, buffer+512);
	read_sector(bloco*4+4, buffer+768);
	return 0;
}
//-----------------------------------------------------------------------------------//
//Cria um handle na lista e inicia-o
void putHandleOnList(int handle, int bloco, int offset)
{
	HandleList *node = (HandleList*)malloc(sizeof(HandleList));
	node->fileHandle = handle;
	node->bloco = bloco;
	node->offset = offset;
	node->posCursor = 0;
	node->next = listaHandles;
	listaHandles = node;
}

//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//--------------------------------funçao CLOSE---------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//
int t2fs_close (t2fs_file handle)
{
	LeSuperbloco();
	HandleList *HandleTemp = listaHandles;
	HandleList *anterior = NULL;
	if(fileHandlerGenerator>0)
	{
		while(HandleTemp != NULL)
		{
			if(HandleTemp->fileHandle == handle)
			{
				if(anterior == NULL)
				{
					listaHandles = HandleTemp->next;
				}
				else
				{
					anterior->next = HandleTemp->next;
				}
				fileHandlerGenerator--;
				write_sector(0,(char *)superbloco);
				

				free(superbloco);
				superbloco = NULL;				
				if(bitmap != NULL)
				{
					free(bitmap);
					bitmap = NULL;
				}
				SuperblocoLido = 0;		
				return 0;
			}
			anterior = HandleTemp;
			HandleTemp = HandleTemp->next;
		}
	}
	return -1;	//Handle não encontrado
}
