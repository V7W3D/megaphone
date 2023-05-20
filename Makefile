CC = gcc
BIB = -lpthread
CFlags = -Wall

serveur : serveur.o fil.o msgcli.o msgsrv.o user.o
	$(CC) $(CFlags) -o serveur serveur.o fil.o msgcli.o msgsrv.o user.o $(BIB)

client: client.o fil.o msgcli.o msgsrv.o user.o
	$(CC) $(CFlags) -o client client.o fil.o msgcli.o msgsrv.o user.o $(BIB)

serveur.o : serveur.c
	$(CC) $(CFlags) -c serveur.c $(BIB)

fil.o : fil.c fil.h
	$(CC) $(CFlags) -c fil.c $(BIB)

msgcli.o : msgcli.c msgcli.h
	$(CC) $(CFlags) -c msgcli.c $(BIB)

msgsrv.o : msgsrv.c msgsrv.h
	$(CC) $(CFlags) -c msgsrv.c $(BIB)

user.o : user.c user.h
	$(CC) $(CFlags) -c user.c $(BIB)

clean:
	rm -f *.o client serveur $(BIB)