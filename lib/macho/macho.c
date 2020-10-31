/*
 * Copyright (C) 2007-2011 Apple Inc. All rights reserved.
 * Copyright (C) 1998-2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
/*
 *  macho.c - Functions for decoding a Mach-o Kernel.
 *
 *  Copyright (c) 1998-2006 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include <debug.h>
#include <lib/macho.h>
#include <platform.h>

#include "macho_header.h"

static addr_t	gRelocBase = (addr_t)NULL;
static addr_t	gLinkeditBase = (addr_t)NULL;
static uint32_t	gLinkeditOffset = 0;
static uint32_t	gkalsr_debug = DEBUG_SPEW;

static bool ProcessSymbols(addr_t cmdBase, size_t slide);

static bool ProcessRelocations(addr_t cmdBase, size_t slide);

static segment_command_t *firstsegfromheader(mach_header_t *header);

static segment_command_t *nextsegfromheader(mach_header_t *header, segment_command_t *seg);

static section_t *firstsect(segment_command_t *sgp);

static section_t *nextsect(segment_command_t *sgp, section_t *sp);

static bool	macho_load_segment(addr_t imageAddr,
				   size_t imageSize,
				   addr_t segmentAddr, 
				   addr_t loadAddr,
				   addr_t *virtualBase, 
				   addr_t *virtualEnd,
				   size_t slide);

/*
 * Check whether something might be a Mach-O object.
 */
bool
macho_valid(addr_t addr)
{
	return(((mach_header_t *)addr)->magic == MH_MAGIC);
}

bool
macho_slideable(addr_t addr)
{
	mach_header_t	*mH;

	mH = (mach_header_t *)addr;

	if (!macho_valid(addr))
		return false;

	return (mH->flags & MH_PIE) != 0;
}

/*
 * Process a mach-o object.
 */
bool
macho_load(
	addr_t imageAddr, 
	size_t imageSize, 
	addr_t loadAddr, 
	addr_t *virtualBase, 
	addr_t *virtualEnd, 
	addr_t *entryPoint,
	size_t slide)
{
	mach_header_t	*mH;
	uint32_t	index;
	addr_t		cmdBase;

	mH = (mach_header_t *)imageAddr;
	cmdBase = (addr_t)(mH + 1);

	*virtualBase = 0;
	*virtualEnd  = 0;

	gRelocBase = (addr_t)NULL;
	gLinkeditBase = (addr_t)NULL;
	gLinkeditOffset = 0;

	dprintf(DEBUG_INFO, "mach-o: loading %p->%p\n", (void *)imageAddr, (void *)loadAddr);
	dprintf(DEBUG_INFO, "  magic      0x%x\n", mH->magic);
	dprintf(DEBUG_INFO, "  cputype    0x%x\n", mH->cputype);
	dprintf(DEBUG_INFO, "  cpusubtype 0x%x\n", mH->cpusubtype);
	dprintf(DEBUG_INFO, "  filetype   0x%x\n", mH->filetype);
	dprintf(DEBUG_INFO, "  ncmds      0x%x\n", mH->ncmds);
	dprintf(DEBUG_INFO, "  sizeofcmds 0x%x\n", mH->sizeofcmds);
	dprintf(DEBUG_INFO, "  flags      0x%x\n", mH->flags);

	for (index = 0; index < mH->ncmds; index++) {
		switch(((struct load_command *)cmdBase)->cmd) {
#if defined(__arm__) || defined(__arm64__)
		case LC_SEGMENT_ARM:
			dprintf(gkalsr_debug, "cmd LC_SEGMENT_ARM:\n");
#else
# error no support for this architecture
#endif		
			if (!macho_load_segment(imageAddr, imageSize, cmdBase, loadAddr, virtualBase, virtualEnd, slide))
				return false;
			break;

		case LC_UNIXTHREAD:
			dprintf(gkalsr_debug, "cmd LC_UNIXTHREAD:\n");
			// plus 8 for flavor and count fields
#if defined(__arm__) || defined(__arm64__)
			*entryPoint = ((arm_thread_state_t *)(cmdBase + sizeof(struct thread_command) + 8))->pc;
#else
# error no support for this architecture
#endif
			break;


		case LC_SYMTAB:
			dprintf(gkalsr_debug, "cmd LC_SYMTAB:\n");
			if (slide == 0) {
                		break;
           		 }
			if (!ProcessSymbols(cmdBase, slide))
				return false;
			break;

		case LC_DYSYMTAB:
			dprintf(gkalsr_debug, "cmd LC_DYSYMTAB:\n");
			if (slide == 0) {
				break;
			}
			if (!ProcessRelocations(cmdBase, slide))
				return false;
			break;
		}
		cmdBase += ((struct load_command *)cmdBase)->cmdsize;
	}

	if (slide != 0) {
		*entryPoint = *entryPoint + slide;
		*virtualBase = *virtualBase + slide;
		*virtualEnd = *virtualEnd + slide;
	}

	dprintf(gkalsr_debug, "macho_load: *entryPoint %p *virtualBase %p *virtualEnd %p\n",
		(void *)*entryPoint, (void *)*virtualBase, (void *)*virtualEnd);

	/*
	 * Clean everything we have just written out to memory.
	 *
	 * XXX this should really not be necessary...
	 */
	platform_cache_operation(CACHE_CLEAN, 0, 0);
	return true;
}


/*
 * Process a Mach-o LC_SEGMENT/LC_SEGMENT_64 command.
 */
static bool
macho_load_segment(
	addr_t imageAddr, 
	size_t imageSize, 
	addr_t segmentAddr, 
	addr_t loadAddr, 
	addr_t *virtualBase, 
	addr_t *virtualEnd,
	size_t slide)
{
	segment_command_t	*cmd;
	addr_t			segmentSourceAddress;
	addr_t			segmentDestinationAddress;
	addr_t			segmentVirtualEnd;
	size_t			segmentLoadSize;
	size_t			segmentFillSize;

	cmd = (segment_command_t *)segmentAddr;

	/*
	 * We assume that the first segment in the object
	 * is lowest in memory, and round it down to the 
	 * nearest megabyte.
	 */
	if (0 == *virtualBase) {
#if defined(__LP64__)
		*virtualBase = cmd->vmaddr & 0xfffffffffff00000ULL;
#else
		*virtualBase = cmd->vmaddr & 0xfff00000UL;
#endif		
	} else {
		/* sanity check the segment */
		if (cmd->vmaddr < *virtualBase)
			return false;
		if ((cmd->vmaddr + cmd->vmsize) < *virtualBase)
			return false;

		/* 
		 * XXX should add checks for the file*
		 * members and sizes 
		 */
	}

	/*
	 * Mostly ignore the __PAGEZERO segment.
	 */
	if (!strcmp(cmd->segname, "__PAGEZERO")) 
		return true;

	/*
	 * Precalculate various things we are going to
	 * need to refer to as we copy and fill the segment.
	 */
	segmentSourceAddress      = imageAddr + cmd->fileoff;
	segmentDestinationAddress = loadAddr + (cmd->vmaddr - *virtualBase);
	segmentLoadSize           = __min(cmd->vmsize, cmd->filesize);
	segmentFillSize           = __min(cmd->vmsize, cmd->vmsize - cmd->filesize);
	segmentVirtualEnd         = cmd->vmaddr + cmd->vmsize;

	dprintf(DEBUG_INFO, 
		"  %-16.16s  %p/%p/%p 0x%zx+0x%zx\n",
		cmd->segname,
		(void *)segmentDestinationAddress,
		(void *)cmd->vmaddr,
		(void *)cmd->vmaddr + slide,
		segmentLoadSize, 
		segmentFillSize);

	/*
	 * Copy the portion of the segment contained in
	 * the object to its location in the loaded area.
	 */
	if (segmentLoadSize) {
		addr_t segmentEnd = segmentDestinationAddress + segmentLoadSize;
		/* no integer overflow */
		RELEASE_ASSERT(segmentEnd > segmentDestinationAddress);
		/* don't overwrite the image we're loading from; assumes we always load
		 * to an address below the image we're loading from
		 */
		RELEASE_ASSERT((segmentEnd <= imageAddr) || (segmentEnd >= imageAddr + imageSize));

		memcpy((void *)segmentDestinationAddress,
		       (void *)segmentSourceAddress,
		       segmentLoadSize);
	}

	/*
	 * Zero the remaining portion of the segment.
	 */
	if (segmentFillSize) {
		addr_t fillEnd = segmentDestinationAddress + segmentLoadSize + segmentFillSize;
		/* no integer overflow */
		RELEASE_ASSERT(fillEnd > segmentDestinationAddress);
		/* don't overwrite the image we're loading from; assumes we always load
		 * to an address below the image we're loading from
		 */
		RELEASE_ASSERT((fillEnd <= imageAddr) || (fillEnd >= imageAddr + imageSize));

		memset((void *)(segmentDestinationAddress + segmentLoadSize),
		       0,
		       segmentFillSize);
	}
			
	/*
	 * Adjust the occupied length if we have extended it.
	 */
	if (segmentVirtualEnd > *virtualEnd)
		*virtualEnd = segmentVirtualEnd;

	if (slide != 0) {
		// Remember reloc and linkedit base
		if (gRelocBase == (addr_t)NULL) {
			gRelocBase = (addr_t)segmentDestinationAddress;
			dprintf(gkalsr_debug,"KASLR: gRelocBase = %p\n", (void *)gRelocBase);
		}

		if ((gLinkeditBase == (addr_t)NULL) && (!strcmp(cmd->segname, "__LINKEDIT"))) {
			gLinkeditBase = (addr_t)segmentDestinationAddress;
			gLinkeditOffset = (uint32_t)(cmd->fileoff);
			dprintf(gkalsr_debug,"KASR: gLinkeditBase = %p gLinkeditOffset = 0x%x\n", (void *)gLinkeditBase, gLinkeditOffset);
		}

		if (strcmp(cmd->segname, "__TEXT") == 0) {
			segment_command_t *seg;
			section_t *sec;
			mach_header_t *mh = (mach_header_t *)segmentDestinationAddress;
 
			dprintf(gkalsr_debug, "Applying KASLR slide to _mh_execute_header contents:\n");
			for (seg = firstsegfromheader(mh); seg != NULL; seg = nextsegfromheader(mh, seg)) {
 
				dprintf(gkalsr_debug," segment %s: %p -> %p\n", seg->segname,
				                   (void *)seg->vmaddr, (void *)(seg->vmaddr + slide));
				seg->vmaddr += slide;
 
				for (sec = firstsect(seg); sec != NULL; sec = nextsect(seg, sec)) {
					dprintf(gkalsr_debug," -> section %s: %p -> %p\n", sec->sectname,
					                   (void *)sec->addr, (void *)(sec->addr + slide));
					sec->addr += slide;
				}
			}
		}
 	}

	/*
	 * Parse segment sections
	 */
	{
		segment_command_t	*seg;
		section_t		*sec;
		addr_t			sectionDestinationAddress;
 
		seg = (segment_command_t *)segmentAddr; 
		for (sec = firstsect(seg); sec != NULL; sec = nextsect(seg, sec)) {
			sectionDestinationAddress = loadAddr + (sec->addr - *virtualBase);

			dprintf(gkalsr_debug," -> section %s: %p/%p 0x%llx\n",
			                   sec->sectname, (void *)sectionDestinationAddress, (void *)sec->addr, (uint64_t)sec->size);
			/* Slide pointers in __nl_symbol_ptr section */
			if (((sec->flags & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS)) {
				void **nl_symbols_ptr;
				dprintf(gkalsr_debug,"Slide S_NON_LAZY_SYMBOL_POINTERS \n");
				for (nl_symbols_ptr = (void **)sectionDestinationAddress;
				     nl_symbols_ptr < (void **)(sectionDestinationAddress+sec->size);
				     nl_symbols_ptr++) {
					if (nl_symbols_ptr == (void **)sectionDestinationAddress)
						dprintf(gkalsr_debug," %p : %p -> %p\n",
						                   nl_symbols_ptr, *nl_symbols_ptr, *nl_symbols_ptr + slide);
					*nl_symbols_ptr = *nl_symbols_ptr + slide;
				}
			}
		}
	}

	return true;
}


/*
 * Return the first segment_command in the header.
 */
segment_command_t *
firstsegfromheader(mach_header_t *header)
{
	uint32_t i = 0;
	segment_command_t *sgp = (segment_command_t *) ((uintptr_t)header + sizeof(*header));

	for (i = 0; i < header->ncmds; i++){
#if defined(__arm__) || defined(__arm64__)
		if (sgp->cmd == LC_SEGMENT_ARM)
			return sgp;
#else
# error no support for this architecture
#endif
		sgp = (segment_command_t *)((uintptr_t)sgp + sgp->cmdsize);
	}
	return (segment_command_t *)NULL;
}

/*
 * This routine operates against any kernel mach segment_command structure
 * pointer and the provided kernel header, to obtain the sequentially next
 * segment_command structure in that header.
 */
segment_command_t *
nextsegfromheader(mach_header_t *header,
		  segment_command_t *seg)
{
	uint32_t i = 0;
	segment_command_t *sgp = (segment_command_t *) ((uintptr_t)header + sizeof(*header));

	/* Find the index of the passed-in segment */
	for (i = 0; sgp != seg && i < header->ncmds; i++) {
		sgp = (segment_command_t *)((uintptr_t)sgp + sgp->cmdsize);
	}

	/* Increment to the next load command */
	i++;
	sgp = (segment_command_t *)((uintptr_t)sgp + sgp->cmdsize);

	/* Return the next segment command, if any */
	for (; i < header->ncmds; i++) {
#if defined(__arm__) || defined(__arm64__)
		if (sgp->cmd == LC_SEGMENT_ARM)
			return sgp;
#else
# error no support for this architecture
#endif

		sgp = (segment_command_t *)((uintptr_t)sgp + sgp->cmdsize);
	}

	return (segment_command_t *)NULL;
}

/*
 * This routine can operate against any kernel segment_command structure to
 * return the first kernel section immediately following that structure.  If
 * there are no sections associated with the segment_command structure, it
 * returns NULL.
 */
section_t *
firstsect(segment_command_t *sgp)
{
	if (!sgp || sgp->nsects == 0)
		return (section_t *)NULL;

	return (section_t *)(sgp + 1);
}

/*
 * This routine can operate against any kernel segment_command structure and
 * kernel section to return the next consecutive  kernel section immediately
 * following the kernel section provided.  If there are no sections following
 * the provided section, it returns NULL.
 */
section_t *
nextsect(segment_command_t *sgp, section_t *sp)
{
	section_t *fsp = firstsect(sgp);

	if (((uintptr_t)(sp - fsp) + 1) >= sgp->nsects)
		return (section_t *)NULL;

	return sp + 1;
}

bool
ProcessSymbols(addr_t cmdBase, size_t slide)
{
	struct symtab_command *symtab;
#if defined(__LP64__)
	struct nlist_64 *sym;
#else
	struct nlist *sym;
#endif
	uint32_t i;
	uint32_t cnt = 0;

	symtab = (struct symtab_command *)cmdBase;
	dprintf(gkalsr_debug, "Symbols:\n");
	dprintf(gkalsr_debug, "nsyms: %d, symoff: 0x%x\n", symtab->nsyms, symtab->symoff);

	if (symtab->nsyms == 0) {
		dprintf(gkalsr_debug, "No symbols to relocate\n");
		return true;
	}

#if defined(__LP64__)
	sym = (struct nlist_64 *)(gLinkeditBase + symtab->symoff - gLinkeditOffset);
#else
	sym = (struct nlist *)(gLinkeditBase + symtab->symoff - gLinkeditOffset);
#endif

	for (i = 0; i < symtab->nsyms; i++) {
		if (sym[i].n_type & N_STAB)
			continue;
		sym[i].n_value += slide;
		cnt++;
	}
	dprintf(gkalsr_debug, "KASLR: Relocated %d symbols\n", cnt);
	return true;
}

bool
ProcessRelocations(addr_t cmdBase, size_t slide)
{
	struct dysymtab_command *dysymtab;

	dysymtab = (struct dysymtab_command *)cmdBase;  

	dprintf(gkalsr_debug, "Symbols:\n");
	dprintf(gkalsr_debug, "local:      %6d @ 0x%x\n", dysymtab->nlocalsym, dysymtab->ilocalsym);
	dprintf(gkalsr_debug, "external:   %6d @ 0x%x\n", dysymtab->nextdefsym, dysymtab->iextdefsym);
	dprintf(gkalsr_debug, "undefined:  %6d @ 0x%x\n", dysymtab->nundefsym, dysymtab->iundefsym);
	dprintf(gkalsr_debug, "indirect:   %6d @ 0x%x\n", dysymtab->nindirectsyms, dysymtab->indirectsymoff);

	dprintf(gkalsr_debug, "Relocation entries:\n");
	dprintf(gkalsr_debug, "local:      %6d @ 0x%x\n", dysymtab->nlocrel, dysymtab->locreloff);
	dprintf(gkalsr_debug, "external:   %6d @ 0x%x\n", dysymtab->nextrel, dysymtab->extreloff);

	if (dysymtab->nlocrel != 0) {
		struct relocation_info *relocStart;
		struct relocation_info *relocEnd;
		struct relocation_info *reloc;

		if (gRelocBase == (addr_t)NULL) {
			dprintf(gkalsr_debug, "KASLR: no writeable segment for relocations found\n");
			return false;
		}

		if (gLinkeditBase == (addr_t)NULL) {
			dprintf(gkalsr_debug, "KASLR: No __LINKEDIT segment found\n");
			return false;
		}

		relocStart = (struct relocation_info *)(gLinkeditBase + dysymtab->locreloff - gLinkeditOffset);
		relocEnd = &relocStart[dysymtab->nlocrel];

		dprintf(gkalsr_debug, "KASLR: processing local relocations...\n");
		dprintf(gkalsr_debug, "relocStart = %p, relocEnd = %p\n", relocStart, relocEnd);

		for (reloc = relocStart; reloc < relocEnd; reloc++) {
			if ((1 << reloc->r_length) != sizeof(void *)) {
				/* 2 to the r_length should be the size of a pointer for all relocations */
				dprintf(gkalsr_debug, "bad local relocation length (%d)\n", reloc->r_length);
				return false;
			}
			if (reloc->r_type != ARM_RELOC_VANILLA) {
				dprintf(gkalsr_debug, "unknown local relocation type (%d)\n", reloc->r_type);
				return false;
			}
			if (reloc->r_pcrel != 0) {
				dprintf(gkalsr_debug, "bad local relocation r_pcrel");
				return false;
			}
			if (reloc->r_extern != 0) {
				dprintf(gkalsr_debug, "extern relocation found with local relocations\n");
				return false;
			}
			*((uintptr_t*)(gRelocBase + reloc->r_address)) += slide;

			if (reloc == relocStart)
				dprintf(gkalsr_debug, "r_address = 0x%x, gRelocBase + reloc->r_addres = %p\n",
					reloc->r_address, (uintptr_t*)(gRelocBase + reloc->r_address));
		}
		dprintf(gkalsr_debug, "KASLR: %d local relocations processed\n", dysymtab->nlocrel);
	}

	return true;
}
