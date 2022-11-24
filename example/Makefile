
ifeq ($(BUILD_SRC),)
	SRC_DIR := .
else
	SRC_DIR := $(BUILD_SRC)
endif

### Extend CFLAGS
INCLUDES ?=
INCLUDES += ./inc/
DEFINES ?=

### CFLAGS
CFLAGS ?= 
CFLAGS += -Wextra -Wall -Og -g
CFLAGS += $(call cc-option,-fno-PIE)

CFLAGS += $(INCLUDES:%=-I$(SRC_DIR)/%)
CFLAGS += $(DEFINES:%=-D%)

### Extend LDFLAGS
LIBPATHS :=
LIBRARIES := rt

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%)

### LDFLAGS
LDFLAGS ?=
LDFLAGS += $(LIBPATHS:%=-L%)
LDFLAGS += $(DEFINES:%=-D%)
LDFLAGS += -Wl,--start-group $(LIBRARIES:%=-l%) -Wl,--end-group

-include lib/subdir.mk
-include src/subdir.mk
-include ../ohmylib.mk

obj := example
C_OBJ := $(C_SRC:%.c=%.c.o)

PHONY += all
all: $(obj)

-include unittest/subdir.mk

$(obj): $(C_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LLINK) -o $@

%.c.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.c.o: %.c
	@echo "$<: $(<); $@: $(@)"
	$(CC) $(CFLAGS) -c $< -o $@

PHONY += clean
clean:
	$(RM) -Rf $(BINARIES) ./src/gev-device.c.o ./src/gev-app.c.o $(C_OBJ)

PHONY += mrproper
mrproper: clean

PHONY += re
re: clean all

.PHONY: $(PHONY)

