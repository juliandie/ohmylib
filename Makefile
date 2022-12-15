
ifeq ($(BUILD_SRC),)
	SRC_DIR := .
else
	SRC_DIR := $(BUILD_SRC)
endif

### Extend CFLAGS
INCLUDES ?=
INCLUDES += ./inc/
DEFINES ?=

### Extend LDFLAGS
LIBPATHS :=
LIBRARIES := rt

### CFLAGS
CFLAGS ?= 
CFLAGS += -Wextra -Wall -Og -g
CFLAGS += -fPIC

CFLAGS += $(INCLUDES:%=-I$(SRC_DIR)/%)
CFLAGS += $(DEFINES:%=-D%)
CFLAGS += $(LIBPATHS:%=-L%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### LDFLAGS
LDFLAGS ?=
LDFLAGS += -Wl,--start-group $(LIBRARIES:%=-l%) -Wl,--end-group

-include cppcheck.mk

C_SRC := $(wildcard $(SRC_DIR)/*.c)
-include src/subdir.mk

C_OBJ := $(C_SRC:%.c=%.c.o)

TARGET:= libohmylib.a
PHONY := all
all: $(TARGET)

libohmylib.a: $(C_OBJ)
	$(AR) rcs $@ $^

libohmylib.so: $(C_OBJ)
	$(CC) $(CFLAGS) -fPIC -shared $^ $(LLINK) -o $@

unittests:
	$(MAKE) -C unittests

# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.c.o: ./%.c ./%.h
	$(CC) $(CFLAGS) -c $< -o $@
	
# Skeleton for subdir.mk, replace "%" with "relative/path/%"
%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

PHONY += cppcheck
cppcheck:
	cppcheck $(CPPCHECKFLAGS)

PHONY += cppcheck_config
cppcheck_config:
	cppcheck $(CPPCHECKFLAGS) --check-config

PHONY += clean
clean:
	$(RM) -Rf $(TARGET) $(C_OBJ)

PHONY += mrproper
mrproper: clean

PHONY += re
re: clean all

.PHONY: $(PHONY)

# vim: noet ts=8 sw=8