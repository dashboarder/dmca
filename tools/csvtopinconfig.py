#!/usr/bin/python
import argparse
import collections
import csv
import pipes
import sys

try:
    import openpyxl
except ImportError:
    openpyxl = None

class ParseError(Exception): pass

'''
Representation of one pin in the GPIO config
'''
class BasePin(object):
    default_drive = 'X1'
    valid_pupdn = set(["PULL_UP", "PULL_DOWN", ""])
    valid_drive_strength = set()
    def __init__(self, gpio_pin_number):
        self.gpio_pin_number = int(gpio_pin_number)
        self.bump_name = None
        self.net_name = None
        self.function = None
        self.drive_strength = 'X1'
        self.pupd = None
        self.bus_hold = None
        self.input_mode = None
        self.row_number = None
        self.slew_rate = ''
        self.rom = None

    @property
    def non_empty_props(self):
        if self.drive_strength == self.default_drive:
            drive_strength = ''
        else:
            drive_strength = "DRIVE_{}".format(self.drive_strength)
        
        if self.slew_rate == '':
            slew_rate = ''
        else:
            slew_rate = "{}_SLEW".format(self.slew_rate)

        return [i for i in [self.function, self.pupd, drive_strength, slew_rate, self.bus_hold, self.input_mode] if i]

    def to_enum_key(self):
        return 'GPIOCFG_' + '_'.join(self.non_empty_props)

    def to_enum_value(self):
        return ' | '.join(self.non_empty_props)

    def to_pinconfig_line(self, use_enum):
        if None in (self.bump_name, self.net_name, self.function, self.drive_strength, self.pupd):
            raise ParseError, "Some attributes were unset for pin %d: %s" % (self.gpio_pin_number, (self.bump_name, self.net_name, self.function, self.drive_strength, self.pupd))

        if use_enum:
            args = '{},'.format(self.to_enum_key())
        else:
            args = '{},'.format(self.to_enum_value())

        gpio_comment = '// {:>3} : {}'.format(self.gpio_pin_number, self.bump_name)
        net_name_comment = '-> {}'.format(self.net_name)
        return [args, gpio_comment, net_name_comment]

    def set_function(self, fn):
        cfg_map = { 'in':          'CFG_IN',
                    'input':       'CFG_IN',
                    'out0':        'CFG_OUT_0',
                    'out 0':       'CFG_OUT_0',
                    'out1':        'CFG_OUT_1',
                    'out 1':       'CFG_OUT_1',
                    'alt func 0':  'CFG_FUNC0',
                    'alt func 1':  'CFG_FUNC1',
                    'alt func 2':  'CFG_FUNC2',
                    'int rising':  'CFG_IN',
                    'int falling': 'CFG_IN',
                    'int any':     'CFG_IN',
                    'disable':     'CFG_DISABLED',
                    'disabled':    'CFG_DISABLED',
                    # The numbering offset isn't a typo, these are legacy
                    'function 1':  'CFG_FUNC0',
                    'function 2':  'CFG_FUNC1',
                    'function 3':  'CFG_FUNC2',
                  }
        self.function = cfg_map[fn.lower()]

    def set_net_name(self, name):
        self.net_name = name.strip()

    def set_bump_name(self, name):
        self.bump_name = name.strip()

    def set_drive_strength(self, strength):
        if strength == '':
            strength = self.default_drive
        else:
            strength = strength.upper()

        if strength.isdigit():
            strength = self.drive_strength_prefix + strength

        if not strength in self.valid_drive_strength:
            sorted_drive_strengths = sorted(self.valid_drive_strength, key=lambda item: (int(item[1:])))
            raise ParseError, "drive strength '{}' not valid (options are {})".format(strength.upper(), sorted_drive_strengths)
        self.drive_strength = strength

    def set_pupd(self, pupd):
        pupd_map = { 'pullup strong' :     'PULL_UP_STRONG',
                     'spu' :     'PULL_UP_STRONG',
                     'pullup':   'PULL_UP',
                     'pu':       'PULL_UP',
                     'pulldown': 'PULL_DOWN',
                     'pd':       'PULL_DOWN',
                     'disable':  '',
                     '':         ''}
        if not pupd_map[pupd.lower()] in self.valid_pupdn:
            pretty_pupdn = filter(lambda x : pupd_map[x] in self.valid_pupdn , pupd_map.keys())
            raise ParseError, "pu/pd value '{}' not valid (options are {})".format(pupd, pretty_pupdn)
        self.pupd = pupd_map[pupd.lower()]


class PinV1(BasePin):
    valid_drive_strength = set(["X1", "X2", "X3", "X4"])
    drive_strength_prefix = 'X'

    def set_bus_hold(self, hold):
        bus_hold_map = { '': '', '0': '', None: '', 'y': 'BUS_HOLD', 'yes': 'BUS_HOLD', '1': 'BUS_HOLD'}
        self.bus_hold = bus_hold_map[hold.lower()]

class PinV2(BasePin):
    valid_drive_strength = set(["X1", "X2", "X4", "X6"])
    drive_strength_prefix = 'X'

    def set_bus_hold(self, hold):
        bus_hold_map = { '': '', '0': '', None: '', 'y': 'BUS_HOLD', 'yes': 'BUS_HOLD', '1': 'BUS_HOLD'}
        self.bus_hold = bus_hold_map[hold.lower()]

class PinV3(BasePin):
    valid_drive_strength = set(["X1", "X2", "X4", "X6"])
    drive_strength_prefix = 'X'
    
    def set_slew_rate(self, slew):
        slew_map = { '': '', None: '', "fast": "FAST", "slow": "SLOW" }
        self.slew_rate = slew_map[slew.lower()]

class PinV4(PinV3):
    def set_input_mode(self, mode):
        mode_map = { '': '', None: '', 'cmos': '', 'schmitt': 'INPUT_SCHMITT', 'Schmitt': 'INPUT_SCHMITT'}
        self.input_mode = mode_map[mode]

class PinV5(PinV4):
    default_drive = 'S0'
    valid_drive_strength = set(["S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "S10", "S11", "S12", "S13", "S14", "S15"])
    drive_strength_prefix = 'S'
    valid_pupdn = set(["PULL_UP_STRONG", "PULL_UP", "PULL_DOWN", ""])

    def set_slew_rate(self, slew):
        slew_map = { '': '', None: '', "fast": "FAST", "slow": "SLOW", "very fast": "VERY_FAST" }
        self.slew_rate = slew_map[slew.lower()]


class ReservedPin(object):
    def to_pinconfig_line(self, use_enum):
        return ['{},'.format(self.to_enum_key() if use_enum else self.to_enum_value())]

    def to_enum_key(self):
        return 'GPIOCFG_CFG_DISABLED'

    def to_enum_value(self):
        return 'CFG_DISABLED'

class GPIODomainInfo(object):
    def __init__(self, offset, count, reserved):
        self.id_offset = offset
        self.pin_count = count
        self.reserved_pins = reserved

class GPIODomain(object):
    def __init__(self, info, pin_class):
        self.id_offset = info.id_offset
        self.pin_count = info.pin_count
        self.reserved_pins = info.reserved_pins

        # GPIO domains must start and end on 8-pin boundaries
        assert self.id_offset % 8 == 0
        if self.pin_count % 8 != 0:
            new_pin_count = self.pin_count + 8 - self.pin_count % 8
            self.reserved_pins |= set(range(self.id_offset + self.pin_count, self.id_offset + new_pin_count))
            self.pin_count = new_pin_count

        self.pins = [None] * self.pin_count

        for i in range(self.pin_count):
            if not i + self.id_offset in self.reserved_pins:
                pin = pin_class(i)
                pin.set_bump_name("UNSPECIFIED")
                pin.set_net_name("UNSPECIFIED")
                pin.set_function("disable")
                pin.set_pupd("disable")
                pin.set_drive_strength('')
            else:
                pin = ReservedPin()
            self.pins[i] = pin

    def has_pin_id(self, pin_id):
        return pin_id >= self.id_offset and pin_id < self.id_offset + self.pin_count

    def get_pin(self, pin_id):
        if not self.has_pin_id(pin_id):
            raise IndexError
        return self.pins[pin_id - self.id_offset]

class BasePinConfig(object):
    # Should be overridden in subclasses to a subclass of BasePin
    pin_class = None
    domain_info = None

    def __init__(self):
        self.domains = [GPIODomain(info, self.pin_class) for info in self.domain_info]

    def get_pin(self, pin_id):
        for d in self.domains:
            if d.has_pin_id(pin_id):
                return d.get_pin(pin_id)
        raise IndexError

    def array_size(self, domain_index):
        if domain_index == 0:
            count_label = ''
        else:
            count_label = '_{}'.format(domain_index)
        return 'GPIO{}_GROUP_COUNT * GPIOPADPINS'.format(count_label)

    def array_declaration(self, array_type, name, domain_index):
        if domain_index == 0:
            count_label = ''
        else:
            count_label = '_{}'.format(domain_index)
        templ = 'static const {} {}_{}[{}] = {{'
        return templ.format(array_type, name, domain_index, self.array_size(domain_index))

    def to_size_array(self, max_domains):
        lines = []
        lines += ['static const uint32_t controller_pins[GPIOC_COUNT] = {']
        for domain_idx in range(min(len(self.domains), max_domains)):
            lines += ['\t{},'.format(self.array_size(domain_idx))]
        lines += ['};']
        return '\n'.join(lines)

    def to_pinconfig_struct(self, name, array_type, max_domains, use_enum):
        lines = []

        for domain_idx in range(min(len(self.domains), max_domains)):
            domain = self.domains[domain_idx]

            pin_matrix = [domain.pins[i].to_pinconfig_line(use_enum) for i in range(domain.pin_count)]
            pin_matrix = tab_align(pin_matrix, min_column_tabs={0: 8})

            lines.append(self.array_declaration(array_type, name, domain_idx))

            rownum = 0
            for row in pin_matrix:
                if (rownum % 8) == 0:
                    lines.append("\n/* Port {:2} */".format(rownum/8))
                lines.append("\t"+"".join(row))
                rownum += 1
            lines.append("};")
            lines.append('')
        return "\n".join(lines)

    def to_enum_items(self):
        items = set()

        for domain in self.domains:
            for pin in domain.pins:
                items.add((pin.to_enum_key(), pin.to_enum_value()))
        return items

class BasePinConfigV1(BasePinConfig):
    array_type = 'uint32_t'
    pin_class = PinV1

class BasePinConfigV2(BasePinConfig):
    array_type = 'uint32_t'
    pin_class = PinV2

class BasePinConfigV3(BasePinConfig):
    array_type = 'uint32_t'
    pin_class = PinV3

class BasePinConfigV4(BasePinConfig):
    array_type = 'uint32_t'
    pin_class = PinV4

class BasePinConfigV5(BasePinConfig):
    array_type = 'uint32_t'
    pin_class = PinV5

class H4APinConfig(BasePinConfigV1):
    domain_info = [GPIODomainInfo(offset=0, count=208, reserved=set())]

class H5PPinConfig(BasePinConfigV2):
    domain_info = [GPIODomainInfo(offset=0, count=232, reserved=set())]

class H5GPinConfig(BasePinConfigV2):
    reserved_pins = set(range(14, 32) + range(140, 160) + range(212, 224) + range(241, 248))
    domain_info = [GPIODomainInfo(offset=0, count=248, reserved=reserved_pins)]

class AlcatrazPinConfig(BasePinConfigV3):
    reserved_pins = set(range(24, 32) + range(45, 64) + range(151, 160) + range(173, 192))
    domain_info = [GPIODomainInfo(offset=0, count=195, reserved=reserved_pins)]

class FijiPinConfig(BasePinConfigV4):
    reserved_pins = set(range(16, 32) + range(88, 96) + range(140, 160))
    domain_info = [GPIODomainInfo(offset=0, count=204, reserved=reserved_pins)]

class CapriPinConfig(BasePinConfigV4):
    reserved_pins = set(range(16, 32))
    domain_info = [GPIODomainInfo(offset=0, count=184, reserved=reserved_pins)]

class M7PinConfig(BasePinConfigV4):
    reserved_pins = set(range(10, 32))
    domain_info = [GPIODomainInfo(offset=0, count=138, reserved=reserved_pins),
                   GPIODomainInfo(offset=160, count=50, reserved=set())]

class MauiPinConfig(BasePinConfigV4):
    reserved_pins = set(range(55, 64) + range(92, 96) + range(152, 160) + range(168, 192))
    domain_info = [GPIODomainInfo(offset=0, count=199, reserved=reserved_pins),
                   GPIODomainInfo(offset=288, count=28, reserved=set())]

class ElbaPinConfig(BasePinConfigV5):
    reserved_pins = set(range(37, 64) + range(125, 128) + range(142, 160))
    domain_info = [GPIODomainInfo(offset=0, count=220, reserved=reserved_pins),
                   GPIODomainInfo(offset=288, count=28, reserved=set())]

class CaymanPinConfig(BasePinConfigV5):
    ap_reserved_pins = set(range(55, 64) + range(92, 96) + range(104, 128) + range(155, 160) + range(188, 192))
    aop_reserved_pins = set(range(318, 320) + range(330, 352))
    domain_info = [GPIODomainInfo(offset=0, count=208, reserved=ap_reserved_pins),   # AP
                   GPIODomainInfo(offset=288, count=85, reserved=aop_reserved_pins), # AOP
                   GPIODomainInfo(offset=256, count=7, reserved=set())]              # SEP
    output_domains = 2

class M8PinConfig(BasePinConfigV5):
    ap_reserved_pins = set(range(138, 160))
    aop_reserved_pins = set(range(264, 288))
    domain_info = [GPIODomainInfo(offset=32, count=139, reserved=ap_reserved_pins),  # AP
                   GPIODomainInfo(offset=256, count=96, reserved=aop_reserved_pins), # AOP
                   GPIODomainInfo(offset=0, count=7, reserved=set()),
                   GPIODomainInfo(offset=192, count=96, reserved=set())]
    output_domains = 2

def excel_reader(filename, sheet):
    try:
        workbook = openpyxl.load_workbook(filename, data_only=True)
    except IOError:
        sys.stderr.write('Could not open workbook {}\n'.format(filename))
        return None
    try:
        worksheet = workbook.get_sheet_by_name(sheet)
    except KeyError:
        sys.stderr.write('Could not open worksheet {}\n'.format(sheet))
        return None

    def row_generator(ws):
        def cell_to_str(cell):
            value = cell.value
            if value is None:
                value = ''
            return unicode(value)

        for row in ws.rows:
            yield [cell_to_str(cell) for cell in row]

    return row_generator(worksheet)

def try_parse_hex(i):
    try:
        return int(i, 16)
    except ValueError:
        return None

def row_is_empty(row):
    for col in row:
        if not col is None and len(col.strip()) > 0:
            return False
    return True

def row_is_footer(row):
    for col in row:
        if not col is None:
            stripped = col.strip()
            if len(stripped) > 0 and stripped != '--':
                return False
    return True

class CSVParser(object):
    def __init__(self, pinconfig_class, per_target_headers):
        self.pinconfig_class = pinconfig_class
        self.per_target_headers = per_target_headers
        self.targets = per_target_headers.keys()
        # Map the standardized name for a property to a list of acceptable names
        self.header_name_map = { 'gpio_id':             ['gpio id', 'id'],
                                 'net_name':            ['net name', 'net names'],
                                 'bump_name':           ['ball name', 'pad name'],
                                 'configurable':        ['gpio configurable'],
                                 'drive_strength':      ['drive strength', 'drive strength (ma)'],
                                 'drive_strength_ap':   ['drive strength form factor', 'drive strength ap'],
                                 'drive_strength_dev':  ['drive strength dev'],
                                 'pupd':                ['pu/pd', 'pullup/pulldown', 'pull-up/pull-down'],
                                 'function':            ['config', 'dir', 'SecureROM config'],
                                 'rom':                 ['rom function']}

        if callable(getattr(self.pinconfig_class.pin_class, 'set_slew_rate', None)):
            self.header_name_map['slew_rate'] =         ['slew rate', 'gpio_sr']
            self.header_name_map['slew_rate_ap'] =      ['slew rate form factor', 'gpio_sr ap']
            self.header_name_map['slew_rate_dev'] =     ['slew rate dev', 'gpio_sr dev']

        if callable(getattr(self.pinconfig_class.pin_class, 'set_bus_hold', None)):
            self.header_name_map['bus_hold'] =          ['bus hold', 'gpio_bh']

        if callable(getattr(self.pinconfig_class.pin_class, 'set_input_mode', None)):
            self.header_name_map['input_mode'] =        ['input mode', 'input select', 'gpio_is']

    def create_pinconfig(self):
        return self.pinconfig_class()

    def parse_version(self, row):
        for col in row:
            lowercase_col = col.lower()
            if 'rev' in lowercase_col or 'version' in lowercase_col or 'ver' in lowercase_col:
                return lowercase_col
        return None

    def canonical_header(self, header):
        for canonical, options in self.header_name_map.iteritems():
            if header.lower() in options:
                return canonical
        return None

    def find_headers(self, row, rom):
        lowercase_row = map(lambda x: x.lower(), row)
        columns_per_target = {}
        columns_per_config = collections.defaultdict(dict)
        per_config_columns = set()
        ignore_missing_keys = set()
        missing_keys = set()

        # create a mapping from canonical header names to column indices
        header_map = {}
        for idx in range(len(lowercase_row)):
            header = lowercase_row[idx]
            canonical = self.canonical_header(header)
            if canonical is None:
                continue
            header_map[canonical] = idx

        # some parameters can be specified per config (ap/dev). figure out
        # which if any are done that way for this spreadsheet
        config_names = ['ap', 'dev']
        for key in ['drive_strength', 'slew_rate', 'net_name']:
            config_to_key_map = {x: '{}_{}'.format(key, x) for x in config_names}
            per_config_keys = set(config_to_key_map.values())

            if per_config_keys <= set(header_map):
                assert True, 'error message' # XXX
                # used in the loop below to skip over columns that are per config
                per_config_columns.update(per_config_keys)
                # passed back to the caller for use in parsing rows
                for config in config_names:
                    columns_per_config[config][key] = header_map[config_to_key_map[config]]
                ignore_missing_keys.add(key)
            elif not per_config_keys.isdisjoint(set(header_map)):
                # either all or none of per-config columns should be present
                found = ', '.join(sorted(set(header_map) & per_config_keys))
                raise ParseError('All or none of the per-config {} columns must be specified (found {})'.format(key, found))
            else:
                # this key isn't per config, so fall through to the logic below, but
                # let the missing column checker know the per-config columns won't be present
                ignore_missing_keys.update(per_config_keys)

        # If no per-config columns were set, then there's only one configuration.
        # Set up the key map accordingly
        if len(columns_per_config) == 0:
            for key in ['drive_strength', 'slew_rate']:
                columns_per_config['all'][key] = header_map[key]

        for target in self.targets:
            target_header_name_map = self.per_target_headers[target]
            columns_per_target[target] = {}
            for key in self.header_name_map.keys():
                if key in per_config_columns:
                    continue
                if key in target_header_name_map:
                    header = target_header_name_map[key]
                    # This value has a column per target, so find the per-target header in the row
                    try:
                        idx = lowercase_row.index(header.lower())
                    except ValueError:
                        sys.stderr.write('Could not find target {}\'s "{}" column "{}"\n'.format(target, key, header))
                        missing_keys.add(key)
                    columns_per_target[target][key] = idx
                else:
                    # This value has one column for all targets, so find one of the standard headers in the row
                    for header in self.header_name_map[key]:
                        if header.lower() in lowercase_row:
                            idx = lowercase_row.index(header.lower())
                            columns_per_target[target][key] = idx
                            break
                    else:
                        missing_keys.add(key)

        if rom:
            for key, value in target_header_name_map.iteritems():
                if key == 'function' and value.lower() != 'SecureROM Config'.lower():
                    sys.stderr.write('ROM pins should have SecureROM Config column defined\n')
                    missing_keys.add(key)

        # Some columns are optional
        missing_keys -= set(['configurable', 'input_mode'])
        # ROM Function is optional if not --rom
        if not rom:
            missing_keys -= set(['rom'])
        # Marked as being unneeded by the per-config column logic
        missing_keys -= ignore_missing_keys

        if len(missing_keys) == 0:
            missing_keys = None

        return columns_per_target, columns_per_config, missing_keys

    def parse(self, filename, force=False, rom=False, sheet=None):
        if filename.endswith('.xlsx'):
            if openpyxl is None:
                sys.stderr.write('To import Excel sheets, install the openpyxl package\n')
                return None, None
            if sheet is None:
                sys.stderr.write('To import Excel sheets, specify the --sheet option\n')
                return None, None
            reader = excel_reader(filename, sheet)
            if reader is None:
                return None, None
        else:
            reader = csv.reader(open(filename, "rU"))
        cnt=0
        row_number = 0

        # top row has version number
        row0 = reader.next()
        row_number += 1
        # next non-blank row has headers
        while True:
            row1 = reader.next()
            row_number += 1
            if len(filter(lambda x: len(x.strip()) > 0, row1)) > 0:
                break

        version = self.parse_version(row0)
        if version is None:
            sys.stderr.write('Could not find version in 1st row\n')
            return None, None

        try:
            columns_per_target, columns_per_config, missing = self.find_headers(row1, rom)
        except ParseError as e:
            sys.stderr.write('{}\n'.format(e))
            return None, None

        if not missing is None:
            missing_options = {"{} ({})".format(col, " or ".join(self.header_name_map[col])) for col in missing}
            sys.stderr.write('Missing columns:\n{}\n'.format('\n    '.join(missing_options)))
            return None, None

        if len(columns_per_config) == 1:
            configs = ['all']
        else:
            configs = ['ap', 'dev']

        pinconfigs = {}
        for target in self.targets:
            pinconfigs[target] = {}
            for config in configs:
                pinconfigs[target][config] = self.create_pinconfig()

        have_error = False

        for row in reader:
            row_number += 1
            if row_is_empty(row):
                continue
            if row_is_footer(row):
                break

            for target, target_columns in columns_per_target.iteritems():
                for config, config_columns in columns_per_config.iteritems():
                    columns = {}
                    columns.update(config_columns)
                    columns.update(target_columns)

                    try:
                        parsed_props = {prop: row[column].strip() for prop, column in columns.iteritems()}

                        # If the optional 'GPIO Configurable' column is present, make sure it says y or yes
                        try:
                            if not parsed_props['configurable'].lower() in ('y', 'yes'):
                                continue
                            # And then get rid of the column, because the code below can't do anything with it
                            del parsed_props['configurable']
                        except KeyError:
                            pass

                        try:
                            gpio_id = int(parsed_props['gpio_id'])
                        except ValueError:
                            if rom and parsed_props['gpio_id'] == '':
                                continue
                            else:
                                raise ParseError('Invalid GPIO ID "{}"'.format(parsed_props['gpio_id']))

                        pinconfig = pinconfigs[target][config]
                        try:
                            pin = pinconfig.get_pin(gpio_id)
                        except IndexError:
                            raise ParseError('GPIO ID {} is out of range'.format(gpio_id))

                        if isinstance(pin, ReservedPin):
                            raise ParseError('GPIO ID {} is reserved pin'.format(gpio_id))
                        elif not pin.row_number is None:
                            raise ParseError('GPIO ID {} redefined (previously defined on row {})'.format(pin.gpio_id, pin.row_number))
                        pin.row_number = row_number

                        # The setters for all of the columns are predictable based on their name
                        for prop, value in parsed_props.iteritems():
                            if rom:
                                if prop == 'function':
                                    if parsed_props['rom'].lower() in ('y', 'yes'):
                                        value = parsed_props['function']
                                    else:
                                        value = 'disabled'
                                if prop == 'net_name':
                                    if not parsed_props['rom'].lower() in ('y', 'yes'):
                                        value = ''
                            if prop != 'gpio_id' and prop != 'rom':
                                setter = getattr(pin, 'set_' + prop)
                                try:
                                    setter(value)
                                except KeyError:
                                    raise ParseError('Invalid value "{}" for key {} in column "{}"'.format(value, prop, row1[columns[prop]]))
                    except ParseError as e:
                        sys.stderr.write('Row {:3}, Config {:3}: {}\n'.format(row_number, config, e))
                        have_error = True

        if not have_error or force:
            return version, pinconfigs
        else:
            return None, None

def tab_align(data_matrix, tabwidth=8, min_spacing_slack=1, min_column_tabs={}):
    def rounduptab(x):
        return int(((x + float(tabwidth-1))/tabwidth))*tabwidth
    def tab(x):
        if x % tabwidth:
            return rounduptab(x)
        else:
            return x+tabwidth
    num_cols = max(map(len, data_matrix))

    # Compute max number of characters seen for each column
    max_char_counts = [0]*num_cols
    for column, min_width in min_column_tabs.iteritems():
        max_char_counts[column] = min_width*tabwidth
    def grow_count(new_row):
        for i in range(len(new_row)):
            max_char_counts[i] = max(max_char_counts[i], rounduptab(len(new_row[i]) + min_spacing_slack))
    for row in data_matrix:
        grow_count(row)

    def pad_tabs(item, required_length):
        length = len(item)
        tabcount = 0
        while length < required_length:
            tabcount+=1
            length=tab(length)
        return "%s%s" % (item, '\t'*tabcount)
    def format_row(row):
        return "".join([pad_tabs(a, b) for (a,b) in zip(row, max_char_counts)]).rstrip()
    return [ format_row(x) for x in data_matrix ]

def print_pinconfigs(target_config_pinconfigs, pinconfig_class, version, radar, copyright, commandline, prefix, rom, header):
    array_type = pinconfig_class.array_type
    num_domains = len(pinconfig_class.domain_info)
    if hasattr(pinconfig_class, 'output_domains'):
        if pinconfig_class.output_domains < num_domains:
            num_domains = pinconfig_class.output_domains

    targets = sorted(target_config_pinconfigs.keys())
    configs = sorted(target_config_pinconfigs[targets[0]].keys())

    if header:
        pinconfig_prefix = 'gpio_default_cfg'
    else:
        pinconfig_prefix = 'pinconfig'

    if not prefix is None:
        pinconfig_prefix = "{}_{}".format(pinconfig_prefix, prefix)

    if rom:
        assert header
        assert configs == ['all']
        assert len(targets) == 1
        assert len(configs) == 1
    else:
        assert configs == ['ap', 'dev'] or configs == ['all']

    print "/*"
    print " * Copyright (C) {} Apple Inc. All rights reserved.".format(copyright)
    print " *"
    print " * This document is the property of Apple Inc."
    print " * It is considered confidential and proprietary."
    print " *"
    print " * This document may not be reproduced or transmitted in any form,"
    print " * in whole or in part, without the express written permission of"
    print " * Apple Inc."
    print " */"
    print ""
    print "/* THIS FILE IS AUTOMATICALLY GENERATED BY tools/csvtopinconfig.py. DO NOT EDIT!"
    print "   I/O Spreadsheet version: {}".format(version)
    print "   I/O Spreadsheet tracker: {}".format(radar)
    print "   Conversion command: {}".format(commandline)
    print "*/"
    print ""

    if header:
        print "#ifndef __PLATFORM_PINCONFIG_H"
        print "#define __PLATFORM_PINCONFIG_H"
        print ""
    else:
        print "#include <debug.h>"
        print "#include <drivers/apple/gpio.h>"
        print "#include <platform.h>"
        print "#include <platform/soc/hwregbase.h>"
        print "#include <stdint.h>"
        if len(targets) > 1:
            print "#include <target/boardid.h>"
        print ""

        # Create a mapping from enumeration values (8-bit) to the real value for the pin (32-bit)
        enum_map = {}
        for target in targets:
            for config in configs:
                enum_items = target_config_pinconfigs[target][config].to_enum_items()
                for key, value in enum_items:
                    if key not in enum_map:
                        enum_map[key] = value
                    else:
                        assert(enum_map[key] == value)

        assert(len(enum_map) < 256)

        # print the enumeration that will be used to index into the map
        print 'enum {'
        for key in sorted(enum_map.keys()):
            print '\t{},'.format(key)
        print '};\n\n'

        # print the map itself
        print 'static const {} enum_map[] = {{'.format(array_type)
        for key in sorted(enum_map.keys()):
            print '\t[{}] = {},'.format(key, enum_map[key])
        print '};\n\n'

    # print the pinconfigs, using the enumeration keys as the array items
    for target in targets:
        for config in configs:
            if len(targets) == 1:
                if len(configs) == 1:
                    name = pinconfig_prefix
                else:
                    name = '{}_{}'.format(pinconfig_prefix, config.lower())
            else:
                if len(configs) == 1:
                    name = '{}_{}'.format(pinconfig_prefix, target.lower())
                else:
                    name = '{}_{}{}'.format(pinconfig_prefix, target.lower(), config.lower())
            print target_config_pinconfigs[target][config].to_pinconfig_struct(name, 'uint8_t', num_domains, not header)

    if header:
        print ""
        print "#endif /* __PLATFORM_PINCONFIG_H */"
    else:
        print 'struct pinconfig_map {'
        print '\tuint32_t board_id;'
        print '\tuint32_t board_id_mask;'
        print '\tconst uint8_t *pinconfigs[GPIOC_COUNT];'
        print '};'
        print ''
        print 'static const struct pinconfig_map cfg_map[] = {'
        if len(targets) == 1:
            if len(configs) == 1:
                # just one target and one config, so make a map that spits out the same pinconfig
                # for all board IDs
                pinconfigs = ', '.join(map(lambda x: '{}_{}'.format(pinconfig_prefix, x), range(num_domains)))
                print '\t{{ 0, 0, {{ {} }} }},'.format(pinconfigs)
            elif len(configs) == 2:
                # One target, but ap/dev configs, so make a map that selects ap/dev based on the lsb
                # of the board ID
                pinconfigs = ', '.join(map(lambda x: '{}_ap_{}'.format(pinconfig_prefix, x), range(num_domains)))
                print '\t{{ 0, 1, {{ {} }} }},'.format(pinconfigs)
                pinconfigs = ', '.join(map(lambda x: '{}_dev_{}'.format(pinconfig_prefix, x), range(num_domains)))
                print '\t{{ 1, 1, {{ {} }} }},'.format(pinconfigs)
            else:
                assert False, 'Can\'t handle more than 2 configs per target'
        else:
            for target in targets:
                if len(configs) == 1:
                    pinconfigs = ', '.join(map(lambda x: '{}_{}_{}'.format(pinconfig_prefix, target.lower(), x), range(num_domains)))
                    print '\t{{ TARGET_BOARD_ID_{}AP,  ~1, {{ {} }}  }},'.format(target.upper(), pinconfigs)
                elif len(configs) == 2:
                    pinconfigs = ', '.join(map(lambda x: '{}_{}ap_{}'.format(pinconfig_prefix, target.lower(), x), range(num_domains)))
                    print '\t{{ TARGET_BOARD_ID_{}AP,  ~0, {{ {}  }} }},'.format(target.upper(), pinconfigs)
                    pinconfigs = ', '.join(map(lambda x: '{}_{}dev_{}'.format(pinconfig_prefix, target.lower(), x), range(num_domains)))
                    print '\t{{ TARGET_BOARD_ID_{}DEV, ~0, {{ {} }} }},'.format(target.upper(), pinconfigs)
                else:
                    assert False, 'Can\'t handle more than 2 configs per target'
        print '};'
        print ''
        print 'static {} *expanded_pinconfigs[GPIOC_COUNT];'.format(array_type)
        print ''
        print target_config_pinconfigs[targets[0]][configs[0]].to_size_array(num_domains)
        print ''
        if prefix is None:
            print 'const {} * target_get_default_gpio_cfg(uint32_t gpioc)'.format(array_type)
        else:
            print 'const {} * target_get_{}_gpio_cfg(int gpioc)'.format(array_type, prefix)
        print '{'
        print '\tstatic const struct pinconfig_map *selected_map = NULL;'
        print ''

        print '\t/* Cannot use malloc as chunk manager is yet to be intialized */'

        for domain_idx in range(num_domains):
            if domain_idx == 0:
                count_label = ''
            else:
                count_label = '_{}'.format(domain_idx)
            print '\tstatic {} pinconfig_buf{}[GPIO{}_GROUP_COUNT * GPIOPADPINS];'.format(array_type, count_label, count_label)
            print '\texpanded_pinconfigs[{}] = pinconfig_buf{};'.format(domain_idx, count_label)

        print ''
        print '\tASSERT(gpioc < GPIOC_COUNT);'
        print ''
        print '\tif (selected_map == NULL) {';
        print '\t\tuint32_t board_id = platform_get_board_id();'
        print '\t\tfor (unsigned i = 0; i < sizeof(cfg_map)/sizeof(cfg_map[0]); i++) {'
        print '\t\t\tif ((board_id & cfg_map[i].board_id_mask) == cfg_map[i].board_id) {'
        print '\t\t\t\tselected_map = &cfg_map[i];'
        print '\t\t\t\tbreak;'
        print '\t\t\t}'
        print '\t\t}'
        print ''
        print '\t\tif (selected_map == NULL)'
        print '\t\t\tpanic("no default pinconfig for board id %u", board_id);'
        print ''
        print '\t\tfor (unsigned i = 0; i < GPIOC_COUNT; i++) {'
        print '\t\t\tuint32_t num_pins = controller_pins[i];'
        print ''
        print '\t\t\tfor (uint32_t j = 0; j < num_pins; j++) {'
        print '\t\t\t\tuint8_t enum_key = selected_map->pinconfigs[i][j];'
        print '\t\t\t\texpanded_pinconfigs[i][j] = enum_map[enum_key];'
        print '\t\t\t}'
        print '\t\t}'
        print '\t}'
        print ''
        print '\treturn expanded_pinconfigs[gpioc];'
        print '}'


def main():
    soc_classes = collections.OrderedDict()
    soc_classes['h4a'] =      H4APinConfig
    soc_classes['h5p'] =      H5PPinConfig
    soc_classes['h5g'] =      H5GPinConfig
    soc_classes['alcatraz'] = AlcatrazPinConfig
    soc_classes['h6'] =       AlcatrazPinConfig
    soc_classes['h6p'] =      AlcatrazPinConfig
    soc_classes['fiji'] =     FijiPinConfig
    soc_classes['h7p'] =      FijiPinConfig
    soc_classes['capri'] =    CapriPinConfig
    soc_classes['h7g'] =      CapriPinConfig
    soc_classes['m7'] =       M7PinConfig
    soc_classes['h8p'] =      MauiPinConfig
    soc_classes['maui'] =     MauiPinConfig
    soc_classes['h8g'] =      ElbaPinConfig
    soc_classes['elba'] =     ElbaPinConfig
    soc_classes['h9p'] =      CaymanPinConfig
    soc_classes['cayman'] =   CaymanPinConfig
    soc_classes['m8'] =       M8PinConfig

    argparser = argparse.ArgumentParser(description='Converts CSV file to iBoot pinconfig.h format')
    argparser.add_argument('--soc', required=True, choices=soc_classes.keys())
    argparser.add_argument('--config-column', type=str, action='append')
    argparser.add_argument('--pupd-column', type=str, action='append')
    argparser.add_argument('--prefix', type=str, help='prefix for auto-generated variable and function names')
    argparser.add_argument('--copyright', type=str, default='YEAR', help='Year(s) to put in copyright notice')
    argparser.add_argument('--netname-column', type=str, action='append')
    argparser.add_argument('--radar', type=str, help='Link to spreadsheet tracker radar')
    argparser.add_argument('--force', action="store_true", help='attempt ouput even when there are parsing errors')
    argparser.add_argument('--rom', action="store_true", help='all pins except ROM function pins are disabled')
    argparser.add_argument('--header', action="store_true", help='generate a C header file instead of a C source file')
    argparser.add_argument('--sheet', default=None, help='sheet to read from when reading Excel files')
    argparser.add_argument('filename')

    args = argparser.parse_args()

    pinconfig_class = soc_classes[args.soc]

    per_target_headers = collections.defaultdict(dict)

    if not args.config_column is None:
        for item in map(lambda x: x.split(":"), args.config_column):
            if len(item) == 1:
                target = None
                column = item[0]
            elif len(item) == 2:
                target = item[0].upper()
                column = item[1]
            else:
                sys.stderr.write("Invalid config-column parameter\n")
                sys.exit(1)
            if 'function' in per_target_headers[target]:
                sys.stderr.write("Duplicate target in --config-column: \"{}\"".format(target))
                sys.exit(1)
            per_target_headers[target]['function'] = column

    if not args.pupd_column is None:
        for item in map(lambda x: x.split(":"), args.pupd_column):
            if len(item) == 1:
                target = None
                column = item[0]
            elif len(item) == 2:
                target = item[0].upper()
                column = item[1]
            else:
                sys.stderr.write("Invalid pupd-column parameter\n")
                sys.exit(1)
            if 'pupd' in per_target_headers[target]:
                sys.stderr.write("Duplicate target in --pupd-column: \"{}\"".format(target))
                sys.exit(1)
            per_target_headers[target]['pupd'] = column

    if not args.netname_column is None:
        for item in map(lambda x: x.split(":"), args.netname_column):
            if len(item) == 1:
                target = None
                column = item[0]
            elif len(item) == 2:
                target = item[0].upper()
                column = item[1]
            else:
                sys.stderr.write("Invalid netname-column parameter\n")
                sys.exit(1)
            if 'net_name' in per_target_headers[target]:
                sys.stderr.write("Duplicate target in --netname-column: \"{}\"".format(target))
                sys.exit(1)
            per_target_headers[target]['net_name'] = column

    if not per_target_headers:
        per_target_headers[None] = {}

    parser = CSVParser(pinconfig_class, per_target_headers)
    version, pinconfigs = parser.parse(args.filename, force=args.force, rom=args.rom, sheet=args.sheet)
    if pinconfigs is None:
        sys.exit(1)

    def pretty_arg(arg):
        if arg == args.filename:
            return '<filename>'
        else:
            return pipes.quote(arg)
    commandline = './tools/csvtopinconfig.py {}'.format(' '.join(pretty_arg(a) for a in sys.argv[1:]))

    print_pinconfigs(pinconfigs, pinconfig_class=pinconfig_class, version=version, radar=args.radar, copyright=args.copyright, commandline=commandline, prefix=args.prefix, rom=args.rom, header=args.header)

if __name__ == '__main__':
    main()
