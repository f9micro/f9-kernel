#!/usr/bin/python
#
# Copyright (c) 2016 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Description:
#       Search in-kernel address by symbol name
#       Default using config board symbol maps.
#
# Usage:
#       symmap.py [symbol name]
#
# TODO:
#       1. search in different board
#


import os
import re
import sys
import glob

usage = "Usage: ./symmap.py [symbol name]\n"
filenotfound = "Cannot found %s f9_symmap.c, please build the project first.\n"


class ConfigParser(dict):
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


class Symmap:
    def __init__(self):
        self.symmap = {}
        self.config = ConfigParser('Makefile')
        self.parse()

    def parse(self):
        try:
            f = open('build/%s/f9_symmap.c' % (self.config.BOARD)).read()
        except FileNotFoundError:
            sys.exit(filenotfound % (self.config.BOARD))

        # Regex out address
        regex = r'{ \(void\*\) \((.*?)\), \d* }'
        address = re.findall(regex, f)

        # Regex out name
        regex = r'\"(.*?)\\0\"'
        names = re.findall(regex, f)

        # Zip it into dict
        for addr, name in zip(address, names):
            self.symmap[name] = int(addr, 16)

    def print_addr_by_name(self, name):
        if name in self.symmap:
            print('Symbol : %s\nAddress: 0x%x' % (name, self.symmap[name]))
        else:
            print('Cannot found "%s" in symmap.' % (name))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(usage)

    symbol_name = sys.argv[1]

    # Change to top dir
    abspath = os.path.abspath(sys.path[0])
    dname = os.path.dirname(abspath)
    os.chdir(dname)

    # Get the symmap
    s = Symmap()

    # Process argv
    print("Board: %s" % (s.config.BOARD))
    s.print_addr_by_name(symbol_name)
