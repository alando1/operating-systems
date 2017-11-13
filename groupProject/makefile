COMPILEFLAGS = -Wall -Wno-pointer-to-int-cast
COMPILECMD = gcc -c $(COMPILEFLAGS)
LINKFLAGS =   
LINKCMD = gcc $(LINKFLAGS) $(OBJS)  $(LIBRARIES) -o $(TARGET)
LIBRARIES = -lpthread -lrt
OBJS = smoker.o
TARGET = run

main:   $(OBJS)
		$(LINKCMD)

clean:
	rm *.o run

smoker.o: smoker.c
	$(COMPILECMD) smoker.c
