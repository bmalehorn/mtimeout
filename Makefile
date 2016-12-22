all: mtimeout

%: %.c
	cc -std=c99 -o $@ $<
