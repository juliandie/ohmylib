### Extend CFLAGS
INCLUDES ?=
INCLUDES += ./inc/
DEFINES ?=

### Extend LDFLAGS
LIBPATHS :=
LIBRARIES := rt

### CFLAGS
CFLAGS ?= -Wextra -Wall -Og -g -fPIC
CFLAGS += $(call cc-option,-fno-PIE)
CFLAGS += $(addprefix  -I, $(INCLUDES))
CFLAGS += $(addprefix  -L, $(LIBPATHS))
CFLAGS += $(addprefix  -D, $(DEFINES))

### Linked libraries
LLINK := -pthread $(addprefix  -l, $(LIBRARIES))

### LDFLAGS
LDFLAGS ?=
LDFLAGS += -Wl,--start-group $(LLINK) -Wl,--end-group

SRC_DIR := ./
-include cppcheck.mk

C_SRC := $(wildcard $(CURDIR)/*.c)
-include src/subdir.mk

C_OBJ := $(C_SRC:%.c=%.o)

TARGET:= libohmylib.a
PHONY := all
all: $(TARGET)

libohmylib.a: $(C_OBJ)
	$(AR) rcs $@ $^

libohmylib.so: $(C_OBJ)
	$(CC) $(CFLAGS) -fPIC -shared $^ $(LLINK) -o $@

examples:
	$(MAKE) -C examples

# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@
	
# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

PHONY += cppcheck
cppcheck:
	cppcheck $(CPPCHECKFLAGS)

PHONY += cppcheck_config
cppcheck_config:
	cppcheck $(CPPCHECKFLAGS) --check-config

PHONY += clean
clean:
	$(MAKE) -C examples clean
	$(RM) -Rf $(TARGET) $(C_OBJ)

PHONY += mrproper
mrproper: clean

PHONY += re
re: clean all

.PHONY: $(PHONY)

# vim: noet ts=8 sw=8
