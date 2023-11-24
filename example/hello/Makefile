CC = gcc
FILES = hello.c  
OUT_EXE = hello 
 
build: $(FILES)
	$(CC) -o $(OUT_EXE) $(FILES)
 
clean:
	rm -f *.o 
	rm -f $(OUT_EXE) 
rebuild: clean build
