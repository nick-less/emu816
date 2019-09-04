SOURCES = loader.s

PROGRAM = loader

TARGET  = none
CC      = cl65
AS      = ca65
LDFLAGS = -C ld.config -t $(TARGET) -m $(PROGRAM).map
ASFLAGS = --listing loader.lst

CFLAGS=-I.
DEPS = 
OBJ = loader.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

loader: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	xxd -i loader >loader.h
	javac MakeSnippet.java
	java MakeSnippet


clean:
	$(RM)  $(PROGRAM) $(PROGRAM).map