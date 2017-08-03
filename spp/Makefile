
obj-m := spp.o
spp-objs := main.o spp_config.o spp_network.o spp_struct.o spp_timer.o spp_comm.o

ccflags-y += -std=gnu99 -Wno-declaration-after-statement

CURRENT_DIR := $(shell pwd)
LINUX_KERNEL_DIR := $(CURRENT_DIR)/../linux-2.6.36.x

all:
	make -C $(LINUX_KERNEL_DIR) M=$(CURRENT_DIR) modules

clean:
	make -C $(LINUX_KERNEL_DIR) M=$(CURRENT_DIR) clean
