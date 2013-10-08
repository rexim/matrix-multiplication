
multiply: main.cpp Matrix.h Makefile
	g++ -std=c++11 -Wall -Wextra -Werror -O2 -o multiply main.cpp -pthread
