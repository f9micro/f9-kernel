# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CHIP := stm32f4
PLATFORM := stm32
STM32_VARIANT := f4

board-y = board.o
loader-board-y = board.loader.o
