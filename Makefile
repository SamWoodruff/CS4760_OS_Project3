all: master palin
master: master.c
	gcc -o master master.c -lpthread
palin: palin.c
	gcc -o palin palin.c -lpthread
clean:
	-rm master palin

