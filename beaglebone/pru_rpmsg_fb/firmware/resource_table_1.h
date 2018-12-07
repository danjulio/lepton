
#ifndef _RSC_TABLE_PRU_H_
#define _RSC_TABLE_PRU_H_

#include <stddef.h> //needed for offset_of
#include <rsc_types.h>
#include "pru_virtio_ids.h"

/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 *
 * VQ0 : PRU->ARM
 * VQ1 : ARM->PRU
 */
#define PRU_RPMSG_VQ0_SIZE	32
#define PRU_RPMSG_VQ1_SIZE	32

/*
 * The feature bitmap for virtio rpmsg
 */
#define VIRTIO_RPMSG_F_NS	0		//name service notifications

/* This firmware supports name service notifications as one of its features */
#define RPMSG_PRU_C0_FEATURES	(1 << VIRTIO_RPMSG_F_NS)

/* Definition for unused interrupts */
#define HOST_UNUSED		255

#ifdef PRU0
#define TO_ARM_HOST			16
#define FROM_ARM_HOST			17
#define PRU0_TO_ARM_CHANNEL  2
#define PRU0_FROM_ARM_CHANNEL 0
#define PRU1_TO_ARM_CHANNEL  HOST_UNUSED
#define PRU1_FROM_ARM_CHANNEL HOST_UNUSED
#define HOST_INT			0x40000000
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 30"
#define CHAN_PORT			30
#define TO_ARM_CHANNEL PRU0_TO_ARM_CHANNEL
#define FROM_ARM_CHANNEL PRU0_FROM_ARM_CHANNEL
#endif
#ifdef PRU1
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19
#define PRU1_TO_ARM_CHANNEL  3
#define PRU1_FROM_ARM_CHANNEL 1
#define PRU0_TO_ARM_CHANNEL  HOST_UNUSED
#define PRU0_FROM_ARM_CHANNEL HOST_UNUSED
#define HOST_INT			0x80000000
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31
#define TO_ARM_CHANNEL PRU1_TO_ARM_CHANNEL
#define FROM_ARM_CHANNEL PRU1_FROM_ARM_CHANNEL
#endif
/* Mapping sysevts to a channel. Each pair contains a sysevt, channel. */
struct ch_map pru_intc_map[] = { {TO_ARM_HOST, TO_ARM_CHANNEL},
				 {FROM_ARM_HOST, FROM_ARM_CHANNEL},
};

struct my_resource_table {
	struct resource_table base;

	uint32_t offset[2]; /* Should match 'num' in actual definition */

	/* rpmsg vdev entry */
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;

	/* intc definition */
	struct fw_rsc_custom pru_ints;
};

#pragma DATA_SECTION(resourceTable, ".resource_table")
#pragma RETAIN(resourceTable)
struct my_resource_table resourceTable = {
	1,	/* Resource table version: only version 1 is supported by the current driver */
	2,	/* number of entries in the table */
	0, 0,	/* reserved, must be zero */
	/* offsets to entries */
	{
		offsetof(struct my_resource_table, rpmsg_vdev),
		offsetof(struct my_resource_table, pru_ints),
	},

	/* rpmsg vdev entry */
	{
		(unsigned)TYPE_VDEV,                    //type
		(unsigned)VIRTIO_ID_RPMSG,              //id
		(unsigned)0,                            //notifyid
		(unsigned)RPMSG_PRU_C0_FEATURES,	//dfeatures
		(unsigned)0,                            //gfeatures
		(unsigned)0,                            //config_len
		(unsigned char)0,                             //status
		(unsigned char)2,                             //num_of_vrings, only two is supported
		{ (unsigned char)0, (unsigned char)0 },             //reserved
		/* no config data */
	},
	/* the two vrings */
	{
		0,                      //da, will be populated by host, can't pass it in
		16,                     //align (bytes),
		PRU_RPMSG_VQ0_SIZE,     //num of descriptors
		0,                      //notifyid, will be populated, can't pass right now
		0                       //reserved
	},
	{
		0,                      //da, will be populated by host, can't pass it in
		16,                     //align (bytes),
		PRU_RPMSG_VQ1_SIZE,     //num of descriptors
		0,                      //notifyid, will be populated, can't pass right now
		0                       //reserved
	},

	{
		TYPE_CUSTOM, TYPE_PRU_INTS,
		sizeof(struct fw_rsc_custom_ints),
		{ /* PRU_INTS version */
			0x0000,
			/* Channel-to-host mapping, 255 for unused */
			PRU0_FROM_ARM_CHANNEL, PRU1_FROM_ARM_CHANNEL, PRU0_TO_ARM_CHANNEL, PRU1_TO_ARM_CHANNEL, HOST_UNUSED,
			HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED,
			/* Number of evts being mapped to channels */
			(sizeof(pru_intc_map) / sizeof(struct ch_map)),
			/* Pointer to the structure containing mapped events */
			pru_intc_map,
		},
	},
};

#endif /* _RSC_TABLE_PRU_H_ */

/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the
 *	  distribution.
 *
 *	* Neither the name of Texas Instruments Incorporated nor the names of
 *	  its contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
