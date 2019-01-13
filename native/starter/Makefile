SOURCES = starter.c 

targets = starter

TARGET  = none
CC=cc65
LD=ld65
AS=ca65
CFLAGS  = --cpu 65816  -t $(TARGET) 
#CFLAGS  =  -t $(TARGET) 
LDFLAGS = -C ld.config  -m $(targets).map none.lib 
ASFLAGS = 

DEPS = 
OBJ = starter.o 

%.o : %.c

%.s: %.c 
	$(CC)  $< -o $@ $(CFLAGS)

%.o: %.s
	$(AS) $< -o $@ $(ASFLAGS)

$(targets): %: %.o
	 $(LD) $< -o $@ $(LDFLAGS)




clean:
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.d) $(targets) $(targets).map