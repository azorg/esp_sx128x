#!/usr/bin/env python3
# -*- coding: UTF8 -*-

hdr="""/*
 * File: "bin2hex_def.h"
 */

#ifndef BIN2HEX_DEF_H
#define BIN2HEX_DEF_H
//-----------------------------------------------------------------------------"""

sep="//-----------------------------------------------------------------------------"

end="""//-----------------------------------------------------------------------------
#endif // BIN2HEX_DEF_H

/*** end of "bin2hex_def.h" file ***/
"""

print(hdr)

for i in range(1 << 8):
    print("#define _" + "0b" + "{:08b}".format(i) + " 0x" + "{:02X}".format(i))

print(sep)

for i in range(1 << 9):
    print("#define _" + "0b" + "{:09b}".format(i) + " 0x" + "{:03X}".format(i))

print(sep)

for i in range(1 << 10):
    print("#define _" + "0b" + "{:010b}".format(i) + " 0x" + "{:03X}".format(i))

print(sep)

for i in range(1 << 16):
    print("#define _" + "0b" + "{:016b}".format(i) + " 0x" + "{:04X}".format(i))

print(end)



