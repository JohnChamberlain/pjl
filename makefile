pjl: pjl.c ui.c ../../shared/src/jcstring.c ../../shared/src/jcprint.c ../../shared/src/jctermio.c ../../shared/src/jcmemory.c
	c99 -ggdb -o pjl ui.c pjl.c ../../shared/src/jcstring.c ../../shared/src/jcprint.c ../../shared/src/jctermio.c  ../../shared/src/jcmemory.c -I../../shared/src

	\cp pjl ../bin
	
clean :
	rm -f pjl

