# make trafficController
# make clean

trafficController: trafficController.o my402list.o initializeParameters.o
	gcc -o trafficController -g trafficController.o my402list.o initializeParameters.o -pthread -lm
	tar cvzf trafficController.tar.gz trafficController.c initializeParameters.c initializeParameters.h my402list.c my402list.h defs.h cs402.h Makefile w2-README.txt

trafficController.o: trafficController.c my402list.h defs.h
	gcc -g -c -Wall trafficController.c

my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

initializeParameters.o: initializeParameters.c initializeParameters.h
	gcc -g -c -Wall initializeParameters.c 

clean:
	rm -f *.o *.gz trafficController 
