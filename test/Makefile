APP_DEP=../out2html
# The scripts to test.
SCRIPTS=$(wildcard *.sh)
SCRIPTS_OUTPUT=$(SCRIPTS:%.sh=%.out)
SCRIPTS_REF=$(SCRIPTS:%.sh=%.ref)
SCRIPTS_COMP=$(SCRIPTS:%.sh=%.comp)

all: $(SCRIPTS_COMP)

.PHONY: $(SCRIPTS_COMP)
$(SCRIPTS_COMP): $(SCRIPTS_OUTPUT) $(SCRIPTS_REF)
	$(call compare,$(@:%.comp=%.out),$(@:%.comp=%.ref))

$(SCRIPTS_OUTPUT): $(SCRIPTS_REF) $(APP_DEP)
	$(call print,"Create output file: $(@)")
	@./$(@:%.out=%.sh) > $@ 2>&1 

$(SCRIPTS_REF):
	$(call print,"Generate reference file: $(@)")
	@./$(@:%.ref=%.sh) > $@ 2>&1 

clean:
	@-rm -f $(SCRIPTS_OUTPUT)

RESET = "\\033[0m"
GREEN = "\\033[1\;32m"
RED   = "\\033[1\;31m"
BOLD  = "\\033[1m"


define print
@{																		\
echo "==== $(BOLD)$(1)$(RESET) ====";\
};
endef

define compare
@{																		\
local err cmd start;													\
cmd="diff $(1) $(2)";													\
start="'$(1)'-'$(2)'";						\
out=$$($${cmd} 2>&1);													\
err=$${?};																\
if test "$${err}" -eq "0"; 												\
then																  	\
	printf "[$(GREEN)%s$(RESET)] $(BOLD)Compare: $(RESET) %s\n" "PASS" "$${start}";\
else																	\
	printf "[$(RED)%s$(RESET)] $(BOLD)Compare: $(RESET) %s\n" "FAIL" "$${start}";\
fi; 																	\
};
endef 
