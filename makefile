CC = gcc
EXECNAME = csd5328shell
SRC = hy345sh.c

all:
	@echo Compiling the shell.
	$(CC) $(SRC) -o $(EXECNAME)

clean:
	@echo Deleteting the executable...
	rm ./$(EXECNAME)
	@echo Shell deleted.