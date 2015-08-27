CC=gcc
AR=ar

CFLAGS+=-Werror -Wall -g \
	-DKSRC=\"$(RUNTIME_PATH)\" \
	$(CONFIGS_Y:%=-D%) \
	$(INCLUDES:%=-I%)

#
# Linux Emulation
#

# lnx fs
LNX_DEBUGFS_OBJS=inode.o
LNX_FS_OBJS=namei.o libfs.o \
	$(patsubst %,debugfs/%,$(LNX_DEBUGFS_OBJS))

# lnx drivers
LNX_DRIVERS_BASE_OBJS=core.o firmware_class.o
LNX_DRIVERS_MMC_CORE_OBJS=core.o host.o mmc.o mmc_ops.o
LNX_DRIVERS_MMC_OBJS= \
		      $(patsubst %,core/%,$(LNX_DRIVERS_MMC_CORE_OBJS))

LNX_DRIVERS_OBJS= \
		  $(patsubst %,base/%,$(LNX_DRIVERS_BASE_OBJS)) \
		  $(patsubst %,mmc/%,$(LNX_DRIVERS_MMC_OBJS))

# lnx lib
LNX_LIB_OBJS=kstrtox.o scatterlist.o

# lnx mm
LNX_MM_OBJS=page_alloc.o kmemleak.o

# all lnx objs
LNX_OBJS= \
	$(patsubst %,fs/%,$(LNX_FS_OBJS)) \
	$(patsubst %,drivers/%,$(LNX_DRIVERS_OBJS)) \
	$(patsubst %,lib/%,$(LNX_LIB_OBJS)) \
	$(patsubst %,mm/%,$(LNX_MM_OBJS))

#
# Unit Test Helper Functions
#

# kut lib
KUT_LIB_OBJS=bug.o random32.o

# kut kernel
KUT_KERNEL_DEBUG=debug_core.o
KUT_KERNEL_OBJS=$(patsubst %,debug/%,$(KUT_KERNEL_DEBUG))

# kut fs
KUT_FS_OBJS=namei.o open.o

# kut mm
KUT_MM_OBJS=memory.o 

# kut drivers
KUT_DRIVERS_BASE_OBJS=core.o
KUT_DRIVERS_MMC_OBJS=core/bus.o core/mmc_ops.o core/host.o host/kunit.o
KUT_DRIVERS_OBJS= \
		  $(patsubst %,base/%,$(KUT_DRIVERS_BASE_OBJS)) \
		  $(patsubst %,mmc/%,$(KUT_DRIVERS_MMC_OBJS))

# all kut objs
KUT_OBJS= \
	$(patsubst %,lib/%,$(KUT_LIB_OBJS)) \
	$(patsubst %,kernel/%,$(KUT_KERNEL_OBJS)) \
	$(patsubst %,fs/%,$(KUT_FS_OBJS)) \
	$(patsubst %,mm/%,$(KUT_MM_OBJS)) \
	$(patsubst %,drivers/%,$(KUT_DRIVERS_OBJS))

#
# All Objects
#
OBJS=$(patsubst %,lnx/%,$(LNX_OBJS)) $(patsubst %,kut/%,$(KUT_OBJS))

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

all: libkernel

libkernel: $(OBJS)
	$(AR) -r $@.a $^

clean:
	@rm -f $(OBJS)

cleanall: clean
	@rm -rf tags
	@rm -f libkernel.a

