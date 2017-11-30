#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./testes

all: objeto movetoBin gloriousLib movetoLib

objeto: $(SRC_DIR)/t2fs.c $(INC_DIR)/apidisk.h $(INC_DIR)/t2fs.h
	$(CC) -m32 -c $(SRC_DIR)/t2fs.c

movetoBin:
	mv t2fs.o $(BIN_DIR)

gloriousLib:
	ar crs libt2fs.a $(LIB_DIR)/*.o

movetoLib:
	mv libt2fs.a $(LIB_DIR)

clean:
	rm -rf $(BIN_DIR)/cthread.o $(LIB_DIR)/libt2fs.a

