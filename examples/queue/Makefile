# Set parent folder name as target elf
obj-elf := $(shell basename $(shell pwd))

### Simplified CFLAGS
# Includes will be prefixed with -I automatically
INCLUDES ?= ../../inc
# Defines will be prefixed with -D automatically
DEFINES ?=

### Simplified LDFLAGS
# Libpaths will be prefixed with -L automatically
LIBPATHS ?= ../../
# Libpaths will be prefixed with -l automatically
# Add libraries like pthread to LLINK directly
LIBRARIES ?= ohmylib

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### CFLAGS
ccflags := $(CFLAGS) -Wextra -Wall -Og -g
ccflags += $(INCLUDES:%=-I%) $(LIBPATHS:%=-L%) $(DEFINES:%=-D%)

### LDFLAGS
LDFLAGS ?= -Wl,--start-group $(DEFINES:%=-l%) -Wl,--end-group

obj-c := $(wildcard *.c */*.c)
obj-cpp := $(wildcard *.cpp */*.cpp)
obj-asm := $(wildcard *.s */*.s)
### Include subdirs
# -include src/subdir.mk

obj-o := $(obj-c:%.c=%.o)
obj-o += $(obj-cpp:%.cpp=%.o)
obj-o += $(obj-asm:%.s=%.o)

all: $(obj-elf)

$(obj-elf): $(obj-o)
	$(CC) $(ccflags) $(LDFLAGS) $^ $(LLINK) -o $@

%.o: %.c
	$(CC) $(ccflags) -c $< -o $@

ohmylib:
	$(MAKE) -C ../../ clean all

clean:
	$(RM) -Rf $(obj-elf) $(obj-o)

.PHONY: all clean
