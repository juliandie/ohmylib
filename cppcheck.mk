CPPCHECKDEFINES :=
CPPCHECKDEFINES += $(DEFINES)

CPPCHECKINCLUDES :=
CPPCHECKINCLUDES += $(INCLUDES)

CPPCHECKIGNORES := 
CPPCHECKIGNORES += doc

################################################################################
# cppcheck flags
################################################################################

CPPCHECKFLAGS := $(CPPCHECKIGNORES:%=-i%)
CPPCHECKFLAGS += $(SRC_DIR)
CPPCHECKFLAGS += $(CPPCHECKINCLUDES:%=-I %)
CPPCHECKFLAGS += $(CPPCHECKDEFINES:%=-D%)
CPPCHECKFLAGS += --force
CPPCHECKFLAGS += --error-exitcode=1
CPPCHECKFLAGS += --language=c
#CPPCHECKFLAGS += --std=c89
CPPCHECKFLAGS += --std=c99
#CPPCHECKFLAGS += --max-ctu-depth=8
#CPPCHECKFLAGS += --inline-suppr
#CPPCHECKFLAGS += --enable=warning,missingInclude,style,performance
#CPPCHECKFLAGS += --enable=warning,performance
CPPCHECKFLAGS += --enable=all
CPPCHECKFLAGS += --platform=unspecified
CPPCHECKFLAGS += --template="{file}:{line}: {severity} ({id}): {message}"
CPPCHECKFLAGS += --suppressions-list=.cppcheck_suppressions.list
CPPCHECKFLAGS += -q
#CPPCHECKFLAGS += --check-config