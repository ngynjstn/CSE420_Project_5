CC = gcc # compiler
CFLAGS = -Wall -g # compile flags
LIBS = -lm # libs

SRCS = mydisksim.c # source files
OBJS = $(SRCS:.c=.o) # object files

TARG = mydisksim # target

all: $(TARG)
# generates the target executable
$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LIBS)

%.o: %.c # generates the object files
	$(CC) $(CFLAGS) -c $*.c

# cleans stuff
clean:
	rm -f $(OBJS) $(TARG) *~
