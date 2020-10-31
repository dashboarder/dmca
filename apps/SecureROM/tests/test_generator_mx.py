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

kDefaultPlatform = 't8002'
kDefaultChipId = 0x8002
kDefaultBoardId = 0x3f
kDefaultBlinkyGPIOReg = '0x47500000'
kDefaultBlinkyGPIOValue = '0x70203'
kDefaultDFUGPIOReg = '0x47500220'
kDefaultDFUGPIOValue = '0x00478283'
kBoardIdStrapBits = 4
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
kSupported_platforms = ['t8002']

# Image sources
kImageSourceDfu = 1
kImageSourceNand = 2
kImageSourceNor = 3

# Outcomes
kOutcomeBlinky = 1
kOutcomeDfu = 2
kOutcomeBlinkyVcoOverride = 3
kOutcomeBlinkyVcoDefault = 4

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
               image_source = None,
               fuse_production_status = None,
               fuse_security_mode = None,
               fuse_security_domain = kSecurityDomainDarwin,
               fuse_epoch = None,
               fuse_ecid = kDefaultEcid,
               fuse_cfg_fuse9 = None,
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
    assert outcome in [kOutcomeBlinky, kOutcomeDfu, kOutcomeBlinkyVcoOverride, kOutcomeBlinkyVcoDefault]
    assert image_source in [kImageSourceDfu, kImageSourceNand, kImageSourceNor]
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
    self._image_source = image_source
    self._fuse_production_status = fuse_production_status
    self._fuse_security_mode = fuse_security_mode
    self._fuse_security_domain = fuse_security_domain
    self._fuse_epoch = fuse_epoch
    self._fuse_ecid = fuse_ecid
    if fuse_cfg_fuse9 is None:
        self._fuse_cfg_fuse9 = '0x00000000'
    else:
        assert (type(fuse_cfg_fuse9) == int)
        self._fuse_cfg_fuse9 = '0x%08x' % fuse_cfg_fuse9
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
      
        all_properties = {'OBJP': object_properties}
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
                            kImageSourceNor: ('illb', 'NOR')}[self._image_source]

    # FAST_NOR boot device is currenly only supported for test mode
    if self._boot_device in ['FAST_NOR', 'SLOW_NOR']:
        if self._test_mode == False:
            self._log.write('  Skipping: FAST_NOR/SLOW_NOR boot device requires test mode\n\n')
            sys.stderr.write(WarningText('Test %d: FAST_NOR/SLOW_NOR boot device requires test mode\n' % self._test_number))
            return False

    if self._boot_device == 'NOR':
        self._boot_config = '0x%x' % (0 + (1 if self._test_mode else 0))
    elif self._boot_device == 'NAND':
        self._boot_config = '0x%x' % (2 + (1 if self._test_mode else 0))
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
                     ('ECID_LO', '0x%08x' % (kDefaultEcid & 0xffffffff)),
                     ('ECID_HI', '0x%08x' % ((kDefaultEcid >> 32) & 0xffffffff)),
                     ('CFG_FUSE9', self._fuse_cfg_fuse9),
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

    outcome_string = {kOutcomeBlinky: 'Blinky - AP writes {} to {}'.format(kDefaultBlinkyGPIOValue, kDefaultBlinkyGPIOReg),
                      kOutcomeDfu: 'DFU - AP writes {} to {}'.format(kDefaultDFUGPIOValue, kDefaultDFUGPIOReg),
                      kOutcomeBlinkyVcoOverride: 'Blinky: SecureROM PLLs (4, CPU) VCO_RCTRL_OW & VCO_RCTRL_SEL overridden',
                      kOutcomeBlinkyVcoDefault: 'Blinky: SecureROM PLLs (4, CPU) VCO_RCTRL_OW & VCO_RCTRL_SEL default values',
                      }[self._outcome]

    if self._outcome == kOutcomeDfu:
      check_usb_clk = 'USB clock frequency'
      check_usbotg = 'USB20PHY_CFG0/USB20PHY_CFG1'
      check_usbotg_text = 'set to tunables values for Device mode'
    else:
      check_usb_clk = ''
      check_usbotg = ''
      check_usbotg_text = ''

    # JTAG gets enabled on a production part if it is demoted.
    if self._fuse_production_status == True and self._demote_production == True and self._outcome == kOutcomeBlinky:
      check_jtag = 'JTAG'
      check_jtag_text = 'Starts disabled before test, but enabled after test'
    else:
      check_jtag = ''
      check_jtag_text = ''


    keys_state = {True: 'Enabled',
                  False: 'Disabled',
                  None: 'Disabled'}[self._enable_keys]

    if self._outcome == kOutcomeDfu:
      fuses_locked_default = 'Unlocked (CFG_FUSE1.AP_LOCK = 0)'
    else:
      fuses_locked_default = 'Locked (CFG_FUSE1.AP_LOCK = 1)'

    fuses_locked_state = {True: 'Locked (CFG_FUSE1.AP_LOCK = 1)',
                          False: 'Unlocked (CFG_FUSE1.AP_LOCK = 0)',
                          None: fuses_locked_default}[self._lock_fuses]

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

    rom_state = {True:  'r/w access enabled',
                 False: 'r/w access disabled'}[self._outcome == kOutcomeDfu]

    settings.Append(
                    'Boot Nonce in PMGR Scratch Registers',
                    [('PMGR_SCRATCH_10', '0x%x' % (boot_nonce & 0xffffffff)),
                     ('PMGR_SCRATCH_11', '0x%x' % ((boot_nonce >> 32) & 0xffffffff)),
                     ])
                     
    settings.Append(
                    'PASS',
                    [('Outcome', outcome_string),
                     ('ROM', rom_state),
                     ('Keys - UID and GIDs', keys_state),
                     ('Fuses', fuses_locked_state),
                     (check_usb_clk, ''),
                     (check_usbotg, check_usbotg_text),
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
  # Boot a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                  options = options,
                  log = log,
                  test_number = 4,
                  outcome = kOutcomeBlinky,
                  image_source = kImageSourceNand,
                  fuse_production_status = False,
                  fuse_security_mode = True,
                  fuse_epoch = 0,
                  production_status = False,
                  security_mode = True,
                  boot_device = 'NAND',
                  chip_id = thisChip_id,
                  board_id = thisBoard_id,
                  ecid = kDefaultEcid,
                  allow_mix_and_match = False,
                  ap_boot_nonce_hash = kDefaultApNonceHash,
                  enable_keys = True,
                  encryption = False,
                  restore_info_nonce = kDefaultApNonceLittleEndian,
                  description = ('Boot a development image on a development part')
                  ).Execute()

  # Test 5:
  # Boot a production image on a production part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 5,
                        outcome = kOutcomeBlinky,
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

  # Test 6:
  # Test Epoch changes by having a chip epoch greater then the
  # certificate epoch, and verifying that the object is rejected.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 6,
                        outcome = kOutcomeDfu,
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

  # Test 7:
  # Test USB DFU mode on a development device.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 7,
                        outcome = kOutcomeBlinky,
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

  # Test 8:
  # Test Force DFU when a valid image is in NOR. i.e. ignore valid NOR image
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 8,
                        outcome = kOutcomeBlinky,
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

  # Test 9:
  # Boot an image in development mode on a production part (Demotion)
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 9,
                        outcome = kOutcomeBlinky,
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

  # Test 10:
  # Boot from NOR (slow mode) a development image on a development part.
  rc &= PersonalizeImg4(platform = thisPlat,
                        options = options,
                        log = log,
                        test_number = 10,
                        outcome = kOutcomeBlinky,
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


  log.write("In addition, provide a clock-tree dump (including spare clocks). Please pick a test from 1 to 10 for this dump.\n")

  log.close()

  if rc:
      sys.stdout.write(PassText('\nSuccess\n'))
      return 0
  else:
      sys.stderr.write(ErrorText('\nTest plan contains errors\n'))
      return 1

if __name__ == '__main__':
  sys.exit(main())
