IDIR=deps/inih
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=out

_DEPS = ini.o
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = nightscout-ps1.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ) $(DEPS))

t:
	@echo $(DEPS)
	@echo $(OBJ)

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p "$(dir $@)"
	$(CC) -c -o $@ $< $(CFLAGS)

nightscout-ps1: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf $(ODIR) nightscout-ps1
