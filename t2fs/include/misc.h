#ifndef MISC_HEADER
#define MISC_HEADER



WORD wordConvert(int *pos, BYTE *buffer);

DWORD dWordConvert(int pos, BYTE *buffer);

void initialize(int* initialized,t2fs_superbloco* superbloco,t2fs_record* rootDir);

int insereListaArqAbertos(t2fs_record* novo_record, t2fs_record* lista_arq_abertos);

DWORD procuraClusterVazio(DWORD pFATSectorStart,DWORD DataSectorStart);


#endif
//END OF FILE
