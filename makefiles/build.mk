# Copyright (C) 2008, 2009-2011, 2013-2015 Apple Inc.  All rights reserved.
# Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
#
# This document is the property of Apple Inc.
# It is considered confidential and proprietary.
#
# This document may not be reproduced or transmitted in any form,
# in whole or in part, without the express written permission of
# Apple Inc.
#

###############################################################################
# Rules for building objects.
#

_INCLUDES		:=	$(addprefix -I,$(GLOBAL_INCLUDES)) $(addprefix -I,$(EXTERNAL_INCLUDES))

# flags always passed to the C compiler
_CFLAGS			:=	$(GLOBAL_CFLAGS) $(CEXTRAFLAGS) $(CWARNFLAGS)

# flags always passed to the C++ compiler
_CPPFLAGS		:=	$(GLOBAL_CFLAGS) $(CPPEXTRAFLAGS) $(CPPWARNFLAGS)

# extra flags to bring in the compiler runtime, since we turn it off via -nodefaultlibs
_RUNTIME_FLAGS		:=	-L$(SDKROOT)/usr/local/lib -lcompiler_rt-static $(LIBBUILTIN_BUILD)

# produce coverage info, but don't add profiling into the built binaries
# this lets us compare coverage from unit tests to the lines built for real targets
ifeq ($(COVERAGE),YES)
_CFLAGS			+=	-ftest-coverage
_CPPFLAGS		+=	-ftest-coverage
endif

ALL_OBJS		:=	$(call TOBUILDDIR,$(ALL_OBJS))

ALL_DEPS		+=	$(ALL_OBJS:%o=%d)

ALL_STATICLIBS		+=	$(PREBUILT_STATICLIBS)

# Make everything depend on the contents of makefiles, as they are sneaky
GLOBAL_SRCDEPS		+=	$(MAKEFILE_LIST)

# Avoid invoking the C++ compiler to link with unless we need it
ifeq ($(WITH_CPLUSPLUS),true)
_LD			:=	$(CPP)
else
_LD			:=	$(CC)
endif

#
# If the build has defined an info symbol file, we can process it to locate TEXT_BASE rather than
# depending on makefiles to generate it.
#
# Note that this must be lazily resolved, as at the time we run this the file probably doesn't exist.
# Note also the hokey '0x' prefix to the value; nm prints as hex.
#
# XXX For now, don't do this unless TEXT_BASE was never defined at the Makefile level.  Later
# we can relax this as platforms stop doing it.
#
ifeq ($(TEXT_BASE),)
ifneq ($(INFO_SYMBOL_FILE),)
SEG1ADDR		=	-seg1addr 0x$(shell nm $(INFO_SYMBOL_FILE) | grep TEXT_BASE | cut -d ' ' -f 1)
endif
endif

# Dump all the interesting variables used to build things with
ifeq (0,1)
$(warning _INCLUDES = $(_INCLUDES))
$(warning _CFLAGS = $(_CFLAGS))
$(warning _CPPFLAGS = $(_CPPFLAGS))
$(warning _RUNTIME_FLAGS = $(_RUNTIME_FLAGS))
$(warning LIBRARY_ALLFLAGS = $(LIBRARY_ALLFLAGS))
$(warning PRODUCT_ALLFLAGS = $(PRODUCT_ALLFLAGS))
$(warning GLOBAL_ALLFLAGS = $(GLOBAL_ALLFLAGS))
$(warning ALL_OBJS = $(ALL_OBJS))
$(warning ALL_DEPS = $(ALL_DEPS))
$(warning ALL_OBJS = $(ALL_OBJS))
endif

# rules for linked binaries
$(PRODUCT_MACHO): $(LINKER_SCRIPT) $(ALL_OBJS) $(ALL_STATICLIBS) ./tools/macho_post_process.py ./tools/macho.py
	@$(MKDIR)
	@echo LD  $@ using $(_RUNTIME_FLAGS)
	$(_v)$(_LD) $(SEG1ADDR) $(GLOBAL_LDFLAGS) $(ALL_OBJS) $(ALL_STATICLIBS) $(ALL_STATICLIBS) $(_RUNTIME_FLAGS) -exported_symbols_list $(LINKER_EXPORTS) -o $@ -Wl,-map,$@.map $(addprefix -Wl,-force_load,$(GLOBAL_LDFORCELIBS))
ifneq ($(TEXT_FOOTPRINT),)
	 @total_size=`$(SIZE) -m $@|sed -n 's/^total \([0-9][0-9]*\)/\1/p'`; if [[ $$total_size -gt $$(($(TEXT_FOOTPRINT))) ]]; then \
		 echo "ERROR: total size is $$(( $$total_size - $(TEXT_FOOTPRINT) )) bytes larger than TEXT_FOOTPRINT, $(TEXT_FOOTPRINT) bytes, for this platform"; \
		 mv $@ $@.OVERSIZED; \
		 exit 1; \
	 fi
endif
	$(_v)./tools/macho_post_process.py --build-tag $(XBS_BUILD_TAG) $@

# rules for objects going into the product build directory
$(BUILDDIR)/%.o: %.c $(PRODUCT_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo CC  $@
	$(_v)$(CC) $(PRODUCT_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@
ifeq ($(ANALYZE),YES)
	@echo ANALYZE  $@
	$(_v)$(ANALYZER) $(PRODUCT_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) $(ANALYZERFLAGS) $<
endif

$(BUILDDIR)/%.o: %.cpp $(PRODUCT_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo C++  $@
	$(_v)$(CPP) $(PRODUCT_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CPPFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(BUILDDIR)/%.o: %.S $(PRODUCT_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo ASM $@
	$(_v)$(CC) $(PRODUCT_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(GLOBAL_ASFLAGS) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

# rules for source files in the builddir
$(BUILDDIR)/%.o: $(BUILDDIR)/%.c $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo CC  $@
	$(_v)$(CC) $(GLOBAL_ALLFLAGS) $(_CFLAGS) -DDEBUG_FILE_HASH=0xbaddadcabadULL $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@
ifeq ($(ANALYZE),YES)
	@echo ANALYZE  $@
	$(_v)$(ANALYZER) $(GLOBAL_ALLFLAGS) $(_CFLAGS) -DDEBUG_FILE_HASH=0xbaddadcabadULL $(_INCLUDES) $(ANALYZERFLAGS) $<
endif

$(BUILDDIR)/%.o: $(BUILDDIR)/%.S $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo ASM $@
	$(_v)$(CC) $(GLOBAL_ALLFLAGS) $(GLOBAL_ASFLAGS) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

# a rule for copying precompiled object files to somewhere they can be found at link time
$(BUILDDIR)/%.o: %.o
	@$(MKDIR)
	@echo CP $@
	$(_v)$(CP) $< $@

#
# Rules for objects going into the libdir
#
# Note that these rules should not be exposed in any other phase, to avoid problems with library 
# objects being spuriously rebuilt.
#
ifeq ($(MAKEPHASE),libraries)
$(LIBOBJDIR)/%.o: %.c $(LIBRARY_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo LIBCC  $@
	$(_v)$(CC) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@
ifeq ($(ANALYZE),YES)
	@echo ANALYZE  $@
	$(_v)$(ANALYZER) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) $(ANALYZERFLAGS) $<
endif

$(LIBOBJDIR)/%.o: %.cpp $(LIBRARY_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo LIBC++  $@
	$(_v)$(CPP) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CPPFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@
ifeq ($(ANALYZE),YES)
	@echo ANALYZE++  $@
	$(_v)$(ANALYZERPP) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CPPFLAGS) $(DEBUG_FILE_HASH) $(_INCLUDES) $(ANALYZERFLAGS) $<
endif

$(LIBOBJDIR)/%.o: %.S $(LIBRARY_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo LIBASM $@
	$(_v)$(CC) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(GLOBAL_ASFLAGS) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

# rules for source files in the libdir
$(LIBOBJDIR)/%.o: $(LIBOBJDIR)/%.c $(LIBRARY_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo LIBCC  $@
	$(_v)$(CC) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) -DDEBUG_FILE_HASH=0xbaddadcabadULL $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@
ifeq ($(ANALYZE),YES)
	@echo ANALYZE  $@
	$(_v)$(ANALYZER) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(_CFLAGS) -DDEBUG_FILE_HASH=0xbaddadcabadULL $(_INCLUDES) $(ANALYZERFLAGS) $<
endif

$(LIBOBJDIR)/%.o: $(LIBOBJDIR)/%.S $(LIBRARY_SRCDEPS) $(GLOBAL_SRCDEPS)
	@$(MKDIR)
	@echo LIBASM $@
	$(_v)$(CC) $(LIBRARY_ALLFLAGS) $(GLOBAL_ALLFLAGS) $(GLOBAL_ASFLAGS) $(_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

# a rule for copying precompiled object files to somewhere they can be found at link time
$(LIBOBJDIR)/%.o: %.o
	@$(MKDIR)
	@echo CP $@
	$(_v)$(CP) $< $@
endif

