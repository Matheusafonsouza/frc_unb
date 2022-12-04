LDFLAGS = -lrt 
BLDDIR = .
INCDIR = $(BLDDIR)/inc
SRCDIR = $(BLDDIR)/src
OBJDIR = $(BLDDIR)/obj
CFLAGS = -c -Wall -I$(INCDIR)
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
EXE = bin/main

all: clean $(EXE) 
    
$(EXE): $(OBJ) 
	gcc  $(OBJDIR)/*.o $(LDFLAGS) -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	gcc $(CFLAGS) $< -o $@

kill:
	fuser 8001/udp -k -i || true
	fuser 8002/udp -k -i || true
	fuser 8003/udp -k -i || true
	fuser 8004/udp -k -i || true

run:
	chmod +x $(EXE)
	$(EXE) $(IP_SERVER) $(PORT_SERVER) $(PORT_SERVER2) $(IP_CLIENT) $(PORT_CLIENT)
