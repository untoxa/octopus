GBDK := gbdk-n
LD := sdcc
CC := sdcc
CPPFLAGS := -mgbz80 -I$(GBDK)/include/ -I$(GBDK)/include/asm/
LDFLAGS := -mgbz80 --no-std-crt0 --data-loc 0xc0a0 -L$(GBDK)/lib $(GBDK)/lib/crt0.rel -lgb

all: build octopus.gb
build:
	mkdir build
%.gb: build/%.ihx
	makebin -Z $< > $@
build/%.ihx: build/%.rel
	$(LD) $(LDFLAGS) -o $@ $<
build/%.rel: src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<
