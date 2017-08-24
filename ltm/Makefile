CC:=/opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc
STRIP:=/opt/buildroot-gcc463/usr/bin/mipsel-linux-strip

CFLAGS += -std=gnu99 -Wall -Werror -fPIC
target:=ltm
objs = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY:clean all
all:$(target) 
	$(STRIP) $^
$(target):$(objs) 
	$(CC) -o $(target) $(objs)   

clean:
	rm -f $(target) $(objs)
