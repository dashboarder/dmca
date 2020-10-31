#!/usr/bin/python2.7

import optparse
import os
import plistlib
import textwrap
import shutil
import subprocess
import sys
import time
import hashlib
import binascii

# ECID: This is the value passed on the personalize_img4 -e option.
kDefaultEcid = 0x000012345678abcd

# BNCN: This is the value passed on the personalize_img4 -r option.
kDefaultApNonce = 0x12345678aabbccdd

# Old fashioned terminal size.
kLogWidth = 78
kOutputDirPrefix = 'test.'
kLogFileName = 'README.txt'

kDefaultPlatform = 't8010'
kDefaultChipId = 0x8010
kDefaultBoardId = 0xff
kDefaultBlinkyGPIOReg = 'GPIO_GPIOCFG20 @2_0f100050[31:0]'
kDefaultBlinkyGPIOValue = '0x00070203'
kDefaultDFUStatusGPIOReg = 'GPIO_GPIOCFG206 @2_0f100338[31:0]'
kDefaultDFUStatusGPIOValue = '0x00478283'
kDefaultUSBDCTLReg = 'OTG_OTG_DCTL.SftDiscon @2_0c100804[1]'
kDefaultUSBDCTLValue = '0'
kDefaultPCIEVCODIGCTRLReg = 'PMGR_PLL_PCIE_ANA_PARAMS3.FCAL_VCODIGCTRL @2_0e030014[20:16]'
kDefaultPCIEVCODIGCTRLValue = 0
kDefaultSiDPReg = 'MINIPMGR_SECURITY_SEAL_CTL_A @2_102d4020[31:0]'
kDefaultSiDPUnlocked = '0x00000000'
kDefaultSiDPInvalid = '0x00010000'
kDefaultSiDPValid = '0x00010001'
kDefaultUSB20PHYCFG0Reg = 'AUSBCTLREG_USB20PHY_CFG0 2_0c000038[31:0]'
kDefaultUSB20PHYCFG0Value = '0x37377bc3'
kDefaultUSB20PHYCFG1Reg = 'AUSBCTLREG_USB20PHY_CFG1 @2_0c00003c[31:0]'
kDefaultUSB20PHYCFG1Value = '0x00020c04'
kBoardIdStrapBits = 5
kSecurityDomainDarwin = 1

# The following calculates the nonce hash and formats it correctly so that a
# person doesn't have to deal with generating the a hash, truncating it, and
# getting the endian-ness right.
kDefaultApNonceHashTruncatedSize = 256
kDefaultApNonceByteArray = bytearray.fromhex(hex(kDefaultApNonce)[2:])
kDefaultApNonceString = binascii.hexlify(kDefaultApNonceByteArray)
kDefaultApNonceLittleEndian = int("".join(reversed([kDefaultApNonceString[i:i+2] for i in range(0, len(kDefaultApNonceString), 2)])), 16)
kDefaultApNonceHash384 = hashlib.sha384(kDefaultApNonceByteArray).hexdigest()
kDefaultApNonceHash = int(kDefaultApNonceHash384[:kDefaultApNonceHashTruncatedSize/4], 16)

kDefaultSepNonceHash = 0x1234567823456789345678904567890156789012

# <rdar://problem/15199519> certificate epoch for server signing fixed at 1
kDefaultEpochForServerSigning = 1

# Supported platforms
kSupported_platforms = ['t8010']

# Image sources
kImageSourceDfu = 1
kImageSourceNand = 2
kImageSourceNor = 3
kImageSourceNVMe = 4

# Outcomes
kOutcomeBlinky = 1
kOutcomeDfu = 2
kOutcomeBlinkyVcoOverride = 3

# Silicon Data Protection (SiDP) State
kSiDPStateUnlocked = 1
kSiDPStateLockedInvalid = 2
kSiDPStateLockedValid = 3

# Core Type State
kCoreTypeECore = 1  # Efficient core: Zephyr
kCoreTypePCore = 2  # Performant core: Hurricane

# Can be overidden by command line options.
kImg4DecodeTestTool = 'xcrun -sdk iphoneos.internal -run img4decodetest'
kPersonalizeImg4Tool = 'xcrun -sdk iphoneos.internal -run personalize_img4'
kBfnMaker = 'xcrun -sdk iphoneos.internal -run bfn_maker'

kBfnMakerPageSize = 8224
kBfnMakerFormat = 'h6'

def ErrorText(text):
    RedColor    = '\033[91m'
    NormalColor = '\033[0m'
    return RedColor + text + NormalColor

def WarningText(text):
    YellowColor = '\033[93m'
    NormalColor = '\033[0m'
    return YellowColor + text + NormalColor

def PassText(text):
    GreenColor  = '\033[92m'
    NormalColor = '\033[0m'
    return GreenColor + text + NormalColor

def ByteSwap(value, bits):
  assert (bits % 8) == 0
  result = 0
  for i in xrange(0, bits / 8):
    result <<= 8
    result |= value & 0xff
    value >>= 8
  return result

class LogSettings:
  def __init__(self):
    self._all_settings = []
  
  def Append(self, text, settings):
    self._all_settings.append((text, settings))
  
  def Log(self, log):
    # Get a format string which aligns everything.
    longest_entry = 0
    for settings in self._all_settings:
      for s in settings[1]:
        if len(s[0]) > longest_entry:
          longest_entry = len(s[0])
    format = '  %%-%ds  %%s\n' % longest_entry
    
    # Dump all settings.
    for settings in self._all_settings:
      log.write(settings[0] + ':\n')
      for s in settings[1]:
        if (type(s[1]) == int) or (type(s[1]) == long):
          if s[1] < 16:
            out = format % (s[0], '%d' % s[1])
          else:
            out = format % (s[0], '0x%x' % s[1])
        else:
          out = format % (s[0], str(s[1]))
        log.write(out)
      log.write('\n')

class PersonalizeImg4:
  def __init__(self,
               description = None,
               manifest = True,
               platform = kDefaultPlatform,
               options = None,
               log = None,
               test_number = None,
               outcome = None,
               sidp_state = None,
               core_type = None,
               image_source = None,
               fuse_production_status = None,
               fuse_security_mode = None,
               fuse_security_domain = kSecurityDomainDarwin,
               fuse_epoch = None,
               fuse_ecid = kDefaultEcid,
               fuse_cfg_fuse4 = None,
               boot_device = None,
               test_mode = False,
               boot_device_empty = False,
               epoch = kDefaultEpochForServerSigning,
               chip_id = None,
               board_id = None,
               force_dfu = 0,
               ecid = None,
               production_status = None,
               security_mode = None,
               security_domain = kSecurityDomainDarwin,
               allow_mix_and_match = None,
               ap_boot_nonce_hash = None,
               sep_boot_nonce_hash = None,
               enable_keys = None,
               encryption = None,
               crypto_hash_method = 'sha2-384',
               restore_info_nonce = None,
               demote_production = None,
               demote_secure = None,
               lock_fuses = None,
               ):
    # Validate required arguments.
    assert options is not None
    assert log is not None
    assert type(test_number) == int
    assert outcome in [kOutcomeBlinky, kOutcomeDfu, kOutcomeBlinkyVcoOverride]
    assert image_source in [kImageSourceDfu, kImageSourceNand, kImageSourceNor, kImageSourceNVMe]
    assert boot_device is not None
    assert type(boot_device_empty) == bool
    assert type(fuse_production_status) == bool
    assert type(fuse_security_mode) == bool
    assert ((type(fuse_security_domain) == int) and
            (fuse_security_domain >= 0) and (fuse_security_domain <= 3))
    assert ((type(fuse_epoch) == int) and
            (fuse_epoch >= 0) and (fuse_epoch <= 0x7f))
    assert ((fuse_ecid is not None) and
            (fuse_ecid >= 0) and (fuse_ecid < (1 << 64)))

    if manifest:
      assert ((type(epoch) == int) and
              (epoch >= 0) and (epoch <= 0x7f))
      assert ((type(chip_id) == int) and
              (chip_id >= 0) and (chip_id < (1 << 16)))
      assert ((type(board_id) == int) and
              (board_id >= 0) and (board_id < (1 << 8)))
      assert (ecid is not None) and (ecid >= 0) and (ecid < (1 << 64))
      assert type(production_status) == bool
      assert type(security_mode) == bool
      assert ((type(security_domain) == int) and
              (security_domain >= 0) and (security_domain <= 3))
      assert type(allow_mix_and_match) == bool
      assert type(enable_keys) == bool
      assert type(encryption) == bool
      if sep_boot_nonce_hash is None:
        sep_boot_nonce_hash = kDefaultSepNonceHash
    # Set members.
    self._description = description
    self._manifest = manifest
    self._platform = platform
    self._images_dir = 'images_{}'.format(platform)
    self._options = options
    self._log = log
    self._test_number = test_number
    self._outcome = outcome
    self._sidp_state = sidp_state
    self._core_type = core_type
    self._image_source = image_source
    self._fuse_production_status = fuse_production_status
    self._fuse_security_mode = fuse_security_mode
    self._fuse_security_domain = fuse_security_domain
    self._fuse_epoch = fuse_epoch
    self._fuse_ecid = fuse_ecid
    if fuse_cfg_fuse4 is None:
        self._fuse_cfg_fuse4 = '0x00000000'
        self._fcal_vcodigctrl_out = '0x00'
    else:
        assert (type(fuse_cfg_fuse4) == int)
        self._fuse_cfg_fuse4 = '0x%08x' % fuse_cfg_fuse4
        self._fcal_vcodigctrl_out = '0x%02x' % fuse_cfg_fuse4
    self._epoch = epoch
    self._chip_id = chip_id
    self._board_id = board_id
    self._boot_device = boot_device
    self._test_mode = test_mode
    self._boot_device_empty = boot_device_empty
    self._force_dfu = force_dfu
    self._ecid = ecid
    self._production_status = production_status
    self._security_mode = security_mode
    self._security_domain = security_domain
    self._allow_mix_and_match = allow_mix_and_match
    self._ap_boot_nonce_hash = ap_boot_nonce_hash
    self._sep_boot_nonce_hash = sep_boot_nonce_hash
    self._enable_keys = enable_keys
    self._encryption = encryption
    self._crypto_hash_method = crypto_hash_method
    self._restore_info_nonce = restore_info_nonce
    self._demote_production = demote_production
    self._demote_secure = demote_secure
    self._lock_fuses = lock_fuses

  def Personalize(self, im4p, output_dir):
    # Form args.
    if self._options.personalize_img4:
      args = self._options.personalize_img4.split(' ')
    else:
      args = kPersonalizeImg4Tool.split(' ')
    args += ['-i', im4p, '-o', output_dir]

    if not self._manifest:
      args += ['-O']
    else:
      if self._chip_id is not None:
        args += ['-c', '0x%04x' % self._chip_id]
      if self._board_id is not None:
        args += ['-b', '0x%x' % self._board_id]
      if self._ecid is not None:
        args += ['-e', '0x%x' % self._ecid]
      if self._production_status:
        args += ['-p']
      if self._security_mode:
        args += ['-m']
        args += ['-W']
      if self._security_domain is not None:
        args += ['-d', '%d' % self._security_domain]
      if self._allow_mix_and_match:
        args += ['-a']
      if self._ap_boot_nonce_hash is not None:
        args += ['-n', '0x%x' % self._ap_boot_nonce_hash]
      if self._sep_boot_nonce_hash is not None:
        args += ['-s', '0x%x' % self._sep_boot_nonce_hash]
      if self._crypto_hash_method == 'sha2-384':
        args += ['-g 384']
      # disabling keys or demotions require a plist file with Trusted, EPRO, and ESEC defined
      if (not self._enable_keys) or (self._demote_production) or (self._demote_secure):
        object_properties = {'Trusted': self._enable_keys}
        manifest_properties = {}

        if self._demote_production is not None:
          object_properties['DPRO'] = True

        if self._demote_production:
          assert self._production_status == True
          object_properties['EPRO'] = False
        else:
          object_properties['EPRO'] = self._production_status

        if self._demote_secure:
          assert self._security_mode == True
          object_properties['ESEC'] = False
        else:
          object_properties['ESEC'] = self._security_mode

        all_properties = {'OBJP': object_properties, 'MANP': manifest_properties}
        plist_str = plistlib.writePlistToString(all_properties)
        plist_file = '/tmp/personalize-{}.plist'.format(self._test_number)
        f = open(plist_file, 'w')
        f.write(plist_str)
        f.close()

        args += ['-f', plist_file]
      # personalize_img4 tool expects RestoreInfo nonce in big endian (only for LLB)
      if (im4p.find('ibss') == -1) and self._restore_info_nonce is not None:
        args += ['-r', '0x%x' % ByteSwap(self._restore_info_nonce, 64)]
      args += ['-X']

    # Convert args to an shell string.
    invoke_string = ' '.join(args)

    # Execute personalize_img4 utility.
    print('Invoking: %s' % invoke_string)
    pipe = subprocess.Popen(invoke_string, shell=True)
    if pipe.wait() == 0:
      return True
    else:
      self._log.write('Invocation of personalize_img4 failed\n')
      sys.stderr.write(ErrorText('Test %d: Invocation of personalize_img4 failed\n' % self._test_number))
      return False

  def DecodeTest(self, img4):
    if self._options.img4decodetest:
      args = self._options.img4decodetest.split(' ')
    else:
      args = kImg4DecodeTestTool.split(' ')
    args += ['-c', '-i', img4]
    if self._crypto_hash_method == 'sha2-384':
      args += ['-n']
    invoke_string = ' '.join(args)
    print('Invoking: %s' % invoke_string)
    pipe = subprocess.Popen(invoke_string, shell=True)
    if pipe.wait() == 0:
      return True
    else:
      self._log.write('Invocation of img4decodetest failed\n')
      sys.stderr.write(ErrorText('Test %d: Invocation of img4decodetest failed\n' % self._test_number))
      return False

  def BfnMaker(self, input, output):
    if self._options.bfn_maker:
      args = self._options.bfn_maker.split(' ')
    else:
      args = kBfnMaker.split(' ')
    args += ['--pageSize', str(kBfnMakerPageSize)]
    args += ['--format', str(kBfnMakerFormat)]
    args += [input, output]
    invoke_string = ' '.join(args)
    pipe = subprocess.Popen(invoke_string, shell=True)
    if pipe.wait() == 0:
      return True
    else:
      self._log.write('Invocation of bfn_maker failed\n')
      sys.stderr.write(ErrorText('Test %d: Invocation of bfn_maker failed\n' % self._test_number))
      return False    

  def Execute(self):
    # Log header.
    self._log.write('-' * kLogWidth + '\n')
    self._log.write('Test %d\n' % self._test_number)
    self._log.write(textwrap.fill(self._description, width=kLogWidth) + '\n\n')

    # Figure out the input file variant.
    image_type, location = {kImageSourceDfu: ('ibss', 'DFU'),
                            kImageSourceNand: ('illb', 'NAND'),
                            kImageSourceNor: ('illb', 'NOR'),
                            kImageSourceNVMe: ('illb', 'NVMe')}[self._image_source]

    # FAST_NOR boot device is currenly only supported for test mode
    if self._boot_device in ['FAST_NOR', 'SLOW_NOR']:
        if self._test_mode == False:
            self._log.write('  Skipping: FAST_NOR/SLOW_NOR boot device requires test mode\n\n')
            sys.stderr.write(WarningText('Test %d: FAST_NOR/SLOW_NOR boot device requires test mode\n' % self._test_number))
            return False

    if self._boot_device == 'NOR':
        self._boot_config = '0x%x' % (0 + (1 if self._test_mode else 0))
    elif self._boot_device == 'NVME0_X2':
        self._boot_config = '0x%x' % (2 + (1 if self._test_mode else 0))
    elif self._boot_device == 'NVME0_X1':
        self._boot_config = '0x%x' % (4 + (1 if self._test_mode else 0))
    elif self._boot_device == 'SLOW_NOR':
        self._boot_config = '0x6'
    elif self._boot_device == 'FAST_NOR':
        self._boot_config = '0x7'
    else:
        assert 0, '_boot_device ' + self._boot_device + ' is unsupported'

    if self._encryption:
      encryption_name = 'encrypted'
    else:
      encryption_name = 'unencrypted'
    im4p_basename = 'rom_test_%s_%s' % (image_type, encryption_name)
    im4p_filename = im4p_basename + '.im4p'
    im4p_path = os.path.join(self._images_dir, im4p_filename)

    # If valid image in boot device, but force_dfu is set, we also need ibss
    if self._force_dfu and self._boot_device_empty == False:
      dfu_im4p_basename = 'rom_test_ibss_%s' % encryption_name
      dfu_im4p_filename = dfu_im4p_basename + '.im4p'
      dfu_im4p_path = os.path.join(self._images_dir, dfu_im4p_filename)
        
    # Output filename.
    output_dir = kOutputDirPrefix + self._platform
    pretty_filename_base = 'test_%d_%s_%s' % (
      self._test_number, image_type, encryption_name)
    pretty_filename_img4 = pretty_filename_base + '.img4'
    pretty_path_base = os.path.join(output_dir, pretty_filename_base)
    pretty_path_img4 = pretty_path_base + '.img4'

    # If valid image in boot device, but force_dfu is set, we also need ibss
    if self._force_dfu and self._boot_device_empty == False:
      pretty_dfu_filename_base = 'test_%d_ibss_%s' % (
        self._test_number, encryption_name)
      pretty_dfu_filename_img4 = pretty_dfu_filename_base + '.img4'
      pretty_dfu_path_base = os.path.join(output_dir, pretty_dfu_filename_base)
      pretty_dfu_path_img4 = pretty_dfu_path_base + '.img4'

    # Remove old output file if it exists.
    img4 = os.path.join(output_dir, im4p_basename + '.img4')
    if self._force_dfu and self._boot_device_empty == False:
      dfu_img4 = os.path.join(output_dir, dfu_im4p_basename + '.img4')
    try:
      os.unlink(img4)
      if dfu_img4 is not None:
        os.unlink(dfu_img4)
    except:
      pass
    # Personalize this case.
    print('Personalize %s -> %s' % (im4p_filename, img4))
    if not self.Personalize(im4p_path, output_dir):
      sys.stderr.write(ErrorText('Test %d failed to generate\n' % self._test_number))
      # Log failure
      self._log.write('  Failed to generate!\n\n')
      return False

    # Now, if force_dfu is set, personalize the 2nd image
    if self._force_dfu and self._boot_device_empty == False:
      print('Personalize %s -> %s' % (dfu_im4p_filename, dfu_img4))
      if not self.Personalize(dfu_im4p_path, output_dir):
        sys.stderr.write(ErrorText('Test %d failed to generate\n' % self._test_number))
        # Log failure
        self._log.write('  Failed to generate!\n\n')
        return False

    # Rename the output file to the path we want for this test case.
    os.rename(img4, pretty_path_img4)
    print('Test %d generated %s' % (self._test_number, pretty_filename_img4))
      
    if self._force_dfu and self._boot_device_empty == False:
      os.rename(dfu_img4, pretty_dfu_path_img4)
      print('Test %d generated %s' % (self._test_number, pretty_dfu_filename_img4))
            
    # Test that there isn't a certificate vs manifest problem, because
    # it's not worth burning a huge amount of DV time running it.
    if self._manifest:
      if not self.DecodeTest(pretty_path_img4):
        self._log.write('  Failed decode test!\n')
        self._log.write('  Check manifest vs cert is compatible\n\n')
        sys.stderr.write(ErrorText('Test %d: Failed decode test\n' % self._test_number))
        return False
      
      if self._force_dfu and self._boot_device_empty == False:
        if not self.DecodeTest(pretty_dfu_path_img4):
          self._log.write('  Failed decode test!\n')
          self._log.write('  Check manifest vs cert is compatible\n\n')
          sys.stderr.write(ErrorText('Test %d: Failed decode test\n' % self._test_number))
          return False    

    # If the source is NAND, we need to pass it through bfn_maker
    if self._image_source == kImageSourceNand:
      nand_filename = pretty_filename_base + '.nand'
      nand_path = os.path.join(output_dir, nand_filename)
      if not self.BfnMaker(pretty_path_img4, nand_path):
        self._log.write('  Failed to create NAND image\n')
        sys.stderr.write(ErrorText('Test %d: Failed to create NAND image\n' % self._test_number))
        return False
      pretty_filename = nand_filename
    else:
      pretty_filename = pretty_filename_img4

    core_type_string = {kCoreTypeECore: "0 (E-core)",
                        kCoreTypePCore: "1 (P-core)",
                        None:           "Random"
                       }[self._core_type]

    # Log success.
    settings = LogSettings()

    settings.Append(
                    'Fuses',
                    [('Production Status', int(self._fuse_production_status)),
                     ('Security Mode', int(self._fuse_security_mode)),
                     ('Security Domain', self._fuse_security_domain),
                     ('Board ID', '0x%x' % ((self._board_id or kDefaultBoardId) >> kBoardIdStrapBits)),
                     ('Epoch', self._fuse_epoch),
                     ('SE Required', 0),
                     ('BOOT_PCORE', core_type_string),
                     ('ECID_LO', '0x%08x' % (kDefaultEcid & 0xffffffff)),
                     ('ECID_HI', '0x%08x' % ((kDefaultEcid >> 32) & 0xffffffff)),
                     ('CFG_FUSE4', self._fuse_cfg_fuse4),
                     ])

    settings.Append(
                    'Straps',
                    [('Boot-device', self._boot_device),
                     ('Test mode', self._test_mode),
                     ('Boot-config', self._boot_config),
                     ('Board-id', (self._board_id or kDefaultBoardId) & ((1 << kBoardIdStrapBits) - 1)),
                     ('Force-dfu', self._force_dfu),
                     ('Boot Device Empty', self._boot_device_empty),
                     ])

    if self._force_dfu and self._boot_device_empty == False:
      pretty_dfu_filename = pretty_dfu_filename_img4
    else:
      pretty_dfu_filename = ''

    settings.Append(
                    'Images',
                    [(pretty_filename, ''),
                     (pretty_dfu_filename, ''),
                     ])

    outcome_string = {kOutcomeBlinky:            'Blinky:             {} = {}'.format(kDefaultBlinkyGPIOReg, kDefaultBlinkyGPIOValue),
                      kOutcomeDfu:               'DFU:                {} = {}, {} = {}'.format(kDefaultUSBDCTLReg, kDefaultUSBDCTLValue, kDefaultDFUStatusGPIOReg, kDefaultDFUStatusGPIOValue),
                      kOutcomeBlinkyVcoOverride: 'Blinky:             {} = {}, {} = {}'.format(kDefaultBlinkyGPIOReg, kDefaultBlinkyGPIOValue, kDefaultPCIEVCODIGCTRLReg, self._fcal_vcodigctrl_out),
                      }[self._outcome]

    sidp_string = {kSiDPStateUnlocked:           "Unlocked & Invalid: {} = {}, SEAL_DATA_A(0..7) = don't care".format(kDefaultSiDPReg, kDefaultSiDPUnlocked),
                   kSiDPStateLockedInvalid:      "Locked & Invalid:   {} = {}, SEAL_DATA_A(0..7) = zero".format(kDefaultSiDPReg, kDefaultSiDPInvalid),
                   kSiDPStateLockedValid:        "Locked & Valid:     {} = {}, SEAL_DATA_A(0..7) = PMGR_SCRATCH_SCRATCH(12..19) = manifest".format(kDefaultSiDPReg, kDefaultSiDPValid),
                  }[self._sidp_state]

    if self._outcome == kOutcomeDfu:
      check_usb_clk = 'USB clock frequency'
      check_usb_clk_text = 'Verify USB clock running at spec frequency'
      check_usbotg = 'USB Device Mode Tunables'
      check_usbotg_text = '{} = {}, {} = {}'.format(kDefaultUSB20PHYCFG0Reg, kDefaultUSB20PHYCFG0Value, kDefaultUSB20PHYCFG1Reg, kDefaultUSB20PHYCFG1Value)
    else:
      check_usb_clk = ''
      check_usb_clk_text = ''
      check_usbotg = ''
      check_usbotg_text = ''

    # JTAG gets enabled on a production part if it is demoted.
    if self._fuse_production_status == True and self._demote_production == True and self._outcome == kOutcomeBlinky:
      check_jtag = 'JTAG'
      check_jtag_text = 'Starts disabled before test, but enabled after test'
      check_prod_mode = 'Production Mode'
      check_prod_mode_text = 'Disabled: MINIPMGR_FUSE_CFG_FUSE0.PRODUCTION_MODE @2_102bc000[0] = 0'
    else:
      check_jtag = ''
      check_jtag_text = ''
      check_prod_mode = ''
      check_prod_mode_text = ''

    if self._enable_keys == None:
        self._enable_keys = False

    keys_state = {True:  'Enabled:            MINIPMGR_SECURITY_SIO_AES_DISABLE @2_102d0000[31:0] = 0x00000000',
                  False: 'Disabled:           MINIPMGR_SECURITY_SIO_AES_DISABLE @2_102d0000[31:0] = 0x00000007'}[self._enable_keys]

    if self._lock_fuses == None:
        if self._outcome == kOutcomeDfu:
          self._lock_fuses = False
        else:
          self._lock_fuses = True

    fuses_locked_state = {True:  'Locked:             MINIPMGR_FUSE_CFG_FUSE1.AP_LOCK @2_102bc004[31] = 1',
                          False: 'Unlocked:           MINIPMGR_FUSE_CFG_FUSE1.AP_LOCK @2_102bc004[31] = 0'}[self._lock_fuses]

    # For LLB boot, boot nonce pre-set to 0xdeadbeefdeadbeef. For iBSS boot, set to 0xddccbbaa78563412
    if pretty_filename.find('llb') != -1 and self._force_dfu == 0:
      boot_nonce = 0xdeadbeefdeadbeef
    else:
      if self._restore_info_nonce is not None:
        boot_nonce = self._restore_info_nonce
      else:
        boot_nonce = kDefaultApNonceLittleEndian

    if self._force_dfu and self._outcome == kOutcomeBlinky:
      outcome_string += ' via iBSS in DFU mode'

    rom_state = {True:  'Enabled:            MINIPMGR_SECURITY_MCC_BOOTROMDIS @2_102d0010[0] = 0',
                 False: 'Disabled:           MINIPMGR_SECURITY_MCC_BOOTROMDIS @2_102d0010[0] = 1'}[self._outcome == kOutcomeDfu]

    settings.Append(
                    'Boot Nonce in PMGR Scratch Registers',
                    [('PMGR_SCRATCH_10', '0x%x' % (boot_nonce & 0xffffffff)),
                     ('PMGR_SCRATCH_11', '0x%x' % ((boot_nonce >> 32) & 0xffffffff)),
                     ])

    settings.Append(
                    'PASS',
                    [('Outcome', outcome_string),
                     ('ROM Access', rom_state),
                     ('Keys - UID and GIDs', keys_state),
                     ('Fuses', fuses_locked_state),
                     ('SiDP', sidp_string),
                     ('', ''),
                     ('Boot Core', 'CPU0 MPIDR_EL1.AFFINITY2 = MINIPMGR_FUSE_CFG_FUSE1.BOOT_PCORE @2_102bc004[18]'),
                     (check_usbotg, check_usbotg_text),
                     (check_usb_clk, check_usb_clk_text),
                     (check_prod_mode, check_prod_mode_text),
                     (check_jtag, check_jtag_text),
                     ])

    settings.Append(
                    'FAIL',
                    [('All PASS conditions not met', ''),
                     ])

    if self._manifest:
      settings.Append(
                      '(Informational only - Image4 Personalization Settings Used)',
                      [('Encryption', self._encryption),
                       ('Crypto Hash Method', self._crypto_hash_method),
                       ('Epoch', self._epoch),
                       ('Chip ID', self._chip_id),
                       ('Board ID', self._board_id),
                       ('ECID', self._ecid),
                       ('Production Status', self._production_status),
                       ('Security Mode', self._security_mode),
                       ('Enable Keys', self._enable_keys),
                       ('DPRO', self._demote_production),
                       ('DSEC', self._demote_secure),
                       ('Security Domain', self._security_domain),
                       ('Allow Mix and Match', self._allow_mix_and_match),
                       ('AP Boot Nonce Hash', self._ap_boot_nonce_hash),
                       ('Restore Info Nonce', self._restore_info_nonce),
                       ('SEP Boot Nonce Hash', self._sep_boot_nonce_hash),
                       ])

    settings.Log(self._log)
    return True

def main():
  help_str = ('Use the source.\n')
  class MyParser(optparse.OptionParser):
    def format_epilog(self, formatter):
      return self.epilog
  parser = MyParser(epilog=help_str)
  parser.add_option(
    '--platform',
    help='Override the default platform value of ' + kDefaultPlatform)
  parser.add_option(
    '--personalize_img4',
    help='Override path to personalize_img4 tool')
  parser.add_option(
    '--img4decodetest',
    help='Override path to img4decodetest tool')
  parser.add_option(
    '--bfn_maker',
    help='Override path to bfn_maker tool')
  (options, args) = parser.parse_args(sys.argv[1:])
  if args:
    sys.stderr.write(ErrorText('Unused trailing options: ' + ' '.join(args) + '\n'))
    return 1

  thisPlat = options.platform
  if thisPlat is None:
    thisPlat = kDefaultPlatform
  output_dir = kOutputDirPrefix + str(thisPlat)

  # Determine the chip_id and board_id based on the platform 
  if thisPlat not in kSupported_platforms:
    sys.stderr.write(ErrorText('Only the following platforms are supported by this script: ' + str(kSupported_platforms)))
    return 1
  elif thisPlat == kDefaultPlatform:
    thisChip_id = kDefaultChipId
    thisBoard_id = kDefaultBoardId
  else:
    assert false, "Unknown platform"

  # Prune any existing output and re-create the directory.
  shutil.rmtree(output_dir, ignore_errors=True)
  os.makedirs(output_dir)

  log = open(os.path.join(output_dir, kLogFileName), 'w')
  log.write('This file generated by iBoot/apps/SecureROM/test_generator_img4\n')
  log.write('Generated on: %s GMT\n\n' % time.asctime(time.gmtime()))

  # Result code
  rc = True

  # Test 1:
  # Boot from SPI NOR without a manifest in testmode. This facility is sometimes used in the factory
  # environment and the test is to ensure that we can successfully
  # boot in this environment. This is booting a non-secure image on a
  # non-secure part in testmode.
  rc &= PersonalizeImg4(manifest = False,
                        platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 1,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedInvalid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = False,
                        fuse_epoch = 0,
                        boot_device = 'NOR',
                        test_mode = True,
                        description = ('Boot image without manifest in testmode with SPI NOR')
                        ).Execute()

  # Test 2:
  # Boot from SPI NOR with manifest that disables keys. This is booting a non-secure
  # image on a non-secure part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 2,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = False,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = False,
                        boot_device = 'NOR',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = True,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = False,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot image with manifest that disables keys on an insecure part')
                        ).Execute()

  # Test 3:
  # Fail to boot a non-production image on a Production part in test mode.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 3,
                        outcome = kOutcomeDfu,
                        sidp_state = kSiDPStateUnlocked,
                        core_type = kCoreTypeECore,
                        image_source = kImageSourceNor,
                        fuse_production_status = True,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NOR',
                        test_mode = True,
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Fail to boot a non-production image on a Production part in test mode')
                        ).Execute()

  # Test 4:
  # Boot a production image on a production part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 4,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = True,
                        fuse_security_mode = True,
                        fuse_epoch = 1,
                        production_status = True,
                        security_mode = True,
                        epoch = kDefaultEpochForServerSigning,
                        boot_device = 'NOR',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = True,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot a production image on a production part')
                        ).Execute()

  # Test 5:
  # Test Epoch changes by having a chip epoch greater then the
  # certificate epoch, and verifying that the object is rejected.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 5,
                        outcome = kOutcomeDfu,
                        sidp_state = kSiDPStateUnlocked,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 2,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NOR',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Fail to boot an image with epoch less than chip epoch')
                        ).Execute()

  # Test 6:
  # Test USB DFU mode on a development device.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 6,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceDfu,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NOR',
                        boot_device_empty = True,
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        description = ('Boot from USB in DFU mode with boot device set to NOR, but NOR is empty')
                        ).Execute()

  # Test 7:
  # Test Force DFU when a valid image is in NOR. i.e. ignore valid NOR image
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 7,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NOR',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        force_dfu = 1,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from USB in DFU mode with boot device set to NOR, and valid image in NOR, but force dfu set')
                        ).Execute()

  # Test 8:
  # Boot an image in development mode on a production part (Demotion)
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 8,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = True,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = True,
                        security_mode = True,
                        epoch = kDefaultEpochForServerSigning,
                        boot_device = 'NOR',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = True,
                        demote_production = True,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot an image in development mode on a production part (Demotion)')
                        ).Execute()

  # Test 9:
  # Boot from NVMe on X2 PCIe link a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 9,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNVMe,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NVME0_X2',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from NVMe on PCIE0 in X2 mode a development image on a development part')
                        ).Execute()

  # Test 10:
  # Boot from NVMe on X1 PCIe link a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 10,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        core_type = kCoreTypePCore,
                        image_source = kImageSourceNVMe,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NVME0_X1',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from NVMe on PCIE0 in X1 mode a development image on a development part')
                        ).Execute()

  # Test 11:
  # Boot from NVMe on X2 PCIe link with L1SS disabled a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 11,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNVMe,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NVME0_X2',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from NVMe with L1SS disabled on PCIE0 in X2 mode a development image on a development part')
                        ).Execute()

  # Test 12:
  # Strapped for NVMe on X2 PCIe link but nothing connected to PCIe Port 0
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 12,
                        outcome = kOutcomeDfu,
                        sidp_state = kSiDPStateUnlocked,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'NVME0_X2',
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Strapped for NVMe on PCI00 boot but nothing connected to PCIe Port 0. Will time out after 10 seconds.')
                        ).Execute()

  # Test 13:
  # Boot from NOR (fast mode) a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 13,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'FAST_NOR',
                        test_mode = True,
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from SPI NOR in fast mode a development image on a development part')
                        ).Execute()

  # Test 14:
  # Boot from NOR (slow mode) a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 14,
                        outcome = kOutcomeBlinky,
                        sidp_state = kSiDPStateLockedValid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = True,
                        fuse_epoch = 0,
                        production_status = False,
                        security_mode = True,
                        boot_device = 'SLOW_NOR',
                        test_mode = True,
                        chip_id = thisChip_id,
                        board_id = thisBoard_id,
                        ecid = kDefaultEcid,
                        allow_mix_and_match = False,
                        ap_boot_nonce_hash = kDefaultApNonceHash,
                        enable_keys = True,
                        encryption = False,
                        restore_info_nonce = kDefaultApNonceLittleEndian,
                        description = ('Boot from SPI NOR in slow mode a development image on a development part')
                        ).Execute()

  # Test 15:
  # Boot from SPI NOR in test mode without a manifest in testmode with PCIe PLL VCO overrides.
  rc &= PersonalizeImg4(manifest = False,
                        platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 15,
                        outcome = kOutcomeBlinkyVcoOverride,
                        sidp_state = kSiDPStateLockedInvalid,
                        image_source = kImageSourceNor,
                        fuse_production_status = False,
                        fuse_security_mode = False,
                        fuse_epoch = 0,
                        fuse_cfg_fuse4 = 0x15,
                        boot_device = 'NOR',
                        test_mode = True,
                        description = ('Boot image without manifest in testmode from SPI NOR with with PCIe PLL VCO overrides')
                        ).Execute()

  log.write("In addition, provide a clock-tree dump (including spare clocks). Please pick a test from 1 to 9 AND a test from 10 to 11 for this dump.\n")

  log.close()

  if rc:
      sys.stdout.write(PassText('\nSuccess\n'))
      return 0
  else:
      sys.stderr.write(ErrorText('\nTest plan contains errors\n'))
      return 1

if __name__ == '__main__':
  sys.exit(main())
