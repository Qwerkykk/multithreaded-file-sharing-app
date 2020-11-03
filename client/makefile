OBJS 	= initialization.o list.o main.o pool.o signal.o socketServe.o utils.o workerThread.o
SOURCE	= initialization.c list.c main.c pool.c signal.c socketServe.c utils.c workerThread.c
HEADER  = initialization.h list.h pool.h signal.h socketServe.h utils.h workerThread.h struct.h
OUT  	= client
CC	= gcc
FLAGS   = -g -c -pthread

$(OUT): $(OBJS)
	$(CC) -g -pthread $(OBJS) -o ./client1/$@
	$(CC) -g  -pthread $(OBJS) -o ./client2/$@
	$(CC) -g -pthread $(OBJS) -o ./client3/$@
	$(CC) -g -pthread $(OBJS) -o ./client4/$@


main.o: main.c
	$(CC) $(FLAGS) main.c

initialization.o: initialization.c
	$(CC) $(FLAGS) initialization.c

list.o: list.c
	$(CC) $(FLAGS) list.c

pool.o: pool.c
	$(CC) $(FLAGS) pool.c


signal.o: signal.c
	$(CC) $(FLAGS) signal.c

socketServe.o: socketServe.c
	$(CC) $(FLAGS) socketServe.c

utils.o: utils.c
	$(CC) $(FLAGS) utils.c

workerThread.o: workerThread.c
	$(CC) $(FLAGS) workerThread.c


clean:
	rm -f $(OBJS) ./client1/$(OUT) ./client2/$(OUT) ./client3/$(OUT) ./client4/$(OUT)

