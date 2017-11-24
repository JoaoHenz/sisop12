#ifndef MISC_HEADER
#define MISC_HEADER




#include "./tsfs.h"

WORD wordConvert(int *pos, BYTE *buffer);
DWORD dWordConvert(int pos, BYTE *buffer);
void initialize(int* initialized,t2fs_superbloco* superbloco,t2fs_record* rootDir);






#endif
//END OF FILE
