/** Função que inilializa parametros necessarios e chama searchDiretorio, retorna 0 se achou e -1 se não achou*/
int findDiretorio(char *nome);

/** Função que procura o diretório especificado por nome  */
int searchDiretorio(char *nome);

/** Função que chama criaDirFile, retorna 0 se criou e -1 se não*/
int createDiretorio(char *nome);

/** Função que cria efetivamente o diretório nome*/
int criaDirFile(char *nome);

/** Função que chama DeleteDirFile, retorna 0 se criou e -1 se não*/
int RemoveDiretorio(char *nome);

/** Função que deleta efetivamente o diretório nome*/
int DeleteDirFile(char *nome);

/** Função que verifica se o diretorio record está vazio */
int isDirFree(struct t2fs_record *record);

/** Função que le o conteudo do diretório e escreve seus dados na linha de comando. Chama PrintConteudoDir() */
int PrintaDiretorio(char *nome);

/** Função que acha o diretorio a ser printado e chama PrintFunc() */
int PrintConteudoDir(char *nome);

/** Função que printa na linha de comando efetivamente */
int PrintFunc(struct t2fs_record *record);
