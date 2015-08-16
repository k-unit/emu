CC=gcc
AR=ar

CONFIGS= \
	 CONFIG_FS=y

CFLAGS+=-Werror -Wall -g \
	$(patsubst %=y,-D%,$(filter %=y,$(strip $(CONFIGS)))) \
	-DKSRC=\"$(RUNTIME_PATH)\" \
	$(patsubst %,-I%,$(INCLUDES))

#fs
DEBUGFS_OBJS=inode.o
FS_OBJS=namei.o open.o libfs.o \
	$(patsubst %,debugfs/%,$(DEBUGFS_OBJS))

# drivers
DRIVERS_BASE_OBJS=core.o
DRIVERS_OBJS=$(patsubst %,base/%,$(DRIVERS_BASE_OBJS))

# all objs
OBJS=$(patsubst %,fs/%,$(FS_OBJS)) \
     $(patsubst %,drivers/%,$(DRIVERS_OBJS))

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

