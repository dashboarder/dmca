//
//  L2V_Dump.m
//  L2V_Dump
//
//  Created by John Vink on 7/29/11.
//  Copyright (c) 2011 Apple, Inc. All rights reserved.
//


#import "L2V_Print.h"
#import "L2V_Types.h"

#import <Foundation/Foundation.h>

#define L2V_SIZE_32_BIT				(0x3c)
#define L2V_SIZE_64_BIT				(0x50)
#define L2V_OFFSET_POOL_64_BIT		(0x38)
#define SIZE_OF_LONG_64_BIT			8

L2V_t L2V;

// -------------------------------------------------------------
// * usage
// -------------------------------------------------------------
static void usage()
{
	printf("L2V_Dump: <file>\n");
}


// -------------------------------------------------------------
// * main
// -------------------------------------------------------------
int main (int argc, const char * argv[])
{
	NSAutoreleasePool* pool = [NSAutoreleasePool new];

	// try to make sure the L2V struct hasn't changed (much) since I wrote the code to convert it from 32 bit to 64 bit
	if (sizeof(long) == SIZE_OF_LONG_64_BIT)
	{
		L2V_t* dummyL2V = (L2V_t*) 0;
		if (sizeof(L2V_t) != L2V_SIZE_64_BIT || (long)&dummyL2V->Pool != L2V_OFFSET_POOL_64_BIT)
		{
			printf("L2V struct has changed.\n");
			return 1;
		}
	}
	
#if 0
	fprintf(stderr, "architecture: %d bit\n", sizeof(long) * 8);
	
	L2V_t* dummyL2V = (L2V_t*) 0;
	printf("numRoots       %2x %d\n", &dummyL2V->numRoots, sizeof(dummyL2V->numRoots));
	printf("Root           %2x %d\n", &dummyL2V->Root, sizeof(dummyL2V->Root));
	printf("RepackCounter  %2x %d\n", &dummyL2V->RepackCounter, sizeof(dummyL2V->RepackCounter));
	printf("treeVersion    %2x %d\n", &dummyL2V->treeVersion, sizeof(dummyL2V->treeVersion));
	printf("max_sb         %2x %d\n", &dummyL2V->max_sb, sizeof(dummyL2V->max_sb));
	printf("vbas_per_sb    %2x %d\n", &dummyL2V->vbas_per_sb, sizeof(dummyL2V->vbas_per_sb));
	printf("bits           %2x %d\n", &dummyL2V->bits, sizeof(dummyL2V->bits));
	printf("bits.node_span %2x %d\n", &dummyL2V->bits.node_span, sizeof(dummyL2V->bits.node_span));
	printf("bits.nodeIdx   %2x %d\n", &dummyL2V->bits.nodeIdx, sizeof(dummyL2V->bits.nodeIdx));
	printf("bits.nand_span %2x %d\n", &dummyL2V->bits.nand_span, sizeof(dummyL2V->bits.nand_span));
	printf("bits.vba       %2x %d\n", &dummyL2V->bits.vba, sizeof(dummyL2V->bits.vba));
	printf("vba            %2x %d\n", &dummyL2V->vba, sizeof(dummyL2V->vba));
	printf("vba.special    %2x %d\n", &dummyL2V->vba.special, sizeof(dummyL2V->vba.special));
	printf("vba.dealloc    %2x %d\n", &dummyL2V->vba.dealloc, sizeof(dummyL2V->vba.dealloc));
	printf("Pool           %2x %d\n", &dummyL2V->Pool, sizeof(dummyL2V->Pool));
	printf("Total Size     %2x\n", sizeof(L2V_t));
#endif
	
	if (argc != 2)
	{
		usage();
		return 1;
	}
	
	const char* filename = argv[1];
	int fd = open(filename, O_RDONLY);
	if (-1 == fd)
	{
		fprintf(stderr, "Cannot open %s\n", filename);
		return 1;
	}
	
	// see how big the XML is
	UInt32 xmlSize = 0;
	ssize_t result = read(fd, &xmlSize, sizeof(xmlSize));
	if (sizeof(xmlSize) != result)
	{
		fprintf(stderr, "Unable to read size of XML\n");
		return 1;
	}
	
	// read the XML
	char* xmlBuffer = (char*) malloc(xmlSize);
	if (NULL == xmlBuffer)
	{
		fprintf(stderr, "Cannot allocate XML buffer of %d\n", xmlSize);
		return 1;
	}
	
	result = read(fd, xmlBuffer, xmlSize);
	if ((ssize_t)xmlSize != result)
	{
		fprintf(stderr, "Unable to read XML\n");
		return 1;
	}
	
	NSData* xmlData = [NSData dataWithBytes:xmlBuffer length:xmlSize];
	if (NULL == xmlData)
	{
		fprintf(stderr, "Cannot create NSData from XML buffer\n");
		return 1;
	}
	
	NSPropertyListReadOptions format = NSPropertyListXMLFormat_v1_0;
	NSError* error = NULL;
	NSDictionary* dict = [NSPropertyListSerialization propertyListWithData:xmlData options:0 format:&format error:&error];
	if (NULL == dict)
	{
		fprintf(stderr, "Cannot convert XML data to NSDictionary\n");
		return 1;
	}
	
	NSDictionary* infoDict = [dict objectForKey:@"Info"];
	if (NULL == infoDict || ![infoDict isKindOfClass:[NSDictionary class]])
	{
		fprintf(stderr, "Cannot get Info dict\n");
		return 1;
	}
	
	NSNumber* ftlTypeNumber = [infoDict objectForKey:@"FTL Type"];
	if (NULL == ftlTypeNumber || ![ftlTypeNumber isKindOfClass:[NSNumber class]])
	{
		fprintf(stderr, "Cannot get FTL Type from Info dict\n");
		return 1;
	}
	
	if ([ftlTypeNumber intValue] != FTL_TYPE_SFTL)
	{
		fprintf(stderr, "FTL type is not AppleSwiss\n");
		return 1;
	}
	
	NSArray* blobs = [dict objectForKey:@"Blobs"];
	if (NULL == blobs || ![blobs isKindOfClass:[NSArray class]])
	{
		fprintf(stderr, "Cannot get Blobs array\n");
		return 1;
	}
	
	NSDictionary* structDict;
	L2V_t l2vStruct;
	for (structDict in blobs)
	{
		if (![structDict isKindOfClass:[NSDictionary class]])
		{
			fprintf(stderr, "Blobs element is not a dictionary\n");
			return 1;
		}
		
		NSString* structType = [structDict objectForKey:@"Label"];
		NSNumber* structSize = [structDict objectForKey:@"Size"];
		
		if (NULL == structType || ![structType isKindOfClass:[NSString class]])
		{
			fprintf(stderr, "Cannot get Blob label\n");
			return 1;
		}
		
		if (NULL == structSize || ![structSize isKindOfClass:[NSNumber class]])
		{
			fprintf(stderr, "Cannot get Blob size\n");
			return 1;
		}
		
		if ([structType isEqualToString:@"AND_STRUCT_FTL_L2V_STRUCT"])
		{
			BOOL read32Bit = NO;
			if ([structSize intValue] != sizeof(L2V_t))
			{
				if ([structSize intValue] == L2V_SIZE_32_BIT)
				{
					read32Bit = YES;
				}
				else
				{
					fprintf(stderr, "Expected size of %d for L2V, but got %d\n", sizeof(L2V_t), [structSize intValue]);
					return 1;
				}
			}
			if (read32Bit)
			{
				uint8_t* l2v32bitBuffer = malloc(L2V_SIZE_32_BIT);
				result = read(fd, l2v32bitBuffer, L2V_SIZE_32_BIT);
				if (result != L2V_SIZE_32_BIT)
				{
					fprintf(stderr, "Could not read L2V struct\n");
					free(l2v32bitBuffer);
					return 1;
				}
				l2vStruct.numRoots = *(UInt32*)l2v32bitBuffer;
				l2vStruct.RepackCounter = *(UInt32*)&l2v32bitBuffer[8];
				l2vStruct.treeVersion = *(UInt32*)&l2v32bitBuffer[0xc];
				l2vStruct.max_sb = *(UInt32*)&l2v32bitBuffer[0x10];
				l2vStruct.vbas_per_sb = *(UInt32*)&l2v32bitBuffer[0x14];
				l2vStruct.bits.node_span = *(UInt32*)&l2v32bitBuffer[0x18];
				l2vStruct.bits.nodeIdx = *(UInt32*)&l2v32bitBuffer[0x1c];
				l2vStruct.bits.nand_span = *(UInt32*)&l2v32bitBuffer[0x20];
				l2vStruct.bits.vba = *(UInt32*)&l2v32bitBuffer[0x24];
				l2vStruct.vba.special = *(UInt32*)&l2v32bitBuffer[0x28];
				l2vStruct.vba.dealloc = *(UInt32*)&l2v32bitBuffer[0x2c];
				free(l2v32bitBuffer);
			}
			else
			{
				result = read(fd, &l2vStruct, sizeof(L2V_t));
				if (result != sizeof(L2V_t))
				{
					fprintf(stderr, "Could not read L2V struct\n");
					return 1;
				}
			}
		}
		else if ([structType isEqualToString:@"AND_STRUCT_FTL_L2V_ROOTS"])
		{
			UInt32 rootSize = [structSize intValue];
			l2vStruct.Root = malloc(rootSize);
			if (NULL == l2vStruct.Root)
			{
				fprintf(stderr, "Could not allocate %d bytes for roots\n", rootSize);
				return 1;
			}
			result = read(fd, l2vStruct.Root, rootSize);
			if (result != (ssize_t)rootSize)
			{
				fprintf(stderr, "Could not read L2V roots\n");
				return 1;
			}
		}
		else if ([structType isEqualToString:@"AND_STRUCT_FTL_L2V_POOL"])
		{
			UInt32 poolSize = [structSize intValue];
			l2vStruct.Pool.Node = malloc(poolSize);
			if (NULL == l2vStruct.Pool.Node)
			{
				fprintf(stderr, "Could not allocate %d bytes for pool\n", poolSize);
				return 1;
			}
			result = read(fd, l2vStruct.Pool.Node, poolSize);
			if (result != (ssize_t)poolSize)
			{
				fprintf(stderr, "Could not read L2V pool\n");
				return 1;
			}
		}
		else
		{
			fprintf(stderr, "Unknown label %s\n", [structType cStringUsingEncoding:NSASCIIStringEncoding]);
			return 1;
		}
	}
	
	l2vStruct.Pool.FreePtr = NULL;
	l2vStruct.Pool.FreeCount = 0;
	
	L2V = l2vStruct;

	printf("Number of roots:%d\n", l2vStruct.numRoots);
	UInt32 rootNumber;
	for (rootNumber = 0; rootNumber < l2vStruct.numRoots; rootNumber++)
	{
		printf("---- Root %d:\n", rootNumber);
		L2V_PrintUsage(rootNumber);
	}
	
	[pool release];	

    return 0;
}

