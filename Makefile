CC=gcc
AR=ar

CONFIGS= \
	 CONFIG_FS=y

CFLAGS+=-Werror -Wall -g \
	$(patsubst %=y,-D%,$(filter %=y,$(strip $(CONFIGS)))) \
	-DKSRC=\"$(RUNTIME_PATH)\" \
	$(patsubst %,-I%,$(INCLUDES)) -Ikut/include

#
# Linux Emulation
#

# lnx fs
LNX_DEBUGFS_OBJS=inode.o
LNX_FS_OBJS=namei.o libfs.o \
	$(patsubst %,debugfs/%,$(LNX_DEBUGFS_OBJS))

# lnx drivers
LNX_DRIVERS_BASE_OBJS=core.o
LNX_DRIVERS_OBJS=$(patsubst %,base/%,$(LNX_DRIVERS_BASE_OBJS))

# all lnx objs
LNX_OBJS= \
	$(patsubst %,fs/%,$(LNX_FS_OBJS)) \
	$(patsubst %,drivers/%,$(LNX_DRIVERS_OBJS))

#
# Unit Test Helper Functions
#

# kut debugfs
KUT_DEBUGFS_OBJS=inode.o

# kut lib
KUT_LIB_OBJS=bug.o

# kut fs
KUT_FS_OBJS=namei.o open.o

# all kut objs
KUT_OBJS= \
	$(patsubst %,debugfs/%,$(KUT_DEBUGFS_OBJS)) \
	$(patsubst %,lib/%,$(KUT_LIB_OBJS)) \
	$(patsubst %,fs/%,$(KUT_FS_OBJS))

#
# All Objects
#
OBJS=$(patsubst %,lnx/%,$(LNX_OBJS)) $(patsubst %,kut/%,$(KUT_OBJS))

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

all: libkernel

libkernel: $(OBJS)
	$(AR) -ru $@.a $^

clean:
	@rm -f $(OBJS)

cleanall: clean
	@rm -rf tags
	@rm -f libkernel.a

