OBJS 	= initialization.o list.o main.o signal.o threadFunction.o
SOURCE	= initialization.c list.c main.c signal.c threadFunction.c
HEADER  = initialization.h list.h main.h signal.h threadFunction.h struct.h
OUT  	= server
CC	= gcc
FLAGS   = -g -c -pthread

$(OUT): $(OBJS)
	$(CC) -g -pthread $(OBJS) -o $@


main.o: main.c
	$(CC) $(FLAGS) main.c

initialization.o: initialization.c
	$(CC) $(FLAGS) initialization.c

list.o: list.c
	$(CC) $(FLAGS) list.c

signal.o: signal.c
	$(CC) $(FLAGS) signal.c

threadFunction.o: threadFunction.c
	$(CC) $(FLAGS) threadFunction.c

clean:
	rm -f $(OBJS) $(OUT)