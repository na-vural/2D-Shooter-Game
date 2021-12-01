CC = g++
CFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

all: game

game:	game.o
	${CC} game.o -o game ${CFLAGS}

game.o:	game.cpp
	${CC} -c game.cpp

clean:
	rm game.o game
