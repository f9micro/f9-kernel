# Copyright (c) 2014 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CHIP := stm32f1
PLATFORM := stm32
STM32_VARIANT := f1

board-y = board.o
loader-board-y = board.loader.o
