# LRT Makefile to prepare/compile wago libraries
# to do with libFAUDES/CompileDES, April 2016

# Choose a variant
KBUSTHREAD=wago_ipc_kbus


#targets
default: libs wago_ipc_kbus.o


clean:
	- make -C libkbus clean
	- make -C libuio clean
	- rm -rf libkbus.a
	- rm -rf libuio.a
	- rm wago_ipc_kbus.o

dist-clean:
	- rm -rf libkbus
	- rm -rf libuio
	- rm -rf libkbus.a
	- rm -rf libuio.a
	- rm -rf wago_ipc_kbus.o

######################################################
# Support libs

libs: libkbus.a libuio.a


libkbus:
	tar -xzf libkbus.tgz

libkbus.a: libkbus libuio.a
	cd libkbus; ./configure --with-uio-prefix=../libuio/include  CFLAGS=-m32
	make -C libkbus
	cp libkbus/src/.libs/libkbus.a libkbus.a

libuio:
	tar -xzf libuio.tgz

libuio.a: libuio
	cd libuio; ./configure CFLAGS=-m32
	make -C libuio
	cp libuio/src/.libs/libuio.a libuio.a

######################################################
# Wago supplied background thread with wago proposed
# compile time options

CFLAGS=-DTRG_X86 -m32 -I../../  -DALIGNATTRIB= -DCDECL= -DHANDLE_WIN32_PRAGMA -O3 -Wall -Wstrict-prototypes -fno-strict-aliasing -DLINUX -DALTERNATE_DEBUG -D_REENTRANT  -DINCLUDE_HIGHRESTIMESTAMP -D__NO_SYSSTREAMS__ -D_GNU_SOURCE -DUSE_MUTEXES -DUSE_GNU -g -rdynamic -I./libkbus/include -I./libuio/include

wago_ipc_kbus.o: $(KBUSTHREAD).c
	cc -c $(CFLAGS) $< -o wago_ipc_kbus.o




