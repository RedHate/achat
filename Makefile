###############
# ultros 2026
###############

TARGET = achat

OBJS = achat-xor.o achat-debug.o achat-filters.o achat-opus.o achat-alsa.o achat-termios.o achat.o

LIBS = -lopus -lasound -lpthread -lm

all: clean $(TARGET)

server:  $(TARGET)
	@printf "\033[1;36m[CC]\033[0m server\n"
	@./achat 6688
	
client-1: clean $(TARGET)
	@printf "\033[1;36m[CC]\033[0m client\n"
	@./achat 127.0.0.1 6688 plug:hw:3 plug:hw:0

client-2: clean $(TARGET)
	@printf "\033[1;36m[CC]\033[0m client\n"
	@./achat 127.0.0.1 6688 plug:hw:0 plug:hw:3

clean:
	@rm -rf achat *.o

%.o: %.c
	@printf "\033[1;35m[CC]\033[0m $(notdir $(basename $<)).o\n" 
	@$(CC) -c $< -o $(basename $<).o

$(TARGET): $(OBJS)
	@printf "\033[1;36m[CC]\033[0m $(TARGET)\n" 
	@gcc -std=c99 $(OBJS) -o $(TARGET) $(LIBS)
	@strip -s achat
