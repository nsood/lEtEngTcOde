CC=/opt/buildroot-gcc463/usr/bin/mipsel-buildroot-linux-uclibc-gcc

CFLAGS += -O2 -std=gnu99 -Wall -Wno-unused-but-set-variable -Werror

HOMEDIR := $(shell pwd)/..
CFLAGS += -I$(HOMEDIR)/
CFLAGS += -I$(HOMEDIR)/uClibc-0.9.33.2/app_headers/include
CFLAGS += -I$(HOMEDIR)/lib/include
CFLAGS += -I$(HOMEDIR)/linux-2.6.36.x/include
CFLAGS += -I$(HOMEDIR)/../profile/86.1/include
LDFLAGS += -L$(HOMEDIR)/lib/lib -L$(HOMEDIR)/uClibc-0.9.33.2/lib

CFLAGS += -I../user/goahead/src/
CFLAGS += -L../user/goahead/src/
CFLAGS += -I../user/json-c/json-c-0.11/
CFLAGS += -L../user/json-c/json-c-0.11/.libs/
#LDFLAGS += -static

LDFLAGS += -ljson-c
#LDFLAGS += -lnvram
#LDFLAGS += -lcommon

target:=tqos

objs = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY:clean all 

all:$(target)

$(target) : $(objs)
	$(CC) $(CFLAGS) -o  $@ $^ $(LDFLAGS)

romfs:

clean:
	rm -f $(objs) $(target)
