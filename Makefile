# Set parent folder name as target lib
#obj-lib := $(shell basename $(shell pwd))
obj-lib := libohmylib.a

### Simplified CFLAGS
# Includes will be prefixed with -I automatically
INCLUDES ?= ./includes/
# Defines will be prefixed with -D automatically
DEFINES ?=

### Simplified LDFLAGS
# Libpaths will be prefixed with -L automatically
LIBPATHS ?= 
# Libpaths will be prefixed with -l automatically
# Add libraries like pthread to LLINK directly
#LIBRARIES ?= rt

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### CFLAGS
ccflags ?= -Wextra -Wall -Og -g -fPIC
ccflags += $(call cc-option,-fno-PIE)
ccflags += $(INCLUDES:%=-I%) $(LIBPATHS:%=-L%) $(DEFINES:%=-D%)

### LDFLAGS
LDFLAGS ?= -Wl,--start-group $(LLINK) -Wl,--end-group

#obj-c := $(wildcard *.c */*.c)
#obj-cpp := $(wildcard *.cpp */*.cpp)
#obj-asm := $(wildcard *.s */*.s)
### Include subdirs
-include lib/subdir.mk

obj-o := $(obj-c:%.c=%.o)
obj-o += $(obj-cpp:%.cpp=%.o)
obj-o += $(obj-asm:%.s=%.o)

all: $(obj-lib)

# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.o: %.c %.h
	$(CC) $(ccflags) -c $< -o $@
	
# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.o: %.c
	$(CC) $(ccflags) -c $< -o $@

libohmylib.a: $(obj-o)
	$(AR) rcs $@ $^

libohmylib.so: $(obj-o)
	$(CC) $(ccflags) -fPIC -shared $^ $(LLINK) -o $@

clean:
	$(RM) -Rf $(obj-o) libohmylib.a libohmylib.so

.PHONY: all clean

# vim: noet ts=8 sw=8
