STD_C = -std=c++11
FLAGS = -Wall -O2

all: ia.out

run: all
	./ia.out < input/${INPUT}.in

ia.out: ia.h main.cpp
	g++ main.cpp -o ia.out ${STD_C} ${FLAGS}

clean:
	rm ia.out
