#!/usr/bin/python
import argparse
import collections
import pipes
import sys

WRITE_COMMAND = 0
WRITE_COMPRESSED_COMMAND = 1
READ_COMMAND = 2
WRITE64_COMMAND = 3
DELAY_COMMAND = 4

def find_string(line, word):
    t = line.split()
    for x in t:
        if word in x:
            s = x.find('=')
            addr = x[s+1:]
    return addr

def convert_seqto_struct(filename, sequence):
    a = []
    with open (filename, 'rb') as config_seq_file:
        for line in config_seq_file:
            if sequence in line and 'comment' not in line:
                for line in config_seq_file:
                    if 'cfg_end' in line:
                        break
                    elif 'comment' in line or 'write_start' in line or 'write_end' in line:
                        continue
                    elif 'poll' in line:
                        # poll command
                        addr = find_string(line, 'reg_addr')
                        value = find_string(line, 'reg_value')
                        mask = find_string(line, 'reg_mask')
                        retryEnable = find_string(line, 'retry_en')
                        retryCnt = find_string(line, 'retry_cnt')   
                        a.append((addr, value, READ_COMMAND, mask, retryEnable, retryCnt))
                    elif 'write64' in line:
                        # 64 BIT write commands
                        addr = find_string(line, 'reg_addr')
                        value = find_string(line, 'reg_value')
                        a.append((addr, value, WRITE64_COMMAND))
                    elif 'cfg_cnt' in line:
                        value = line.split()[1]
                        a.append((0, value, DELAY_COMMAND))
                    else:
                        # write commands
                        addr = find_string(line, 'reg_addr')
                        value = find_string(line, 'reg_value')
                        a.append((addr, value, WRITE_COMMAND))


    j = 0
    print 'uint32_t %s[] = {' % (sequence.split()[1])
    for i in xrange(len(a)):
        if WRITE_COMMAND == a[i][2]:
            wrCommand = ((int(a[i][0], 16)>>10)<<6) | (1)
            offCommand = (int(a[i][0], 16)>>2) & 0xff 
            wrDword = int(a[i][1], 16)
            print '\t0x{:X}, 0x{:X}, 0x{:X},\t\t\t\t// Write'.format(wrCommand, offCommand, wrDword)
            j += 3
        elif READ_COMMAND == a[i][2]:
            rdCommand = ((int(a[i][0], 16)>>10)<<6) | (2)
            offCommand = (int(a[i][4], 16)<<16) | ((int(a[i][5], 16) & 0xFF)<<8) | ((int(a[i][0], 16)>>2) & 0xFF)
            rdDataMask = int(a[i][3], 16)
            rdDword = int(a[i][1], 16)
            print '\t0x{:X}, 0x{:X}, 0x{:X}, 0x{:X},\t\t// Read'.format(rdCommand, offCommand, rdDataMask, rdDword)
            j += 4
        elif WRITE64_COMMAND == a[i][2]:
            wrCommand = ((int(a[i][0], 16)>>10)<<6) | (1<<1) | (1)
            offCommand = (int(a[i][0], 16)>>2) & 0xff
            wrDwordLow = int(a[i][1], 16) & 0xFFFFFFFF
            wrDwordHigh = (int(a[i][1], 16) >> 32) & 0xFFFFFFFF
            if ((j+2) % 8) :
		    print '\t0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X},\t\t\t\t// Write'.format(wrCommand, offCommand, 0x0, wrDwordLow, wrDwordHigh)
		    j += 5
            else:
		    print '\t0x{:X}, 0x{:X}, 0x{:X}, 0x{:X},\t\t\t\t// Write'.format(wrCommand, offCommand, wrDwordLow, wrDwordHigh)
                    j += 4
        elif DELAY_COMMAND == a[i][2]:
            delayCommand = (int(a[i][1], 16)<<6) | 4
            print '\t0x{:X}, \t\t// Delay'.format(delayCommand)
            j += 1


    print '};'

def main():
    argparser = argparse.ArgumentParser(description='Converts config sequence to data structures to to be programmed')
    argparser.add_argument('filename')
    args = argparser.parse_args()

    convert_seqto_struct(args.filename, 'seq_name: awake_to_aop_ddr_pre')
    convert_seqto_struct(args.filename, 'seq_name: awake_to_aop_ddr_post')
    convert_seqto_struct(args.filename, 'seq_name: aop_ddr_to_s2r_aop_pre')
    convert_seqto_struct(args.filename, 'seq_name: s2r_aop_to_aop_ddr_post')
    convert_seqto_struct(args.filename, 'seq_name: aop_ddr_to_awake_pre')
    convert_seqto_struct(args.filename, 'seq_name: aop_ddr_to_awake_post')

    return 0    

if __name__ == '__main__':
    main()

