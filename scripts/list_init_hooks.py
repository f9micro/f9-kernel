#!/usr/bin/python
#
# Copyright (c) 2016 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Description:
#       list all init hooks in f9-kernel.
#
#       At now this script will list by hook level,
#       but if the hook is on the same int level, this will
#       not dump by real sequence when kernel perform run_init_hook.
#

# ----------------- LIBRARIES --------------------

import os
import re
import sys
import glob
import subprocess
from collections import OrderedDict

# ------------------- CONST --------------------

# Define in kernel/init_hook.h
HOOK_LEVEL = OrderedDict([])
HOOK_LEVEL.update({'INIT_LEVEL_EARLIEST': 1})
HOOK_LEVEL.update({'INIT_LEVEL_PLATFORM_EARLY': 0x1000})
HOOK_LEVEL.update({'INIT_LEVEL_PLATFORM': 0x2000})
HOOK_LEVEL.update({'INIT_LEVEL_KERNEL_EARLY': 0x3000})
HOOK_LEVEL.update({'INIT_LEVEL_KERNEL': 0x4000})
HOOK_LEVEL.update({'INIT_LEVEL_LAST': 0xFFFFFFFF})

# Dynamiclly adding variable on the fly
for hook in HOOK_LEVEL:
    exec('%s = %d' % (hook, HOOK_LEVEL[hook]))


# ----------------- CLASS --------------------
class MakefileConfig(dict):
    __getattr__ = dict.__getitem__
    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__

    def __init__(self, infile=None):
        self.board = {}

        # Parsing .config file
        self.parse_defconfig()
        self.parse_config('.config', user_config=True)

        # Parsing infile for Makefile
        if infile is not None:
            self.parse_makefile(infile)

    def parse_makefile(self, infile):
        f = map(lambda x: x.strip('\r\n ').replace(' ', ''),
                open(infile, 'r').readlines())

        for i in f:
            if len(i.split('?=')) == 2:
                key, value = i.split('?=')

                # Hack for ?=, if not set then set.
                try:
                    self.__getattr__(key)
                except:
                    self.__setattr__(key, value)

    def parse_config(self, path, user_config=False):
        maps = map(lambda x: x.strip('\r\n '),
                   open(path, 'r').readlines())

        for i in maps:
            if i and i.startswith('#') is False:
                key, value = i.split('=')

                if user_config:
                    self.__setattr__(key, True)

                    if key.startswith('CONFIG_BOARD_'):
                        self.__setattr__('BOARD', self.board[key])
                else:
                    if key.startswith('CONFIG_BOARD_'):
                        self.board[key] = path.split('/')[1]

    def parse_defconfig(self):
        defconfig = glob.glob('board/*/defconfig')
        for path in defconfig:
            self.parse_config(path)


class hook:

    def __init__(self, s):
        self.config = MakefileConfig('Makefile')
        self.config.parse_makefile('mk/toolchain.mk')
        self.parse(s)

    def __str__(self):
        return "%-30s%-30s%-30s%x" % (
            self.files, self.hook_function, self.hook_level, self.hook_level_int)

    def __lt__(self, other):
        return self.hook_level_int < other.hook_level_int

    def parse_from_obj(self, f):
        path = 'build/%s/%s' % (self.config.BOARD, f.replace('.c', '.o'))
        s = subprocess.check_output(['%sobjdump' % (self.config.CROSS_COMPILE),
                                     '-s',
                                     '-j',
                                     '.init_hook',
                                     path]).decode('utf-8')

        # Get hook level
        s = s.split('\n')[4].split(' ')[2]

        # Convert to hex value
        ret = int(''.join([s[i:i + 2] for i in range(0, len(s), 2)][::-1]), 16)
        return ret

    def parse(self, s):
        s = s.replace(' ', '')
        self.files, hooks = s.split(':')

        # Regex for splitting function and level
        regex = r"\((.*?),(.*?)\)"
        self.hook_function, self.hook_level = re.search(regex, hooks).groups()
        try:
            self.hook_level_int = eval(self.hook_level)
        except:
            self.hook_level_int = self.parse_from_obj(self.files)


def parse_hooks(hooks):
    init_hooks = sorted([hook(i) for i in hooks])

    return init_hooks


def print_by_level(hooks):
    last_level = 0
    for level_tag, level_int in HOOK_LEVEL.items():
        print(level_tag)

        token = False
        for hook in hooks:
            if hook.hook_level_int > last_level and \
                    hook.hook_level_int <= level_int:
                token = True
                print(hook)

        if token is False:
            print('--- None ---')

        last_level = level_int
        print('')


if __name__ == '__main__':
    # Change to top dir
    abspath = os.path.abspath(sys.path[0])
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # This command search all INIT_HOOK in top dir
    grep_command = ['grep', '-e', 'INIT_HOOK', '-r', '--include', '*.c']

    # Get grep output
    grep_output = filter(lambda x: x, subprocess.check_output(
        grep_command).decode('utf-8').split('\n'))

    # Process
    ph = parse_hooks(grep_output)
    print_by_level(ph)
