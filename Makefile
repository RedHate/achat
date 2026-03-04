
TARGET = achat

OBJS = achat-xor.o achat-debug.o achat-opus.o achat-termios.o achat.o

LIBS = -lopus -lasound -lpthread

all: $(TARGET)

clean:
	@rm -rf achat *.o

%.o: %.c
	@printf "\033[1;35m[CC]\033[0m $(notdir $(basename $<)).o\n" 
	@$(CC) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o

$(TARGET): $(OBJS)
	@printf "\033[1;36m[CC]\033[0m $(TARGET)\n" 
	@gcc -std=c99 $(OBJS) -o $(TARGET) $(LIBS)
	@strip -s achat
