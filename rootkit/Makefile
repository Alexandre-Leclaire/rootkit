KERNELDIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)

obj-m = rootkit.o

rootkit-objs = rootkit_main.o \
                hide/hide.o \
                persistence/persistence.o \
                connection/client.o \
				commands/commands.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
