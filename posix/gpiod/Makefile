# Set parent folder name as target elf
obj-elf := $(shell basename $(shell pwd))

### Simplified CFLAGS
INCLUDES ?= 
DEFINES ?=

### Simplified LDFLAGS
LIBPATHS ?= 
LIBRARIES ?= gpiod

### CFLAGS
ccflags := $(CFLAGS) -Wextra -Wall -Og -g
ccflags += $(INCLUDES:%=-I%) $(LIBPATHS:%=-L%) $(DEFINES:%=-D%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)
LDFLAGS ?= -Wl,--start-group $(LLINK) -Wl,--end-group

obj-c := $(wildcard *.c */*.c)
obj-cpp := $(wildcard *.cpp */*.cpp)
obj-asm := $(wildcard *.s */*.s)
### Include subdirs
# -include src/subdir.mk

obj-o := $(obj-c:%.c=%.o)
obj-o += $(obj-cpp:%.cpp=%.o)
obj-o += $(obj-asm:%.s=%.o)

all: $(obj-elf)

%.o: %.c
	$(CC) $(ccflags) -c $< -o $@

$(obj-elf): $(obj-o)
	$(CC) $(ccflags) $(LDFLAGS) $^ $(LLINK) -o $@

clean:
	$(RM) -Rf $(obj-elf) $(obj-o)

.PHONY: all clean