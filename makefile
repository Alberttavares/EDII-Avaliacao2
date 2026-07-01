# ==============================================
# |     Trabalho de Estrutura de Dados II      |
# ----------------------------------------------
# | Arquivo: Makefile                          |
# ==============================================

CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o Bplus.o RH.o
EXEC = rh_system

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: all
	./$(EXEC)

clean:
	rm -f *.o $(EXEC) *.dat *.bin