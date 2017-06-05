ifneq ($(KERNELRELEASE),)
# Kbuild part of Makefile.
include Kbuild

else
# Normal makefile.
KDIR ?= ${HOME}/linux-stable

all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif
