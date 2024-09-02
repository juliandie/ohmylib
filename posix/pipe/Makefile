# Set parent folder name as target elf
obj-elf := $(shell basename $(shell pwd))

### Simplified CFLAGS
INCLUDES ?= 
DEFINES ?=

### Simplified LDFLAGS
LIBPATHS ?= 
LIBRARIES ?= 

### CFLAGS
ccflags := $(CFLAGS) -Wextra -Wall -Og -g
ccflags += $(INCLUDES:%=-I%) $(LIBPATHS:%=-L%) $(DEFINES:%=-D%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)
LDFLAGS ?= -Wl,--start-group $(LLINK) -Wl,--end-group

obj-c :=   #$(wildcard *.c)
obj-cpp := #$(wildcard *.cpp)
obj-asm := #$(wildcard *.s)
obj-main := $(wildcard ./*.c)
obj-elf  := $(obj-main:%.c=%)
### Include subdirs
# -include src/subdir.mk

obj-o := $(obj-c:%.c=%.o)
obj-o += $(obj-cpp:%.cpp=%.o)
obj-o += $(obj-asm:%.s=%.o)

all: $(obj-elf)

%.o: %.c
	$(CC) $(ccflags) -c $< -o $@
	
# Factory: Expand goals for obj-elf
define make-elf-goal
$(1): $(1).o $(2)
	$(CC) $(CFLAGS) $(LDFLAGS) $$^ $(LLINK) -o $$@
endef
$(foreach elf,$(obj-elf),$(eval $(call make-elf-goal,$(elf),$(obj-o))))

clean:
	$(RM) -Rf $(obj-elf) $(obj-o)

.PHONY: all clean