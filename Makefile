# Set parent folder name as target lib
#obj-lib := $(shell basename $(shell pwd))
obj-lib := libohmylib.a

### Simplified CFLAGS
INCLUDES ?= ./includes/
DEFINES ?=

### Simplified LDFLAGS
LIBPATHS ?= 
# Add libraries like pthread to LLINK directly
LIBRARIES ?=

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### CFLAGS
ccflags ?= -Wextra -Wall -Og -g -fPIC
ccflags += $(call cc-option,-fno-PIE)
ccflags += $(INCLUDES:%=-I%) $(LIBPATHS:%=-L%) $(DEFINES:%=-D%)

### LDFLAGS
LDFLAGS ?= -Wl,--start-group $(LLINK) -Wl,--end-group

#obj-c   := $(wildcard *.c)
#obj-cpp := $(wildcard *.cpp)
#obj-asm := $(wildcard *.s)
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
	
cppcheck:
	cppcheck ./

clean:
	$(RM) -Rf $(obj-o) libohmylib.a libohmylib.so

.PHONY: all cppcheck clean 

# vim: noet ts=8 sw=8