CFLAGS=-Wall -Wextra -Werror

build: tema1

tema1: functions.o main.cpp
	g++ $(CFLAGS) functions.o main.cpp -lpthread -o tema1 

functions.o: functions.cpp
	g++ $(CFLAGS) functions.cpp -lpthread -c -o functions.o 

.PHONY: clean

clean:
	rm *.o
	rm tema1