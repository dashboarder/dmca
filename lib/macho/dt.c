/*
 * Copyright (C) 2007-2014 Apple Inc. All rights reserved.
 * Copyright (C) 1998-2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#include <debug.h>
#if WITH_HW_GASGAUGE
#include <drivers/gasgauge.h>
#endif
#include <drivers/power.h>
#include <lib/devicetree.h>
#include <lib/env.h>
#include <lib/image.h>
#include <lib/macho.h>
#include <lib/net.h>
#include <lib/nvram.h>
#include <lib/paint.h>
#include <lib/random.h>
#include <lib/syscfg.h>
#if WITH_TICKET
#include <lib/ticket.h>
#endif
#include <platform.h>
#include <platform/memmap.h>
#include <sys.h>
#include <sys/hash.h>
#include <target.h>
#if WITH_CONSISTENT_DBG
#include <drivers/consistent_debug.h>
#endif

extern bool contains_boot_arg(const char *bootArgs, const char *arg, bool prefixmatch);

#include "boot.h"

#define kBootPResponceSize (0x12C)
static char gBootPResponse[kBootPResponceSize];

#define ROOT_DTPATH		""

#ifndef ETHERNET_DTPATH
#define ETHERNET_DTPATH		"ethernet"
#endif

#ifndef MULTITOUCH_DTPATH
#define MULTITOUCH_DTPATH	""
#endif

#ifndef WIFI_DTPATH
#define WIFI_DTPATH		"arm-io/sdio"
#endif

#ifndef PINTO_DTPATH
#define PINTO_DTPATH		""
#endif

#ifndef BT_DTPATH
#define BT_DTPATH		"arm-io/uart3/bluetooth"
#endif
#ifndef BASEBAND_DTPATH
#define BASEBAND_DTPATH		"baseband"
#endif

#ifndef CHARGER_DTPATH
#define CHARGER_DTPATH		"charger"
#endif

#ifndef USBDEVICE_DTPATH
#define USBDEVICE_DTPATH	"arm-io/usb-complex/usb-device"
#endif

int UpdateDeviceTree(const boot_args *boot_args)
{
  DTNodePtr node, chosen;
  uint32_t propSize, boot_partition;
  char *propName;
  void *propData;
  const char *str;
  int result, noenet = 0;
  uint32_t ipaddr;
  uint8_t ethaddr[6];
  void *wifiaddr;
  const char *dram_vendor_string;
  const char *product_id;
#if WITH_SYSCFG
  void *batteryId = NULL;
  uint32_t batteryIdSize = 0;
#endif

#if WITH_SYSCFG
  // Put the model number in the root node
  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "model-number";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      syscfgCopyDataForTag('Mod#', propData, propSize);
    }
  }
  
  // Put the regulatory model number in the root node
  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "regulatory-model-number";
    if (FindProperty(node, &propName, &propData, &propSize))
      if (syscfgCopyDataForTag('RMd#', propData, propSize) == -1 && ((char *)propData)[0] == 0)
        propName[0] = '~';
  }

  // Put the region info in the root node
  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "region-info";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      syscfgCopyDataForTag('Regn', propData, propSize);
    }
  }
  
  // Put the serial number in the root node
  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "serial-number";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      syscfgCopyDataForTag('SrNm', propData, propSize);
    }
  }
	
	//  Put the mlb serial number in the root node
  	if (FindNode(0, ROOT_DTPATH, &node)) {
  		propName = "mlb-serial-number";
		 if (FindProperty(node, &propName, &propData, &propSize)) {
			 syscfgCopyDataForTag('MLB#', propData, propSize);
 		}
	}

	//  Put the config number in the root node
  	if (FindNode(0, ROOT_DTPATH, &node)) {
  		propName = "config-number";
		 if (FindProperty(node, &propName, &propData, &propSize)) {
			 syscfgCopyDataForTag('CFG#', propData, propSize);
 		}
	}
	
#endif
  
  // Find the pram node
  if (FindNode(0, "pram", &node)) {
    // Fill in the memory range 
    propName = "reg";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      ((uintptr_t *)propData)[0] = platform_get_memory_region_base(kMemoryRegion_Panic);
      ((uintptr_t *)propData)[1] = platform_get_memory_region_size(kMemoryRegion_Panic);
    }
  }
  
  // Find the vram node
  if (FindNode(0, "vram", &node)) {
    // Fill in the memory range 
    propName = "reg";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      ((uintptr_t *)propData)[0] = platform_get_memory_region_base(kMemoryRegion_Display);
      ((uintptr_t *)propData)[1] = platform_get_memory_region_size(kMemoryRegion_Display);
    }
  }

#if defined(SLEEP_TOKEN_BUFFER_BASE) && defined(SLEEP_TOKEN_BUFFER_SIZE) 
  // Find the stram (sleep-token ram) node
  if (FindNode(0, "stram", &node)) {
    // Fill in the memory range 
    propName = "reg";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      ((uintptr_t *)propData)[0] = platform_get_memory_region_base(kMemoryRegion_SleepToken);
      ((uintptr_t *)propData)[1] = platform_get_memory_region_size(kMemoryRegion_SleepToken);
    }
  }
#endif

  // Find the arm-io node
  if (FindNode(0, "arm-io", &node)) {
    dt_set_prop_32(node, "chip-revision", platform_get_chip_revision());
    dt_set_prop_32(node, "fuse-revision", platform_get_fuse_revision());
  }

 if (FindNode(0, "product", &node)) {
	propName = "product-id";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		product_id = platform_get_product_id();
	    memcpy(propData, product_id, propSize);	
	}	

#if WITH_SYSCFG
	propName = "device-color-legacy";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		syscfgCopyDataForTag('ClrC', propData, propSize);
	}
	
	propName = "primary-calibration-matrix";
	if (FindProperty(node, &propName, &propData, &propSize)) {
		syscfgCopyDataForTag('DPCl', propData, propSize);

		uint32_t *data_ptr = (uint32_t*)propData;
		if (data_ptr[0] == 0x01000000) {
			for(uint32_t i = 0; i < ((uint32_t)propSize / sizeof(uint32_t)); i ++) {
				data_ptr[i] = swap32(data_ptr[i]);
			}
		}
	}

#if WITH_PAINT
  propName = "device-colors";
  if (FindProperty(node, &propName, &propData, &propSize)) {
    paint_color_map_get_DClr((syscfg_DClr_t *)propData, propSize, false);
  }
  propName = "device-color-policy";
  if (FindProperty(node, &propName, &propData, &propSize)) {
    if (propSize == sizeof(uint32_t)) {
      *(uint32_t *)propData = paint_get_color_policy();
    }
  }
#endif
#endif
 }
  
  str = env_get("network-type");
  if (str && strcmp(str, "ethernet")) noenet = 1;
  
  // Find the chosen node.
  if (!FindNode(0, "chosen", &chosen)) {
    printf("UpdateDeviceTree: failed to find the /chosen node\n");
    return -1;
  }
  
#if WITH_NVRAM
  // <rdar://problem/9529235> race condition possible between
  // IODTNVRAM and IONVRAMController (restore loses boot-args)
  nvram_update_devicetree(chosen, "nvram-proxy-data");
#endif

  // Fill in the debug-enabled property
  propName = "debug-enabled";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityModeKDPEnabled)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }

#if WITH_CONSISTENT_DBG
  // Fill in the consistent-debug-root property
  propName = "consistent-debug-root";
  if(FindProperty(chosen, &propName, &propData, &propSize))
  {
    *(uint64_t*)propData = (uint64_t)(consistent_debug_get_registry());
  }
#endif  
  
  // Fill in the development-cert property
  propName = "development-cert";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityModeDevCertAccess)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }
  
  // Fill in the production-cert property
  propName = "production-cert";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityModeProdCertAccess)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }
  
  // Fill in the gid-aes-key property
  propName = "gid-aes-key";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityModeGIDKeyAccess)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }
  
  // Fill in the uid-aes-key property
  propName = "uid-aes-key";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityModeUIDKeyAccess)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }
  
  // Fill in the secure-boot property
  propName = "secure-boot";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityStatusSecureBoot)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }

  // Fill in the certificate-production-status (raw production mode) property
  propName = "certificate-production-status";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
      ((uint32_t *)propData)[0] = platform_get_raw_production_mode();
  }

  // Fill in the certificate-security-mode (raw security mode) property
  propName = "certificate-security-mode";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
      ((uint32_t *)propData)[0] = platform_get_secure_mode();
  }

  // Fill in the effective-production-status-ap property
  propName = "effective-production-status-ap";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
      ((uint32_t *)propData)[0] = platform_get_current_production_mode();
  }

  // Fill in the effective-security-mode-ap property
  // This is for legacy reasons. As far as the AP is concerned, its effective security mode
  // doesn't change with demotion
  // <rdar://problem/16434259> Get rid of effective-security-mode-ap device tree property
  propName = "effective-security-mode-ap";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
      ((uint32_t *)propData)[0] = platform_get_secure_mode();
  }
  
#if WITH_IMAGE4
  // Fill in the boot-nonce property
  propName = "boot-nonce";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    uint64_t boot_nonce =  platform_get_nonce();
    memcpy(propData, &boot_nonce, propSize);	
  }
#endif  

  // Fill in the memory vendor info
  propName = "dram-vendor";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    dram_vendor_string = platform_get_memory_manufacturer_string();
    strlcpy(propData, dram_vendor_string, propSize);
  }
  
#if WITH_SYSCFG
  // Fill in the software-behavior property
  propName = "software-behavior";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    syscfgCopyDataForTag('SwBh', propData, propSize);
  }

  // Fill in the software-bundle-version property
  propName = "software-bundle-version";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    syscfgCopyDataForTag('SBVr', propData, propSize);
  }
#endif
  
  // Fill in the system-trusted property
  propName = "system-trusted";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    if (security_allow_modes(kSecurityStatusSystemTrusted)) {
      ((uint32_t *)propData)[0] = 1;
    }
  }
  
  // Fill in the board-id property
  propName = "board-id";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint32_t *)propData)[0] = platform_get_board_id();
  }
  
  // Fill in the chip-id property
  propName = "chip-id";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint32_t *)propData)[0] = platform_get_chip_id();
  }
  
  // Fill in the unique-chip-id property
  propName = "unique-chip-id";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint64_t *)propData)[0] = platform_get_ecid_id();
  }
  
  // Fill in the die-id property
  propName = "die-id";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint64_t *)propData)[0] = platform_get_die_id();
  }

  // Fill in the random-seed property
  propName = "random-seed";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    random_get_bytes(propData, propSize);
  }

  // Fill in the firmware-version
  propName = "firmware-version";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    strlcpy(propData, XBS_BUILD_TAG, propSize);
  }

#if WITH_TICKET
  // Fill in the root-ticket-hash property
  propName = "root-ticket-hash";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
	uint8_t hash[CCSHA1_OUTPUT_SIZE];
	if (propSize >= sizeof(hash) && ticket_get_hash(hash, sizeof(hash))) {
	  memcpy(propData, hash, sizeof(hash));
	}
  }
#endif

#if WITH_IMAGE4
  {
    uint8_t boot_manifest_hash[HASH_OUTPUT_SIZE];

    propName = "boot-manifest-hash";
    if (FindProperty(chosen, &propName, &propData, &propSize)) {
      if (platform_get_boot_manifest_hash(boot_manifest_hash) == 0) {
        // Fill in the boot-manifest-hash property
        if (propSize >= sizeof(boot_manifest_hash)) {
            memcpy(propData, boot_manifest_hash, sizeof(boot_manifest_hash));
        } else {
          panic("%s property is too small", propName);
        }
      }
      else {
        // Rename the boot-manifest-hash property to mark it invalid
        propName[0] = '~';
      }
    }
  }

  // Fill in the mix-n-match-prevention-status property
  propName = "mix-n-match-prevention-status";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint32_t *)propData)[0] = platform_get_mix_n_match_prevention_status();
  }
#endif

  // Fill in the display-rotation
  propName = "display-rotation";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint32_t *)propData)[0] = ((boot_args->video.v_depth >> 8) & 0xFF) * 90;
  }

  // Fill in the display-rotation
  propName = "display-scale";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    ((uint32_t *)propData)[0] = ((boot_args->video.v_depth >> 16) & 0xFF) + 1;
  }

  // Fill in the bootp-response packet
  propName = "bootp-response";
  if (FindProperty(chosen, &propName, &propData, &propSize)) {
    *gBootPResponse = 2;
    
    ipaddr = 0;
    env_get_ipaddr("ipaddr", &ipaddr);
    memcpy(gBootPResponse + 0x10, &ipaddr, 4);
    if (env_get("gateway")) {
      env_get_ipaddr("gateway", &ipaddr);
      memcpy(gBootPResponse + 236, "\143\202\123\143", 4); //dhcp magic cookie
      memcpy(gBootPResponse + 236 + 4, "\003\004", 1 + 1); //dhcp tag=3(gateway), len = 4
      memcpy(gBootPResponse + 236 + 6, &ipaddr, 4); //dhcp gateway ip
    }
    
    memcpy(propData, gBootPResponse, kBootPResponceSize);
  }

  dt_set_prop_32(chosen, "security-domain", platform_get_security_domain());
  dt_set_prop_32(chosen, "chip-epoch", platform_get_hardware_epoch());

  // tell the kernel which partition to root off of, unless the user was explicit
  if (!contains_boot_arg(boot_args->commandLine, "rd=", true)) {
    propName = "root-matching";
    if (FindProperty(chosen, &propName, &propData, &propSize)) {
#if RELEASE_BUILD
      boot_partition = 1;
#else
      boot_partition = env_get_uint("boot-partition", 0) + 1;
#endif
	    boot_partition = 
      // default to the same logic as what is used for loading
      // the kernelcache, even if the user loaded that over usb.
      // The IOStorageFamily uses 1-indexed partitions
	  snprintf(propData, propSize,
               "<dict><key>IOProviderClass</key><string>IOMedia</string><key>IOPropertyMatch</key><dict><key>Partition ID</key><integer>%u</integer></dict></dict>",
               boot_partition);
    }
  }

#if defined(ASP_BASE) && defined(ASP_SIZE)
  // Update ASP region info
  if (FindNode(0, "arm-io/ans/iop-ans-nub", &node)) {
    propName = "region-base";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      ((uintptr_t *) propData)[0] = platform_get_memory_region_base(kMemoryRegion_StorageProcessor);
    }
    propName = "region-size";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      ((uintptr_t *) propData)[0] = platform_get_memory_region_size(kMemoryRegion_StorageProcessor);
    }
  }
#endif

  // Add the platform name to the root node
  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "platform-name";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      strlcpy(propData, PLATFORM_NAME_STRING, propSize);
    }
  }

  // Populate consistent location in device tree with mac addresses for Gestalt
  if (FindNode(0, "chosen", &node)) {	
    propName = "mac-address-ethernet0";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if (env_get("ethaddr")) {
	env_get_ethaddr("ethaddr", propData);
      } else {
	int retlen = 0;
	if (!target_get_property(TARGET_PROPERTY_ETH_MACADDR, propData, 6, &retlen)) {
	  propName[0] = '~';
	}	
      }
    }

    propName = "mac-address-wifi0";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if (env_get("wifiaddr")) {
        env_get_ethaddr("wifiaddr", propData);
      } else {
        int retlen = 0;
	if (!target_get_property(TARGET_PROPERTY_WIFI_MACADDR, propData, 6, &retlen)) {
          propName[0] = '~';
	}
      }
    }

    propName = "mac-address-bluetooth0";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if (env_get("btaddr")) {
	env_get_ethaddr("btaddr", propData);
      } else {
	int retlen = 0;
	if (!target_get_property(TARGET_PROPERTY_BT_MACADDR, propData, 6, &retlen)) {
	  propName[0] = '~';
	}
      }
    }
  }
  
  // Set the ethernet address in "/"
  memset(ethaddr, 0, sizeof(ethaddr));
  if (env_get("ethaddr")) {
    env_get_ethaddr("ethaddr", ethaddr);
  } else {
    int retlen = 0;
    target_get_property(TARGET_PROPERTY_ETH_MACADDR, ethaddr, 6, &retlen);	
  }

  if (FindNode(0, ROOT_DTPATH, &node)) {
    propName = "local-mac-address";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      memcpy(propData, ethaddr, 6);
    }
  }
  
  // Set the ethernet address is ETHERNET_DTPATH
  if (FindNode(0, ETHERNET_DTPATH, &node)) {
    propName = "local-mac-address";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      memcpy(propData, ethaddr, 6);
      if (noenet) {
	propName = "compatible";
	if (FindProperty(node, &propName, &propData, &propSize)) {
	  strlcpy(propData, "xxx", propSize);
	  printf("Ethernet disabled\n");
	}
      }
    }
  }
  // store enet mac-address into 'device-mac-address' in 'usb-device' node for tethering
  else if (FindNode(0, USBDEVICE_DTPATH, &node)) {
    propName = "device-mac-address";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      memcpy(propData, ethaddr, 6);
    }
  }
  
  // Set the wifi address/calibration in the wifi node (SDIO or HSIC)
  if (FindNode(0, WIFI_DTPATH, &node)) {
    DTNodePtr usb_node;
    propName = "local-mac-address";
    if (FindProperty(node, &propName, &wifiaddr, &propSize)) {
      if (env_get("wifiaddr")) {
        env_get_ethaddr("wifiaddr", wifiaddr);
      } else {
        int retlen = 0;
        target_get_property(TARGET_PROPERTY_WIFI_MACADDR, wifiaddr, 6, &retlen);
      }
      // store 'wifiaddr' into 'host-mac-address' in 'usb-device' node for tethering
      if (FindNode(0, USBDEVICE_DTPATH, &usb_node)) {
          propName = "host-mac-address";
          if (FindProperty(usb_node, &propName, &propData, &propSize)) {
              memcpy(propData, wifiaddr, 6);
          }
      }
    }

    propName = "tx-calibration";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_TX, propData, 1024, &retlen);
    }

    propName = "tx-calibration-2.4";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_TX_24, propData, propSize, &retlen);
    }

    propName = "tx-calibration-5.0";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_TX_50, propData, propSize, &retlen);
    }
    
    propName = "rx-calibration-2.4";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_RX_24, propData, propSize, &retlen);
    }

    propName = "rx-calibration-5.0";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_RX_50, propData, propSize, &retlen);
    }
    
    propName = "rx-calibration-temp";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_RX_TEMP, propData, propSize, &retlen);
    }

    propName = "freq-group-2g-calibration";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_CALIBRATION_FREQ_GROUP_2G, propData, propSize, &retlen);
    }

    propName = "wireless-board-snum";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_BOARD_SNUM, propData, propSize, &retlen);
    }
    
    propName = "wifi-calibration-msf";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_WIFI_WCAL, propData, propSize, &retlen);
    }
  }

  if (FindNode(0, BT_DTPATH, &node)) {
    // Set the bt address in the bluetooth node
    propName = "local-mac-address";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if (env_get("btaddr")) {
        env_get_ethaddr("btaddr", propData);
      } else {
        int retlen = 0;
        target_get_property(TARGET_PROPERTY_BT_MACADDR, propData, 6, &retlen);
      }
    }
    
#if WITH_SYSCFG
    // Update Bluetooth TX and RX calibration data. If calibration data is
    // missing, don't publish these properties.
    propName = "bluetooth-tx-calibration";
    if (FindProperty(node, &propName, &propData, &propSize))
      if (syscfgCopyDataForTag('BTTx', propData, propSize) == -1)
        propName[0] = '~';

    propName = "bluetooth-rx-calibration";
    if (FindProperty(node, &propName, &propData, &propSize))
      if (syscfgCopyDataForTag('BTRx', propData, propSize) == -1)
        propName[0] = '~';
#endif
  }

  //Fill Pinto address
  if (FindNode(0, PINTO_DTPATH, &node)) {
    // Set the pinto address in the corresponding node
    propName = "local-mac-address";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if (env_get("pintoaddr")) {
        env_get_ethaddr("pintoaddr", propData);
      } else {
        int retlen = 0;
        target_get_property(TARGET_PROPERTY_PINTO_MACADDR, propData, 6, &retlen);
      }
    }
 }

  // Fill in baseband properties as necessary (don't qualify this with presence of the
  // radio driver since there may be work to do regardless)
  if (FindNode(0, BASEBAND_DTPATH, &node)) {
#if WITH_SYSCFG
    propName = "battery-id";
    if (FindProperty(node, &propName, &propData, &propSize)) {
#if WITH_HW_GASGAUGE
      if (gasgauge_get_battery_id(propData, propSize) == 0) {
        batteryId = propData;
        batteryIdSize = propSize;
      }
      else	
#endif
      {
	syscfgCopyDataForTag('Batt', propData, propSize);
      }
    }
#endif
    
    propName = "region-sku";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      int retlen = 0;
      target_get_property(TARGET_PROPERTY_BB_REGION_SKU, propData, propSize, &retlen);
    }
  }

  // Put the battery-id, boot-voltage in the charger node
  if (FindNode(0, CHARGER_DTPATH, &node)) {
#if WITH_SYSCFG
    propName = "battery-id";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      if ((batteryId != NULL) && (propSize == batteryIdSize)) {
        memcpy(propData, batteryId, propSize);  
      }
#if WITH_HW_GASGAUGE
      else if (gasgauge_get_battery_id(propData, propSize) != 0)
#endif
      {
	syscfgCopyDataForTag('Batt', propData, propSize);
      }
    }
#endif

#if WITH_HW_POWER
    propName = "boot-voltage";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      *(uint32_t *)propData = power_get_boot_battery_level();
    }

    propName = "boot-adapter-id";
    if (FindProperty(node, &propName, &propData, &propSize)) {
	power_get_usb_brick_id((uint32_t *)propData, propSize / sizeof(uint32_t));
    }
#endif
  }

#if WITH_HW_MERLOT
  // Set the lcd panel info in the merlot node
  if (FindNode(0, MERLOT_DTPATH, &node)) {
    extern const uint32_t gMerlotPanelID, gMerlotInitRegisterCount;
    extern const uint8_t  *gMerlotInitRegisters;
    
    propName = "lcd-panel-id";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      *(uint32_t *)propData = gMerlotPanelID;
    }
    propName = "lcd-init-register-count";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      *(uint32_t *)propData = gMerlotInitRegisterCount;
    }
    propName = "lcd-init-registers";
    if ((gMerlotInitRegisters != 0) && FindProperty(node, &propName, &propData, &propSize)) {
      memcpy(propData, gMerlotInitRegisters, gMerlotInitRegisterCount * 2);
    }
  }
#endif

#if WITH_SYSCFG
  // Save the multi-touch calibration
  if (FindNode(0, MULTITOUCH_DTPATH, &node)) {
    propName = "multi-touch-calibration";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      syscfgCopyDataForTag('MtCl', propData, propSize);
    }
  }

  uint32_t sysCfgData = 0;
  // Obtain the syscfg entry that identifies when a rear camera is absent
  result = syscfgCopyDataForTag('NoBC', (uint8_t *)(&sysCfgData), sizeof(sysCfgData));
	
  // Set rear camera information
  if (FindNode(0, "arm-io/isp", &node)) {
    propName = "camera-rear";
    if (FindProperty(node, &propName, &propData, &propSize)) {
      // if bit 0 is 1, then delete rear camera entry
      if (result == sizeof(sysCfgData) && (sysCfgData & 0x1))
        propName[0] = '~';
    }
  }

  // Also delete rear camera related properties in product/camera if rear camera is absent
  if (FindNode(0, "product/camera", &node)) {
    
    if (result == sizeof(sysCfgData) && (sysCfgData & 0x1)) {
      propName = "flash";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';

      propName = "rear-burst";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';

      propName = "rear-slowmo";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';
      
      propName = "rear-max-video-zoom";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';

      propName = "rear-hdr";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';

      propName = "panorama";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';

      propName = "rear-auto-hdr";
      if (FindProperty(node, &propName, &propData, &propSize))
        propName[0] = '~';
    }
    
  }
	
#endif

  // Let the platform and target make updates
  result = platform_update_device_tree();
  if (result < 0) return result;
  
  // Find the iBoot node under the chosen direectory
  // as that's where the boot specific stuff goes.
  if (FindNode(0, "chosen/iBoot", &node)) {
      // Fill in the boot performance information 
      extern const uint32_t gPowerOnTime;
      extern const uint32_t gDebugPromptTime;
      extern const uint32_t gLoadKernelTime;
      const uint32_t populateRegistryTime = system_time();
      
      propName = "start-time";
      if (FindProperty(node, &propName, &propData, &propSize)) {
          *(uint32_t *)propData = gPowerOnTime;
      }
      propName = "debug-wait-start";
      if (FindProperty(node, &propName, &propData, &propSize)) {
          *(uint32_t *)propData = gDebugPromptTime;
      }
      propName = "load-kernel-start";
      if (FindProperty(node, &propName, &propData, &propSize)) {
          *(uint32_t *)propData = gLoadKernelTime;
      }
      propName = "populate-registry-time";
      if (FindProperty(node, &propName, &propData, &propSize)) {
          *(uint32_t *)propData = populateRegistryTime;
      }
  }
  
  return 0;
}
