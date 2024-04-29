TARGET_MODULE := rngdrv

obj-m += $(TARGET_MODULE).o
rngdrv-objs := driver.o GF.o poly.o utils.o

ccflags-y := -std=gnu99

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(CURDIR)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod $(TARGET_MODULE).ko

unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null
