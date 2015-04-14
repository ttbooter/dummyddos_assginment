CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

LFLAGS  = -lpthread -lrt

# the build target executable:
SERVER = server
ATTACKER = attacker
COORDINATOR = coordinator

all: $(SERVER) $(ATTACKER) $(COORDINATOR)

$(SERVER): $(SERVER).c
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER).c $(LFLAGS)

$(ATTACKER): $(ATTACKER).c
	$(CC) $(CFLAGS) -o $(ATTACKER) $(ATTACKER).c $(LFLAGS)

$(COORDINATOR): $(COORDINATOR).c
	$(CC) $(CFLAGS) -o $(COORDINATOR) $(COORDINATOR).c $(LFLAGS)

clean:
	$(RM) $(SERVER) $(ATTACKER) $(COORDINATOR) 

