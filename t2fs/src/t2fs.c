#ifndef T2FS_C
#define T2FS_C

#include "../include/misc.h"
#include "../include/t2fs.h"
#include <stdio.h>
#include <string.h>

#define INIT {
	initialize(&initialized,superbloco,rootDir);
}

t2fs_superbloco superbloco;
int initialized = 0;
char *current_path= "/";
t2fs_record rootDir;

t2fs_record* lista_arq_abertos[10] = { NULL,NULL,NULL,NULL,NULL,
												NULL,NULL,NULL,NULL,NULL };



// FUN��ES PRINCIPAIS DA BILBIOTECA


int identify2 (char *name, int size){ INIT;
	int i;
	char *ident = "Fernando Garcia Bock 242255\nLeonardo Wellausen 261571\nJoão Batista Manique Henz 242251\0";

	if (size >= sizeof(ident)){
		strcpy(name, ident);
		return 0;
	}
	else{
		for(i = 0; i < size; i++){
			name[i] = ident[i];
		}
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
	t2fs_record* novo_record;

	novo_record->TypeVal = 0x01;
	strcpy(novo_record->name,filename);
	novo_record->bytesFileSize = 0;
	novo_record->firstCluster = procuraClusterVazio(superbloco.pFATSectorStart) ;


	int handle = insereListaArqAbertos(novo_record,lista_arq_abertos);
	if(handle)
		return handle;



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
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Apagar um arquivo do disco.
		O nome do arquivo a ser apagado � aquele informado pelo par�metro "filename".

	Entra:	filename -> nome do arquivo a ser apagado.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



FILE2 open2 (char *filename){ INIT;
	return -1;
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
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o arquivo identificado pelo par�metro "handle".

	Entra:	handle -> identificador do arquivo a ser fechado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int read2 (FILE2 handle, char *buffer, int size){ INIT;
	return -1;
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
	return -1;
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
	return -1;
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
	return -1;
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
	return -1;
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
	return -1;
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
	return -1;
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
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o diret�rio identificado pelo par�metro "handle".

	Entra:	handle -> identificador do diret�rio que se deseja fechar (encerrar a opera��o).

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}











#endif
//END OF FILE
