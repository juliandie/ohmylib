
ifeq ($(BUILD_SRC),)
	SRC_DIR := .
else
	SRC_DIR := $(BUILD_SRC)
endif

### Extend CFLAGS
INCLUDES ?=
INCLUDES += ./inc/
-include ../ohmylib.mk
INCLUDES += ../$(OHMYLIB_INCLUDES)
DEFINES ?=

### Extend LDFLAGS
LIBPATHS ?=
LIBPATHS += .. 
LIBRARIES ?= 

### CFLAGS
CFLAGS := -Wextra -Wall -Og -g
CFLAGS += $(call cc-option,-fno-PIE)

CFLAGS += $(INCLUDES:%=-I$(SRC_DIR)/%)
CFLAGS += $(DEFINES:%=-D%)
CFLAGS += $(LIBPATHS:%=-L%)

### Linked libraries
LLINK := -pthread $(LIBRARIES:%=-l%) $(OHMYLIB_LIBRARIES)

### LDFLAGS
LDFLAGS ?=
LDFLAGS += -Wl,--start-group $(LIBRARIES:%=-l%) -Wl,--end-group

C_SRC := $(wildcard $(SRC_DIR)/*.c)
UNITTESTS := $(C_SRC:%.c=%)

PHONY += all
all: $(UNITTESTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LLINK) -o $@
	
ohmylib: 
	$(MAKE) -C ../
	
PHONY += cppcheck
cppcheck:
	cppcheck ./

PHONY += clean
clean:
	$(RM) -Rf $(UNITTESTS)

PHONY += mrproper
mrproper: clean

PHONY += re
re: clean all

.PHONY: $(PHONY)