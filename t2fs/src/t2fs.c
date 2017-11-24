#define	SECTOR_SIZE	256
#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02
#include <stdio.h>
#include <string.h>

typedef int FILE2;
typedef int DIR2;

typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)

/** Superbloco */
struct t2fs_superbloco {		// Tabela 1 � Descri��o dos campos do bloco de boot
	char    id[4];			// �T2FS�	Identifica��o do sistema de arquivo.
	WORD    version;		// Vers�o atual desse sistema de arquivos Valor fixo 0x7E12 (0x7E1=2017; 2=2� semestre)
	WORD	SuperBlockSize;		// Quantidade de setores l�gicos que formam o superbloco.
	DWORD	DiskSize;		// Tamanho total, em bytes, da parti��o T2FS. Inclui o superbloco, a �rea de FAT e os clusters de dados.
	DWORD	NofSectors;		// Quantidade total de setores l�gicos da parti��o T2FS. Inclui o superbloco, a �rea de FAT e os clusters de dados.
	DWORD	SectorsPerCluster;	// N�mero de setores l�gicos que formam um cluster.
	DWORD	pFATSectorStart;	// N�mero do setor l�gico onde a FAT inicia.
	DWORD	RootDirCluster;		// Cluster onde inicia o arquivo correspondente ao diret�rio raiz
	DWORD	DataSectorStart;	// Primeiro setor l�gico da �rea de blocos de dados (cluster).
};


/** Registro de diret�rio (entrada de diret�rio) */
#define	MAX_FILE_NAME_SIZE	55
struct t2fs_record {
    BYTE    TypeVal;        		/* TypeVal	Tipo da entrada. Indica se o registro � v�lido e, se for, o tipo do arquivo (regular ou diret�rio). */
					/*	0x00, registro inv�lido (n�o associado a nenhum arquivo);
						0x01, arquivo regular;
						0x02, arquivo de diret�rio.
						Outros valores, registro inv�lido (n�o associado a nenhum arquivo)
						*/
    char    name[MAX_FILE_NAME_SIZE]; 	/* Nome do arquivo. : string com caracteres ASCII (0x21 at� 0x7A), case sensitive.             */
    DWORD   bytesFileSize;  		/* Tamanho do arquivo. Expresso em n�mero de bytes.          */
    DWORD   firstCluster;		/* N�mero do primeiro cluster de dados correspondente a essa entrada de diret�rio */
};

#pragma pack(pop)

/** Registro com as informa��es da entrada de diret�rio, lida com readdir2 */
typedef struct {
    char    name[MAX_FILE_NAME_SIZE+1]; /* Nome do arquivo cuja entrada foi lida do disco      */
    BYTE    fileType;                   /* Tipo do arquivo: regular (0x01) ou diret�rio (0x02) */
    DWORD   fileSize;                   /* Numero de bytes do arquivo                          */
} DIRENT2;

t2fs_superbloco superbloco;
int initialized = 0;
char *current_path;
t2fs_record rootDir; 

// FUN��ES AUXILIARES

WORD wordConvert(int *pos, BYTE *buffer){
	WORD bird, auxbuffer[2] = {0};
	for (i = 0; i < 2; i++){
		auxbuffer[i] = buffer[(*pos)+1-i];
	}
	bird = (unsigned short int) (auxbuffer[0] | (auxbuffer[1] << 8));
	(*pos) += 2;
	return bird;
}

DWORD dWordConvert(int pos, BYTE *buffer){
	DWORD birdbird, auxbuffer[4] = {0};
	for (i = 0; i < 4; i++){
		theword = buffer[(*pos)+3-i];
	}
	birdbird = (unsigned int) (auxbuffer[0] | (auxbuffer[1] << 8) | (auxbuffer[2] << 16) | (auxbuffer[3] << 24));
	(*pos) += 4;
	return birdbird;
}

void initialize(){
	int i, *curr_pos, sector_aux;
	BYTE *buffer = (char*) malloc(SECTOR_SIZE);
	
	if (!initialized){
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
	}
}



// FUN��ES PRINCIPAIS DA BILBIOTECA

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
int identify2 (char *name, int size){
	int i;
	char *ident = "Fernando Garcia Bock 242255\nLeonardo Wellausen 261571\nJo�o Batista Manique Henz 242251\0";
	
	initialize();
	if (size >= sizeof(ident)){
		strcpy(name, ident);
	}
	else{
		for(i = 0; i < size; i++){
			name[i] = ident[i];
		}
	}
	// PLACEHOLDER DE FUN��O TBQH
}


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
FILE2 create2 (char *filename);


/*-----------------------------------------------------------------------------
Fun��o:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado � aquele informado pelo par�metro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename);


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
FILE2 open2 (char *filename);


/*-----------------------------------------------------------------------------
Fun��o:	Fecha o arquivo identificado pelo par�metro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle);


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
int read2 (FILE2 handle, char *buffer, int size);


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
int write2 (FILE2 handle, char *buffer, int size);


/*-----------------------------------------------------------------------------
Fun��o:	Fun��o usada para truncar um arquivo.
	Remove do arquivo todos os bytes a partir da posi��o atual do contador de posi��o (CP)
	Todos os bytes a partir da posi��o CP (inclusive) ser�o removidos do arquivo.
	Ap�s a opera��o, o arquivo dever� contar com CP bytes e o ponteiro estar� no final do arquivo

Entra:	handle -> identificador do arquivo a ser truncado

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle);


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
int seek2 (FILE2 handle, unsigned int offset);


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
int mkdir2 (char *pathname);


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
int rmdir2 (char *pathname);


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
int chdir2 (char *pathname);


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
int getcwd2 (char *pathname, int size);


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
DIR2 opendir2 (char *pathname);


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
-----------------------------------------------------------------------------*/
#define	END_OF_DIR	1
int readdir2 (DIR2 handle, DIRENT2 *dentry);


/*-----------------------------------------------------------------------------
Fun��o:	Fecha o diret�rio identificado pelo par�metro "handle".

Entra:	handle -> identificador do diret�rio que se deseja fechar (encerrar a opera��o).

Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
	Em caso de erro, ser� retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle);
