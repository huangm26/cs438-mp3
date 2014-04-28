SENDER_EXE = reliable_sender 
SENDER_OBJS = reliable_sender.o
RECEIVER_EXE = reliable_receiver 
RECEIVER_OBJS = reliable_receiver.o


COMPILER = g++
COMPILER_OPTS = -c -g -O0 -Wall -Werror -lpthread
LINKER = g++
LINKER_OPTS = -lpng -lpthread

.PHONY: all clean
all : $(SENDER_EXE) $(RECEIVER_EXE)

$(SENDER_EXE) : $(SENDER_OBJS)
	$(LINKER) $(SENDER_OBJS) $(LINKER_OPTS) -o $(SENDER_EXE)
	
$(RECEIVER_EXE) : $(RECEIVER_OBJS)
	$(LINKER) $(RECEIVER_OBJS) $(LINKER_OPTS) -o $(RECEIVER_EXE)



reliable_sender.o : reliable_sender.cpp 
	$(COMPILER) $(COMPILER_OPTS) reliable_sender.cpp
	
reliable_receiver.o : reliable_receiver.cpp 
	$(COMPILER) $(COMPILER_OPTS) reliable_receiver.cpp


clean :
	-rm -f *.o $(SENDER_EXE) $(RECEIVER_EXE) LINK