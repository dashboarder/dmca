#!/usr/bin/python

import lldb
import struct

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand("command script add -f lldb_os_iboot.iboot_tasks iboot_tasks")

def iboot_tasks(debugger, command, result, internal_dict):
    """Print the list of tasks maintained by the iBoot runtime"""
    process = debugger.GetSelectedTarget().GetProcess()
    if not process or not process.target:
        print 'Not connected'
        return

    plugin = OperatingSystemPlugIn(process)

    current_task = plugin.get_global('current_task')
    assert current_task is not None
    current_task_addr = current_task.GetValueAsUnsigned()

    tasks = plugin.get_task_dict().itervalues()
    print '  TID     Task  Name              Address'
    print '  ------  ----  ----------------  ------------------'
    for t in tasks:
        task_addr = t.AddressOf().GetValueAsUnsigned()
        current = "*" if task_addr == current_task_addr else " "

        tv = lldb.value(t)
        task_id = int(tv.task_id)
        tid = task_id + plugin.TID_OFFSET
        name = plugin.value_to_str(tv.name.sbvalue)

        print '{} {:#06x}  {:<4}  {:<16}  {:#018x}'.format(current, tid, task_id, name, task_addr)

class OperatingSystemPlugIn(object):
    """Class that provides data for an instance of a LLDB 'OperatingSystemPython' plug-in class"""

    TID_OFFSET = 0x1000
    
    def __init__(self, process):
        '''Initialization needs a valid.SBProcess object'''
        self.process = None
        self.registers = None
        self.task_dict = None
        self.arch = None
        if type(process) is lldb.SBProcess and process.IsValid():
            self.process = process
            triple = process.target.triple
            if triple:
                self.arch = triple.split('-')[0]

    def get_global(self, name):
        return self.process.target.FindGlobalVariables(name, 1)[0]

    def iboot_task_iter(self, head, skip_task=None):
        task_type = self.process.target.FindFirstType('task')
        task_members = { member.name : member for member in task_type.get_members_array() }
        list_offset = task_members['task_list_node'].byte_offset

        if not skip_task is None:
            skip_addr = skip_task.GetValueAsUnsigned()
        else:
            skip_addr = 0
        
        for node in head.AddressOf().linked_list_iter('next'):
            task_addr = node.GetValueAsUnsigned() - list_offset
            if task_addr == skip_addr:
                continue
            task = node.CreateValueFromAddress('task', task_addr, task_type)
            tid = int(lldb.value(task).task_id)
            yield tid, task

    def summarize_iboot_task(self, task):
        task_value = lldb.value(task)
        return {
            'name': self.value_to_str(task_value.name.sbvalue).ljust(15),
            'tid': int(task_value.task_id) + self.TID_OFFSET,
            'state': 'stopped',
            'stop_reason': 'none'
        }

    def get_task_dict(self, skip_current=False):
        if self.task_dict is None:
            task_list = self.get_global('task_list')
            assert task_list is not None

            if skip_current:
                skip_task = self.get_global('current_task')
            else:
                skip_task = None

            task_tuples = list(self.iboot_task_iter(task_list, skip_task))
            # The first entry in the list isn't a task, so remove it from the list
            task_tuples = task_tuples[1:]

            # If there aren't any tasks in the list, we must still be
            # in the bootstrap task, which we can get through current_task
            if len(task_tuples) == 0 and not skip_current:
                current_task = self.get_global('current_task').Dereference()
                current_tid = int(lldb.value(current_task).task_id)
                task_tuples = [(current_tid, current_task)]

            self.task_dict = {tid : thread for tid,thread in task_tuples}
        return self.task_dict

    @staticmethod
    def value_to_str(value):
        return "".join([chr(x) for x in value.GetData().uint8s if x != 0])

    @staticmethod
    def value_to_bytes(value):
        return "".join([chr(x) for x in value.GetData().uint8s])

    def get_thread_info(self):
        task_iter = self.get_task_dict(skip_current=True).itervalues()
        return map(lambda x: self.summarize_iboot_task(x), task_iter)
    
    def get_register_data(self, tid):
        assert tid >= self.TID_OFFSET
        tasks = self.get_task_dict()
        task = tasks[tid - self.TID_OFFSET]

        task_arch_value = lldb.value(task).arch

        if self.arch == 'arm64':
            regs = task_arch_value.regs.sbvalue
            fp = task_arch_value.fp.sbvalue
            lr = task_arch_value.lr.sbvalue
            sp = task_arch_value.sp.sbvalue
            # We don't know the PC, so use the LR instead to keep LLDB happy
            pc = task_arch_value.lr.sbvalue
            registers = [regs, fp, lr, sp, pc]
        else:
            regs = task_arch_value.regs.sbvalue
            # We don't know the PC, so use the LR instead to keep LLDB happy
            pc = task_arch_value.regs[9].sbvalue
            registers = [regs, pc]

        result = "".join(map(self.value_to_bytes, registers))
        return result
    
    def get_register_info(self):
        if self.registers == None:
            self.registers = dict()            
            if self.arch == 'arm64':
                self.registers['sets'] = ['GPR']
                self.registers['registers'] = [
                    { 'name':'x0'   , 'bitsize' :  64, 'offset' :  0x00, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  0, 'dwarf' :  0},
                    { 'name':'x1'   , 'bitsize' :  64, 'offset' :  0x08, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  1, 'dwarf' :  1},
                    { 'name':'x2'   , 'bitsize' :  64, 'offset' :  0x10, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  2, 'dwarf' :  2},
                    { 'name':'x3'   , 'bitsize' :  64, 'offset' :  0x18, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  3, 'dwarf' :  3},
                    { 'name':'x4'   , 'bitsize' :  64, 'offset' :  0x20, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  4, 'dwarf' :  4},
                    { 'name':'x5'   , 'bitsize' :  64, 'offset' :  0x28, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  5, 'dwarf' :  5},
                    { 'name':'x6'   , 'bitsize' :  64, 'offset' :  0x30, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  6, 'dwarf' :  6},
                    { 'name':'x7'   , 'bitsize' :  64, 'offset' :  0x38, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  7, 'dwarf' :  7},
                    { 'name':'x8'   , 'bitsize' :  64, 'offset' :  0x40, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  8, 'dwarf' :  8},
                    { 'name':'x9'   , 'bitsize' :  64, 'offset' :  0x48, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  9, 'dwarf' :  9},
                    { 'name':'x10'  , 'bitsize' :  64, 'offset' :  0x50, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 10, 'dwarf' : 10},
                    { 'name':'x11'  , 'bitsize' :  64, 'offset' :  0x58, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 11, 'dwarf' : 11},
                    { 'name':'x12'  , 'bitsize' :  64, 'offset' :  0x60, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 12, 'dwarf' : 12},
                    { 'name':'x13'  , 'bitsize' :  64, 'offset' :  0x68, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 13, 'dwarf' : 13},
                    { 'name':'x14'  , 'bitsize' :  64, 'offset' :  0x70, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 14, 'dwarf' : 14},
                    { 'name':'x15'  , 'bitsize' :  64, 'offset' :  0x78, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 15, 'dwarf' : 15},
                    { 'name':'x16'  , 'bitsize' :  64, 'offset' :  0x80, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 16, 'dwarf' : 16},
                    { 'name':'x17'  , 'bitsize' :  64, 'offset' :  0x88, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 17, 'dwarf' : 17},
                    { 'name':'x18'  , 'bitsize' :  64, 'offset' :  0x90, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 18, 'dwarf' : 18},
                    { 'name':'x19'  , 'bitsize' :  64, 'offset' :  0x98, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 19, 'dwarf' : 19},
                    { 'name':'x20'  , 'bitsize' :  64, 'offset' :  0xa0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 20, 'dwarf' : 20},
                    { 'name':'x21'  , 'bitsize' :  64, 'offset' :  0xa8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 21, 'dwarf' : 21},
                    { 'name':'x22'  , 'bitsize' :  64, 'offset' :  0xb0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 22, 'dwarf' : 22},
                    { 'name':'x23'  , 'bitsize' :  64, 'offset' :  0xb8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 23, 'dwarf' : 23},
                    { 'name':'x24'  , 'bitsize' :  64, 'offset' :  0xc0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 24, 'dwarf' : 24},
                    { 'name':'x25'  , 'bitsize' :  64, 'offset' :  0xc8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 25, 'dwarf' : 25},
                    { 'name':'x26'  , 'bitsize' :  64, 'offset' :  0xd0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 26, 'dwarf' : 26},
                    { 'name':'x27'  , 'bitsize' :  64, 'offset' :  0xd8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 27, 'dwarf' : 27},
                    { 'name':'x28'  , 'bitsize' :  64, 'offset' :  0xe0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 28, 'dwarf' : 28},
                    { 'name':'fp'   , 'bitsize' :  64, 'offset' :  0xe8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 29, 'dwarf' : 29, 'generic':'fp'},
                    { 'name':'lr'   , 'bitsize' :  64, 'offset' :  0xf0, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 30, 'dwarf' : 30, 'generic':'lr'},
                    { 'name':'sp'   , 'bitsize' :  64, 'offset' :  0xf8, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 31, 'dwarf' : 31, 'generic':'sp'},
                    { 'name':'pc'   , 'bitsize' :  64, 'offset' : 0x100, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 32, 'dwarf' : 32, 'generic':'pc'},
                    ]
            else:
                self.registers['sets'] = ['GPR']
                self.registers['registers'] = [
                    { 'name':'r4'   , 'bitsize' :  32, 'offset' :  0x00, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  4, 'dwarf' :  4},
                    { 'name':'r5'   , 'bitsize' :  32, 'offset' :  0x04, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  5, 'dwarf' :  5},
                    { 'name':'r6'   , 'bitsize' :  32, 'offset' :  0x08, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  6, 'dwarf' :  6},
                    { 'name':'r7'   , 'bitsize' :  32, 'offset' :  0x0c, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  7, 'dwarf' :  7, 'generic':'fp'},
                    { 'name':'r8'   , 'bitsize' :  32, 'offset' :  0x10, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  8, 'dwarf' :  8},
                    { 'name':'r9'   , 'bitsize' :  32, 'offset' :  0x14, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' :  9, 'dwarf' :  9},
                    { 'name':'r10'  , 'bitsize' :  32, 'offset' :  0x18, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 10, 'dwarf' : 10},
                    { 'name':'r11'  , 'bitsize' :  32, 'offset' :  0x1c, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 11, 'dwarf' : 11},
                    { 'name':'r13'  , 'bitsize' :  32, 'offset' :  0x20, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 13, 'dwarf' : 13, 'generic':'sp'},
                    { 'name':'r14'  , 'bitsize' :  32, 'offset' :  0x24, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 14, 'dwarf' : 14, 'generic':'lr'},
                    { 'name':'pc'   , 'bitsize' :  32, 'offset' :  0x28, 'encoding':'uint'  , 'format':'hex', 'set': 0, 'gcc' : 15, 'dwarf' : 15, 'generic':'pc'},
                    ]
        return self.registers
            
