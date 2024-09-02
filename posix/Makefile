dirs := $(patsubst %/, %, $(sort $(dir $(wildcard $(CURDIR)/*/))))
targets := $(patsubst $(CURDIR)/%, %, $(dirs))

PHONY := all
all: build

test:
	@echo $(dirs)
	@echo ""
	@echo $(targets)

PHONY += build
build:
	for t in $(targets); do $(MAKE) -C $$t; done

PHONY += clean
clean:
	for t in $(targets); do $(MAKE) -C $$t clean; done

PHONY += mrproper
mrproper:
	for t in $(targets); do $(MAKE) -C $$t mrproper; done

define dir-goals
PHONY += $1-build
$1-build:
	$(MAKE) -C $1

PHONY += $1-clean
$1-clean:
	$(MAKE) -C $1 clean

PHONY += $1-mrproper
$1-mrproper:
	$(MAKE) -C $1 mrproper
endef

$(foreach t,$(targets),$(eval $(call dir-goals,$(t))))

.PHONY: $(PHONY)
