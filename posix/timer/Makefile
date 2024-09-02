# Set parent folder name as target elf
obj-elf := $(shell basename $(shell pwd))

### Simplified CFLAGS
INCLUDES ?= ../../inc/
DEFINES ?=

### Simplified LDFLAGS
LIBPATHS ?= ../../
LIBRARIES ?= ohmylib rt

### CFLAGS
CFLAGS := -Wextra -Wall -Og -g
CFLAGS += $(call cc-option,-fno-PIE)
CFLAGS += $(INCLUDES:%=-I%)
CFLAGS += $(DEFINES:%=-D%)
CFLAGS += $(LIBPATHS:%=-L%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### LDFLAGS
LDFLAGS ?= -Wl,--start-group $(LIBRARIES:%=-l%) -Wl,--end-group

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