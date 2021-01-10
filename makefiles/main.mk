# Copyright (C) 2007-2014 Apple Inc. All rights reserved.
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
# Makefile to build/install a specific executable from the iBoot source tree
#
# The following must be set by the caller:
#
#	CONFIG
#	PRODUCT
#	BUILD
#	XBS_BUILD_TAG
#	OBJROOT
#	SYMROOT
#
# The following may be set by the caller:
#
#	APPLICATION_OPTIONS
#		List of options from application to be added to OPTIONS
#
#	IMAGE_TYPE
#		Installation will generate an image component of the
#		specified type.
#
#	IMAGE_FORMAT
#		Installation will generate images of the format(s) listed.
#		Valid formats are 2 and 3 (Image2, Image3 respectively).
#
#	DFU_IMAGE
#		The generated image will also be installed into the DFU
#		 install location.
#
#	ROM_IMAGE
#		Installation will generate a raw binary.
#
#	INTEL_HEX
#		Installation will generate an Intel Hex format version 
#		of the binary.  ROM_IMAGE_SIZE must be set to a value
#		of the form XXk where XX is your average power of 2
#		and larger than the binary image (this causes the 
#		image to be padded correspondingly).
#
# Environment:
#	TOOLCHAIN_PREFIX
#		Locate tools somewhere other than on the path.
#		
# XXX need something to cause the mach-o to be installed

include makefiles/macros.mk
include makefiles/config.mk

#$(warning Building $(SUB_TARGET)-$(PRODUCT) $(CONFIG) $(BUILD) config)

# Directories
BUILDDIR		:=	$(OBJROOT)/build/$(SUB_TARGET)$(CONFIG)-$(PRODUCT)-$(BUILD)
HDRDIR			:=	$(OBJROOT)/build/include
SYMDIR			:=	$(SYMROOT)/build/$(SUB_TARGET)$(CONFIG)-$(PRODUCT)-$(BUILD)
APPDIR			:=	apps/$(APPLICATION)
LIBDIR			:=	/BAD

# Built products
PRODUCT_BUILD		:=	$(call TOSYMDIR,$(PRODUCT))
PRODUCT_MACHO		:=	$(PRODUCT_BUILD).sys
PRODUCT_DSYM		:=	$(PRODUCT_MACHO).dSYM
PRODUCT_BIN		:=	$(PRODUCT_BUILD).bin
PRODUCT_LIBLIST		:=	$(call TOBUILDDIR,$(PRODUCT)).library_list
PRODUCT_HDRLIST		:=	$(call TOBUILDDIR,$(PRODUCT)).header_list

# Local tools
BHC			:=	$(call TOTOOLSDIR,bhc)
MACHO2BIN		:=	$(call TOTOOLSDIR,Macho2Bin)

# All sources depend on these
GLOBAL_SRCDEPS		:=
PRODUCT_SRCDEPS		:=

# Global headers live here:
GLOBAL_INCLUDES		+=	$(HDRDIR)

# Product-related compilation flags
PRODUCT_ALLFLAGS	:=

# Object files to build, added to by included rules.mk files.
ALL_OBJS		:=

# Deps for object files, computed locally (except for libraries, XXX probably a bug)
ALL_DEPS		:=

# Static libraries we build locally.
LOCALLIBS		:=

# Static libraries built for us by libraries.mk.
COMMONLIBS		:=

# Static libraries someone else already built.
PREBUILT_STATICLIBS	:=

# Static libraries imported externally.
EXTERNAL_STATICLIBS	:=

# All static libraries.
ALL_STATICLIBS		:=

# Additional dependencies for the product.
EXTRA_PRODUCTDEPS	:=

# Dependencies for cleaning
CLEAN			:=

# Source modules list, added to by included rules.mk files.
MODULES			:=

# Module opt-out list, added to by included rules.mk files.
MODULES_ELIDE		:=

# Common library module dependency list, added to by included rules.mk files.
LIBRARY_MODULES		:=

# Options to be passed to all compiled sources in the form <NAME>=<value>.  -D will be added
# automatically.
OPTIONS			:=

# Tag that identifies libraries used/generated by this build
# This is appended by other rules to uniquely identify library builds.
LIBRARY_TAG		:=

# Options passed to the library build
LIBRARY_OPTIONS		:=	BUILD=$(BUILD)

# Compiler options for the library build
LIBRARY_ALLFLAGS	:=

# Default is no synch AES
AES_SYNCH_FIXUP		:=	false

# Expose what we are building to the compiler
# APPLICATION_$(APPLICATION)
# PRODUCT_$(PRODUCT)
# SUB_TARGET_$(SUB_TARGET)
# SUB_TARGET_$(SUB_TARGET)_CONFIG_$(CONFIG)
# CONFIG_$(CONFIG)
APPLICATION_UPPER	:=	$(call UPCASE, $(APPLICATION))
PRODUCT_UPPER		:=	$(call UPCASE, $(PRODUCT))
SUB_TARGET_UPPER	:=	$(call UPCASE, $(SUB_TARGET))
CONFIG_UPPER		:=	$(call UPCASE, $(CONFIG))
OPTIONS			+=	APPLICATION_$(APPLICATION_UPPER)=1
# 8208311
OPTIONS			+=	$(APPLICATION_OPTIONS)
OPTIONS			+=	PRODUCT_$(PRODUCT_UPPER)=1
OPTIONS			+=	SUB_TARGET_$(SUB_TARGET_UPPER)=1
OPTIONS			+=	SUB_TARGET_STRING=\"$(SUB_TARGET)\"
OPTIONS			+=	SUB_TARGET_UPPER_STRING=\"$(SUB_TARGET_UPPER)\"
ifneq ($(CONFIG),)
OPTIONS			+=	SUB_TARGET_$(SUB_TARGET_UPPER)_CONFIG_$(CONFIG_UPPER)=1
OPTIONS			+=	CONFIG_$(CONFIG_UPPER)=1
endif
OPTIONS			+=	$(BUILD)_BUILD=1
ifneq ($(CRYPTO_HASH),sha1)
CRYPTO_HASH_UPPER	:=	$(call UPCASE, $(subst -,_,$(CRYPTO_HASH)))
OPTIONS			+=	WITH_$(CRYPTO_HASH_UPPER)=1
endif

# Also define RELEASE_BUILD for the ROMRELEASE build flavor so that everything
# uses BUILD_RELEASE doesn't have to be touched to use ROMRELEASE also.
ifeq ($(BUILD),ROMRELEASE)
OPTIONS			+=	RELEASE_BUILD=1
endif

# export hacks as options
OPTIONS			+=	$(addprefix HACK_,$(HACKS))

# Identifying strings
OPTIONS			+=	BUILD_STYLE=\"$(BUILD)\"
OPTIONS			+=	CONFIG_PROGNAME_STRING=\"$(PRODUCT)\"
OPTIONS			+=	CONFIG_BOARD_STRING=\"$(SUB_TARGET)$(CONFIG)\"

# can't OPTIONS this as it changes for every non-XBS build
GLOBAL_CFLAGS		+=	-DXBS_BUILD_TAG=\"$(XBS_BUILD_TAG)\"
GLOBAL_ASFLAGS		+=	-DXBS_BUILD_TAG=\"$(XBS_BUILD_TAG)\"

# deal with the options.h file which is dynamically built from the OPTIONS make variable
OPTIONS_HEADER_FILE	:=	$(call TOSYMDIR,options.h)
OPTIONS_HEADER_FILE_TEMP :=	$(call TOSYMDIR,options.h.temp)
PRODUCT_ALLFLAGS	+=	-include $(OPTIONS_HEADER_FILE)
PRODUCT_SRCDEPS		+=	$(OPTIONS_HEADER_FILE) 

# Read the application makefile, which may add modules
include $(APPDIR)/application.mk

# Application may adjust TARGET
TARGET			:=	$(SUB_TARGET)

# Read the target configuration, which may add modules
# Allow the config file path to be overridden to support 
# configs outside of the application's regular location.
CONFIG_FILE		?=	$(APPDIR)/config/$(SUB_TARGET)-config.mk
include $(CONFIG_FILE)

# The target configuration may elect to abort the build of this product
ifeq ($(ABORT),true)
$(warning Build of $(SUB_TARGET)-$(PRODUCT) $(CONFIG) $(BUILD) aborted by configuration)
build:
install:
clean:
library_list:
else

# Ensure TARGET exists
TARGET_UPPER		:=	$(call UPCASE, $(TARGET))
OPTIONS			+=	TARGET_$(TARGET_UPPER)=1
ifneq ($(CONFIG),)
OPTIONS			+=	TARGET_$(TARGET_UPPER)_CONFIG_$(CONFIG_UPPER)=1
endif

# Ensure SUB_PLATFORM exists
SUB_PLATFORM		?=	$(PLATFORM)
SUB_PLATFORM_UPPER	:=	$(call UPCASE, $(SUB_PLATFORM))
OPTIONS			+=	SUB_PLATFORM_$(SUB_PLATFORM_UPPER)=1

# Create PLATFORM_NAME_STRING, PLATFORM_NAME_UNQUOTED
OPTIONS			+=	PLATFORM_NAME_STRING=\"$(SUB_PLATFORM)\"
OPTIONS			+=	PLATFORM_NAME_UNQUOTED=$(SUB_PLATFORM)

# Read module configurations
MODULES_ALREADY_INCLUDED:=
MODULES_VAR		:= 	MODULES
MODULE_MAKEFILE		:=	rules.mk
include makefiles/main-nested-module.mk

# Now that we have processed modules, we know where common library objects will be located.
LIBRARY_UNIQUE		:=	$(subst ${empty} ${empty},-,$(sort ${LIBRARY_TAG}))
LIBDIR			:=	$(OBJROOT)/build/lib-$(LIBRARY_UNIQUE)-$(BUILD)

# Do library modules
MODULES_ALREADY_INCLUDED:=
MODULES_VAR		:=	LIBRARY_MODULES
MODULE_MAKEFILE		:=	library.mk
include makefiles/main-nested-module.mk
LIBRARY_MODULE_LIST	:=	$(sort $(MODULES_ALREADY_INCLUDED))

# Compute the static library set based on the build target list.
# We have two sets of static libraries; common libraries are built by libraries.mk,
# whilst local libraries are built here.
# Note local libraries before common libraries to permit local->common depdendencies.
# (We could list both sets twice...)
COMMONLIB_TARGETVARS	:=	$(addsuffix _BUILD,$(COMMONLIBS))
COMMONLIB_LIBS		:=	$(foreach i,$(COMMONLIB_TARGETVARS),$(value $i))
LOCALLIB_TARGETVARS	:=	$(addsuffix _BUILD,$(LOCALLIBS))
LOCALLIB_LIBS		:=	$(foreach i,$(LOCALLIB_TARGETVARS),$(value $i))
ALL_STATICLIBS		+=	$(LOCALLIB_LIBS) $(COMMONLIB_LIBS) $(EXTERNAL_STATICLIBS)

# At the very least, installing requires the product be built
INSTALL_PREREQS		=	$(PRODUCT)

# If an image type is specified, enable the image format targets
ifneq ($(IMAGE_TYPE),)
    ifeq ($(IMAGE_FORMAT),img3)
        INSTALL_PREREQS		+=	install-image3
    else
        INSTALL_PREREQS		+=	install-image4
    endif	
endif

# If a DFU image is being handled, enable the image format targets
# In Image4, this collides with the DSTROOT products of EmbeddedFirmware
# due to not having a unique extension. It is already installed under
# /image4, which EmbeddedFirmware searches, so we can skip this duplicate.
ifeq ($(DFU_IMAGE),true)
    ifeq ($(IMAGE_FORMAT),img3)
        INSTALL_PREREQS		+=	install-dfuimage3
    endif	
endif

# If we are generating a plain binary, enable the target
ifeq ($(BINARY_IMAGE),true)
INSTALL_PREREQS		+=	install-binary-image
endif

# If a Secure ROM image is being handled, enable the target
ifeq ($(ROM_IMAGE),true)
INSTALL_PREREQS		+=	install-romimage
endif

# Intel Hex output, requires ROM_IMAGE_SIZE be set to pad the image
ifeq ($(INTEL_HEX),true)
INSTALL_PREREQS		+=	install-intelhex
endif

# If a header file is being generated, enable the target
ifneq ($(IMAGE_WITH_HEADER),)
INSTALL_PREREQS		+= 	install-headerfile
endif

# If additional headers should be installed, enable the target that installs them
ifneq ($(INSTALL_HEADERS),)
INSTALL_PREREQS		+=	install-additional-headers
endif

# Default to no optional arguments to image3maker
IMAGE3_OPTARGS		:=

# If AES synch support is enabled, we need the header to be found.
# This isn't generally safe as there are headers we own that get installed
# there and we end up using a stale version.
ifneq ($(AES_SYNCH_FIXUP),false)
GLOBAL_INCLUDES		+=	/usr/local/standalone/firmware
endif

build:		$(PRODUCT)

library_list:	$(PRODUCT_HDRLIST) $(PRODUCT_LIBLIST)

$(PRODUCT): 	$(PRODUCT_BIN) $(PRODUCT_BUILD).size

install:	$(INSTALL_PREREQS)

install-image3: PRODUCT_IMG		=	$(PRODUCT_BUILD).img3
install-image3: $(PRODUCT)
	@echo installing Image3 object \'$(IMAGE_TYPE)\' from $(PRODUCT_BIN)
	$(_v)$(IMAGE3MAKER) \
		--create \
		--data $(PRODUCT_BIN) \
		--version $(XBS_BUILD_TAG) \
		--type $(IMAGE_TYPE) \
		--imagefile $(PRODUCT_IMG) \
		$(IMAGE3_OPTARGS) 
	@mkdir -p $(INSTALL_DIR)/image3
	@install -C -m 644 $(PRODUCT_IMG) $(INSTALL_DIR)image3/$(INSTALL_NAME).img3

install-dfuimage3: PRODUCT_IMG		=	$(PRODUCT_BUILD).img3
install-dfuimage3: $(PRODUCT) install-image3
	@echo installing DFU object $(PRODUCT_IMG) as $(INSTALL_NAME).img3
ifneq ($(MAX_DFU_SIZE),)
	@if test "$(PRODUCT)" = "iBSS"; then \
		tools/check_product_size $(PRODUCT_IMG) $(MAX_DFU_SIZE) "MAX_DFU_SIZE" ; \
	fi
endif
	@mkdir -p $(INSTALL_DIR)/dfu
	@install -C -m 644 $(PRODUCT_IMG) $(INSTALL_DIR)dfu/$(INSTALL_NAME).img3

install-image4: PRODUCT_IMG		=	$(PRODUCT_BUILD).im4p
install-image4: $(PRODUCT)
	@echo installing Image4 object \'$(IMAGE_TYPE)\' from $(PRODUCT_BIN)
	$(_v)$(IMG4PAYLOAD) \
		-i $(PRODUCT_BIN) \
		-v $(XBS_BUILD_TAG) \
		-t $(IMAGE_TYPE) \
		-o $(PRODUCT_IMG)
ifneq ($(MAX_DFU_SIZE),)
	@if test "$(PRODUCT)" = "iBSS" -o "$(PRODUCT)" = "LLB"; then \
		tools/check_product_size $(PRODUCT_IMG) $(MAX_DFU_SIZE) "MAX_DFU_SIZE"; \
	fi
endif
	@mkdir -p $(INSTALL_DIR)/image4
	@install -C -m 644 $(PRODUCT_IMG) $(INSTALL_DIR)image4/$(INSTALL_NAME).im4p

install-binary-image: $(PRODUCT)
	@echo installing binary object $(PRODUCT_BIN) as $(INSTALL_NAME).bin
	@mkdir -p $(INSTALL_DIR)
	@install -C -m 644 $(PRODUCT_BIN) $(INSTALL_DIR)/$(INSTALL_NAME).bin

install-romimage: $(PRODUCT)
	@echo installing binary object $(PRODUCT_BIN) as $(INSTALL_NAME).bin
	@mkdir -p $(INSTALL_DIR)/SecureROM
	@install -C -m 644 $(PRODUCT_BIN) $(INSTALL_DIR)SecureROM/$(INSTALL_NAME).bin

install-intelhex: $(PRODUCT)
	@echo installing IntelHex file $(INSTALL_DIR)SecureROM/$(INSTALL_NAME).hex
	@$(BHC) -h $(PRODUCT_BIN) $(INSTALL_DIR)SecureROM/$(INSTALL_NAME).hex \
		$(ROM_IMAGE_SIZE) 2>&1 > /dev/null

install-headerfile: PRODUCT_HEADER	=	$(PRODUCT_BUILD).h
install-headerfile: $(PRODUCT)
	@echo installing binary object $(PRODUCT_BIN) in headerfile $(INSTALL_NAME) using $(IMAGE_WITH_HEADER)
	@mkdir -p $(INSTALL_DIR)
	@NM=$(NM) \
	 OBJFILE=$(PRODUCT_MACHO) \
	 BINFILE=$(PRODUCT_BIN) \
	 XBS_BUILD_TAG=$(XBS_BUILD_TAG) \
	 $(AWK) -f $(SRCROOT)/makefiles/format-headerfile.awk < $(IMAGE_WITH_HEADER) > $(PRODUCT_HEADER)
	@install -C -m 644 $(PRODUCT_HEADER) $(INSTALL_DIR)$(INSTALL_NAME)

# Note that we can't use install -C here as it can race with another
# install's unlink of the destination due to the same header(s) being
# installed by more than one invocation simultaneously. Ssen notes
# in rdar://8317733 that -S doesn't fix this.
install-additional-headers:	$(INSTALL_HEADERS)
	@echo installing additional headers: $(INSTALL_HEADERS)
	@mkdir -p $(INSTALL_DIR)
	@test -w $(INSTALL_DIR) || (echo "ERROR: I just created $(INSTALL_DIR) but now I cannot write files to it." && exit 1)
	@install -m 644 $(INSTALL_HEADERS) $(INSTALL_DIR)

# someone can request additional system dependencies
$(PRODUCT): $(EXTRA_PRODUCTDEPS)

# Produce the prelinked binary image.
#
# Configuration may supply the following options:
#
# BIN_IMAGE_PREFIX
#	A binary image to be prepended to the image.
#
# BIN_INCLUDES_BSS
#	The image is padded to include the initialised BSS.
#
# BIN_ROUND_UP
#	The output image is padded to a multiple of the supplied value.
#
# BIN_FIXUP
#	The output image is passed to this command once fully assembled.
#
BIN_ROUND_UP	?= 1
ifneq ($(BIN_ROUND_UP),1)
ifeq ($(BIN_INCLUDES_BSS),true)
$(error Cannot specify both BIN_ROUND_UP and BIN_INCLUDES_BSS)
endif
endif

$(PRODUCT_BIN): $(PRODUCT_BUILD).stripped $(PRODUCT_DSYM) $(BIN_IMAGE_PREFIX)
	@$(SIZE) $<
	@echo BIN $@
ifneq ($(BIN_IMAGE_PREFIX),)
	$(_v)dd if=$(BIN_IMAGE_PREFIX) of=$@
else
	$(_v)dd if=/dev/zero of=$@ count=0
endif
	$(_v)dd if=$< ibs=1 skip=$(shell $(OTOOL) -lv $< | grep -m1 '^  fileoff ' | cut -c 11-) \
		obs=$(BIN_ROUND_UP) conv=osync >> $@
ifeq ($(BIN_INCLUDES_BSS),true)
	@echo PAD $@
	$(_v)dd if=/dev/zero \
	    of=$@ \
	    bs=1 \
	    seek=`stat -f %z $@` \
	    count=`expr \`$(SIZE) $< | tail -1 | cut -f 5\` - \`stat -f %z $@\``
endif
ifneq ($(BIN_FIXUP),)
	$(_v)$(BIN_FIXUP) $@
endif
ifneq ($(AES_SYNCH_FIXUP),false)
	$(AES_SYNCH_FIXUP) $(PRODUCT_BUILD)
endif

$(PRODUCT_DSYM): $(PRODUCT_MACHO) tools/lldb_init_iboot.py tools/lldb_os_iboot.py
	@echo dSYM $@
	$(_v)$(DSYMUTIL) --out=$@ $<
	@mkdir -p $(PRODUCT_DSYM)/Contents/Resources/Python/
	@install -C -m 644 tools/lldb_init_iboot.py $(PRODUCT_DSYM)/Contents/Resources/Python/$(notdir $(PRODUCT_BUILD))-disabled.py
	@install -C -m 644 tools/lldb_os_iboot.py $(PRODUCT_DSYM)/Contents/Resources/Python/lldb_os_iboot.py

$(PRODUCT_BUILD).stripped: $(PRODUCT_MACHO)
	@echo STRIP $@
	$(_v)$(STRIP) -o $@ $<

$(PRODUCT_BUILD).size: $(PRODUCT_MACHO)
	@echo SIZE $@
	$(_v)$(SIZE) $(ALL_OBJS) > $@

$(PRODUCT_HDRLIST):	$(MAKEFILE_LIST)
	$(_v)$(MKDIR)
	$(_v)rm -f $(PRODUCT_HDRLIST)
	$(_v)touch $(PRODUCT_HDRLIST)
	$(_v)for i in $(GLOBAL_HEADERS) ; do \
		echo "$(HDRDIR)@$$i" >> $(PRODUCT_HDRLIST) ; \
	done

ENCODED_LIBRARY_OPTIONS	:=	$(subst $(empty) ,~,$(addprefix -D,$(strip $(LIBRARY_OPTIONS))))
ENCODED_LIBRARY_FLAGS	:=	$(subst $(empty) ,~,$(strip $(GLOBAL_ALLFLAGS) $(LIBRARY_ALLFLAGS)))
$(PRODUCT_LIBLIST):	$(MAKEFILE_LIST)
	$(_v)$(MKDIR)
	$(_v)rm -f $(PRODUCT_LIBLIST)
	$(_v)touch $(PRODUCT_LIBLIST)
	$(_v)for i in $(LIBRARY_MODULE_LIST) ; do \
		echo "$$i~$(LIBRARY_UNIQUE)-$(BUILD)~$(ENCODED_LIBRARY_OPTIONS)~$(ENCODED_LIBRARY_FLAGS)" \
			>> $(PRODUCT_LIBLIST) ; \
	 done
	$(_v)$(CHECK_LIBLIST) $(PRODUCT_LIBLIST)

# XXX build static libraries the 'libraries' pass didn't get
# XXX need to differentiate these...
$(LOCALLIB_LIBS): objs = $(value $(addsuffix _OBJS,$(basename $(notdir $@))))
$(LOCALLIB_LIBS):
	@$(MKDIR)
	@echo AR  $@
	$(_v)$(AR) -crS $@ $(objs)
	$(_v)$(RANLIB) $@

# build the options.h file from the OPTIONS variable
$(OPTIONS_HEADER_FILE): $(MAKEFILE_LIST)
	@$(MKDIR)
	@rm -f $(OPTIONS_HEADER_FILE_TEMP); \
	(for o in $(OPTIONS); do \
		echo "#define $$o"; \
	done) | sort | sed s/=/\ / >> $(OPTIONS_HEADER_FILE_TEMP); \
	if [ -f "$(OPTIONS_HEADER_FILE)" ]; then \
		if cmp "$(OPTIONS_HEADER_FILE_TEMP)" "$(OPTIONS_HEADER_FILE)"; then \
			rm -f $(OPTIONS_HEADER_FILE_TEMP) ; \
		else \
			echo UPD $@ ; \
			mv $(OPTIONS_HEADER_FILE_TEMP) $(OPTIONS_HEADER_FILE) ; \
		fi \
	else \
		echo GEN $@ ; \
		mv $(OPTIONS_HEADER_FILE_TEMP) $(OPTIONS_HEADER_FILE) ; \
	fi \

# Now we can define object build rules
include makefiles/build.mk

clean:
	$(_v)rm -rf $(BUILDDIR)

# Defining CLEAN_INCLUDE_TEST turns on this snippet which will cause an 
# error on 'make clean' if any directory mentioned in GLOBAL_INCLUDES does not exist
ifneq ($(CLEAN_INCLUDE_TEST),)
clean-include-test:	$(filter-out $(BUILDDIR)/include,$(patsubst -I%,%,$(GLOBAL_INCLUDES)))
clean:	clean-include-test
endif

# Empty rule for .d files
%.d:
%.Td:

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(ALL_DEPS)
endif

# !ABORT
endif