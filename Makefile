# ==============================================
# |     Trabalho de Estrutura de Dados II      |
# ----------------------------------------------
# | Arquivo: Makefile                          |
# ==============================================

CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o Bplus.o RH.o
EXEC = rh_system
SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
SAN_OBJ = main.san.o Bplus.san.o RH.san.o
SAN_EXEC = rh_system_memcheck

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: all
	./$(EXEC)

$(SAN_EXEC): $(SAN_OBJ)
	$(CC) $(CFLAGS) $(SAN_FLAGS) -o $@ $^

%.san.o: %.c
	$(CC) $(CFLAGS) $(SAN_FLAGS) -c $< -o $@

memcheck: $(SAN_EXEC)
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./$(SAN_EXEC)

clean:
	rm -f *.o *.san.o $(EXEC) $(SAN_EXEC) *.dat *.bin
