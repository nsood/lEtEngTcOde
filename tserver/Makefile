CFLAGS += -O2 -std=gnu99 -Wall -Wno-unused-but-set-variable -Werror
CFLAGS += -I../json-c/json-c-0.11/                                                              
CFLAGS += -L../../romfs/lib/
#LDFLAGS += -static

LDFLAGS += -ljson-c

target:=tqos

objs = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY:clean all 

all:$(target)

$(target) : $(objs)
	$(CC) $(CFLAGS) -o  $@ $^ $(LDFLAGS)

romfs:

clean:
	rm -f $(objs) $(target)
