/*
 * Samsung Exynos SoC series Sensor driver
 *
 *
 * Copyright (c) 2020 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef IS_VENDER_CAMINFO_H
#define IS_VENDER_CAMINFO_H

#include <linux/vmalloc.h>
#include "is-core.h"

#ifndef _LINUX_TYPES_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

static bool sec2lsi_conversion_done[8] = {false, false, false, false,
										false, false, false, false};

#define IS_CAMINFO_IOCTL_MAGIC 			0xFB
#define CAM_MAX_SUPPORTED_LIST			20

#define IS_CAMINFO_IOCTL_COMMAND		_IOWR(IS_CAMINFO_IOCTL_MAGIC, 0x01, caminfo_ioctl_cmd *)

#ifdef CONFIG_COMPAT_CAMERA
#define IS_CAMINFO_IOCTL_COMMAND_COMPAT		_IOWR(IS_CAMINFO_IOCTL_MAGIC, 0x01, caminfo_ioctl_cmd_compat *)
#endif

#define SEC2LSI_AWB_DATA_SIZE			8
#define SEC2LSI_LSC_DATA_SIZE			6632
#define SEC2LSI_MODULE_INFO_SIZE		11
#define SEC2LSI_CHECKSUM_SIZE			4

typedef struct
{
	uint32_t cmd;
	void *data;
} caminfo_ioctl_cmd;

#ifdef CONFIG_COMPAT_CAMERA
typedef struct
{
	uint32_t cmd;
	compat_uptr_t data;
} caminfo_ioctl_cmd_compat;
#endif

enum caminfo_cmd_id
{
	CAMINFO_CMD_ID_GET_FACTORY_SUPPORTED_ID = 0,
	CAMINFO_CMD_ID_GET_ROM_DATA_BY_POSITION = 1,

	/* Standard CAL */
	CAMINFO_CMD_ID_GET_MODULE_INFO = 20,
	CAMINFO_CMD_ID_GET_SEC2LSI_BUFF = 21,
	CAMINFO_CMD_ID_SET_SEC2LSI_BUFF = 22,

	/* Ap2Ap Standard CAL */
	CAMINFO_CMD_ID_GET_AP2AP_CAL_SIZE = 23,
	CAMINFO_CMD_ID_GET_AP2AP_BUFF = 24,
	CAMINFO_CMD_ID_SET_AP2AP_BUFF = 25,
};

typedef struct
{
	uint32_t cam_position;
	uint32_t buf_size;
	uint8_t *buf;
	uint32_t rom_size;
} caminfo_romdata;

#ifdef CONFIG_COMPAT_CAMERA
typedef struct
{
	uint32_t cam_position;
	uint32_t buf_size;
	compat_uptr_t buf;
	uint32_t rom_size;
} caminfo_romdata_compat;
#endif

typedef struct
{
	uint32_t camID;

	/* Standard CAL */
	unsigned char *secBuf;
	unsigned char *lsiBuf;
	unsigned char *mdInfo; //Module information data in the calmap Header area
	uint32_t awb_size;
	uint32_t lsc_size;

	/* Ap2Ap Standard CAL */
	unsigned char *inCalBuf;
	unsigned char *outCalBuf;
	uint32_t cal_size;
	uint32_t bank; // only sc501 is effective
} caminfo_romdata_sec2lsi;

#ifdef CONFIG_COMPAT_CAMERA
typedef struct
{
	uint32_t camID;

	/* Standard CAL */
	compat_uptr_t secBuf;
	compat_uptr_t lsiBuf;
	compat_uptr_t mdInfo; //Module information data in the calmap Header area
	uint32_t awb_size;
	uint32_t lsc_size;

	/* Ap2Ap Standard CAL */
	compat_uptr_t inCalBuf;
	compat_uptr_t outCalBuf;
	uint32_t cal_size;
	uint32_t bank; // only sc501 is effective
} caminfo_romdata_sec2lsi_compat;
#endif

typedef struct
{
	uint32_t size;
	uint32_t data[CAM_MAX_SUPPORTED_LIST];
} caminfo_supported_list;

typedef struct
{
	struct mutex	mlock;
} is_vender_caminfo;

#endif /* IS_VENDER_CAMINFO_H */
