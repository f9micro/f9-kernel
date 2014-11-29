/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef L4_IPC_H_
#define L4_IPC_H_

#define IPC_TI_MAP_GRANT 0x8
#define IPC_TI_GRANT 	 0x2

#define IPC_MR_COUNT	16

typedef union {
	struct {
		/* Number of words */
		uint32_t n_untyped : 6;
		uint32_t n_typed : 6;

		uint32_t flags : 4;	/* Type of operation */
		uint16_t label;
	} s;
	uint32_t raw;
} ipc_msg_tag_t;

typedef union {
	struct {
		uint32_t	header : 4;
		uint32_t	dummy  : 28;
	} s;
	struct {
		uint32_t	header : 4;
		uint32_t	base  : 28;
	} map;
	uint32_t raw;
} ipc_typed_item;

typedef union {
	uint16_t raw;
	struct {
		uint32_t	m : 10;
		uint32_t	e : 5;
		uint32_t	a : 1;
	} period;
	struct {
		uint32_t	m : 10;
		uint32_t	c : 1;
		uint32_t	e : 4;
		uint32_t	a : 1;
	} point;
} ipc_time_t;

#endif	/* L4_IPC_H_ */
