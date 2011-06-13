#
# Makefile
#
CFLAGS = -g -O0 
LDFLAGS = -lext2fs #-lsqlite3

extcarve:
	gcc $(CFLAGS) $(LDFLAGS) src/extcarve.c -o extcarve

clean:
	rm -f extcarve

all: extcarve 

.PHONY: clean

.PHONY: extcarve all
