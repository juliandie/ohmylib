# Set parent folder name as target elf
obj-elf := $(shell basename $(shell pwd))

### Simplified CFLAGS
INCLUDES ?=
DEFINES ?=

### Simplified LDFLAGS
LIBPATHS ?=
LIBRARIES ?= rt

### CFLAGS
CFLAGS := -Wextra -Wall -Og -g
CFLAGS += $(call cc-option,-fno-PIE)
CFLAGS += $(INCLUDES:%=-I%) $(DEFINES:%=-D%) $(LIBPATHS:%=-L%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### LDFLAGS
LDFLAGS ?= -Wl,--start-group $(LLINK) -Wl,--end-group

obj-c := $(wildcard *.c */*.c)
obj-cpp := $(wildcard *.cpp */*.cpp)
obj-asm := $(wildcard *.S */*.S)

obj-o := $(obj-c:%.c=%.o)

PHONY += all
all: $(obj-elf)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(obj-elf): $(obj-o)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LLINK) -o $@

ohmylib: 
	$(MAKE) -C ../../ libohmylib.a
	
PHONY += cppcheck
cppcheck:
	cppcheck ./

PHONY += clean
clean:
	$(RM) -Rf $(obj-elf) $(obj-o)

.PHONY: $(PHONY)