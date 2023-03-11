obj-dir := $(patsubst %/, %, $(sort $(dir $(wildcard */))))

PHONY += all
all: $(obj-dir)

# Factory: Expand goals
define make-goals
PHONY += $1-clean
$1-clean:
	$(MAKE) -C $1 clean
	
PHONY += $1-build
$1-build:
	$(MAKE) -C $1
	
PHONY += $1-rebuild
$1-rebuild: $1-clean $1-build
	
PHONY += $1
$1:
	$(MAKE) -C $1
endef

$(foreach d,$(obj-dir),$(eval $(call make-goals,$(d))))

clean: $(obj-dir:%=%-clean)

.PHONY: $(PHONY)