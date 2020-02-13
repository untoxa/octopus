GBDK := gbdk-n
LD := sdcc
CC := sdcc
CPPFLAGS := -mgbz80 -DUSE_SFR_FOR_REG -I$(GBDK)/include/ -I$(GBDK)/include/asm/
LDFLAGS := -mgbz80 --no-std-crt0 --data-loc 0xc0a0 -L$(GBDK)/lib $(GBDK)/lib/crt0.rel -lgb

all: src/octopus.gb
%.gb: %.ihx
	makebin $< > $@
%.ihx: %.rel
	$(LD) $(LDFLAGS) -o $@ $<
%.rel: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<
