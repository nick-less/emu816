SOURCES = loader.s

PROGRAM = loader

TARGET  = none
CC      = cl65
AS      = ca65
LDFLAGS = -C ld.config -t $(TARGET) -m $(PROGRAM).map

CFLAGS=-I.
DEPS = 
OBJ = loader.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

loader: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	xxd -i loader >loader.h


clean:
	$(RM)  $(PROGRAM) $(PROGRAM).map