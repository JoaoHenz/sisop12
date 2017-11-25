all: dirt2 rmdirt2 mkdirt2 cpy2t2 cleanObject

dirt2: ./src/dirt2.c libt2fs.a
	gcc -o ./bin/dirt2 ./src/dirt2.c -I./include -L./lib -lt2fs -lapidisk -Wall -lm

rmdirt2: ./src/mkdirt2.c libt2fs.a
	gcc -o ./bin/rmdirt2 ./src/rmdirt2.c -I./include -L./lib -lt2fs -lapidisk -Wall -lm

mkdirt2: ./src/mkdirt2.c libt2fs.a
	gcc -o ./bin/mkdirt2 ./src/mkdirt2.c -I./include -L./lib -lt2fs -lapidisk -Wall -lm

cpy2t2: ./src/cpy2t2.c libt2fs.a
	gcc -o ./bin/cpy2t2 ./src/cpy2t2.c -I./include -L./lib -lt2fs -lapidisk -Wall -lm	

libt2fs.a: libt2fs.o ./lib/libapidisk.a
	ar crs ./lib/libt2fs.a ./bin/libt2fs.o ./lib/libapidisk.a

libt2fs.o:  ./src/libt2fs.c
	gcc -c -I./include ./src/libt2fs.c -Wall -ls -o ./bin/libt2fs.o	

cleanObject:
	rm -f ./bin/*.o

clean:
	rm -f ./lib/libt2fs.a ./bin/*.o ./bin/cpy2t2 ./bin/mkdirt2 ./bin/dirt2 ./bin/rmdirt2
