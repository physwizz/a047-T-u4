/*
 * Samsung Exynos5 SoC series IS driver
 *
 * exynos5 is video functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>

#include "is-sec-define.h"
#include "is-vender-specific.h"
#include "is-sec-util.h"
#include "is-device-eeprom.h"
#include "is-vender-rom-config.h"
#include "is-vender-caminfo.h"
#include "is-device-sensor-peri.h"

#define IS_LATEST_ROM_VERSION_M		'M'

/* crc_check_list is initialized in is_vender_probe */
bool crc32_check_list[SENSOR_POSITION_MAX][CRC32_SCENARIO_MAX];
bool check_latest_cam_module[SENSOR_POSITION_MAX] = {false, };
bool check_final_cam_module[SENSOR_POSITION_MAX] = {false, };

//static bool is_caldata_read = false;
bool force_caldata_dump = false;
bool is_dumped_fw_loading_needed = false;
bool sec2lsi_reload = false;

static int cam_id = CAMERA_SINGLE_REAR;

#ifdef SUPPORT_SENSOR_DUALIZATION
static u32 is_check_dualized_sensor[SENSOR_POSITION_MAX] = {false, };
#endif

static struct is_rom_info sysfs_finfo[SENSOR_POSITION_MAX];
static struct is_rom_info sysfs_pinfo[SENSOR_POSITION_MAX];

#ifdef IS_REAR_MAX_CAL_SIZE
static char cal_buf_rear[IS_REAR_MAX_CAL_SIZE];
static char cal_buf_rom_data_rear[IS_REAR_MAX_CAL_SIZE];
#endif
#ifdef IS_FRONT_MAX_CAL_SIZE
static char cal_buf_front[IS_FRONT_MAX_CAL_SIZE];
static char cal_buf_rom_data_front[IS_FRONT_MAX_CAL_SIZE];
#endif
#ifdef IS_REAR2_MAX_CAL_SIZE
static char cal_buf_rear2[IS_REAR2_MAX_CAL_SIZE];
static char cal_buf_rom_data_rear2[IS_REAR2_MAX_CAL_SIZE];
#endif
#ifdef IS_FRONT2_MAX_CAL_SIZE
static char cal_buf_front2[IS_FRONT2_MAX_CAL_SIZE];
static char cal_buf_rom_data_front2[IS_FRONT2_MAX_CAL_SIZE];
#endif
#ifdef IS_REAR3_MAX_CAL_SIZE
static char cal_buf_rear3[IS_REAR3_MAX_CAL_SIZE];
static char cal_buf_rom_data_rear3[IS_REAR3_MAX_CAL_SIZE];
#endif
#ifdef IS_FRONT3_MAX_CAL_SIZE
static char cal_buf_front3[IS_FRONT3_MAX_CAL_SIZE];
static char cal_buf_rom_data_front3[IS_FRONT3_MAX_CAL_SIZE];
#endif
#ifdef IS_REAR4_MAX_CAL_SIZE
static char cal_buf_rear4[IS_REAR4_MAX_CAL_SIZE];
static char cal_buf_rom_data_rear4[IS_REAR4_MAX_CAL_SIZE];
#endif
#ifdef IS_FRONT4_MAX_CAL_SIZE
static char cal_buf_front4[IS_FRONT4_MAX_CAL_SIZE];
static char cal_buf_rom_data_front4[IS_FRONT4_MAX_CAL_SIZE];
#endif


static char *cal_buf[SENSOR_POSITION_MAX] = {
#ifdef IS_REAR_MAX_CAL_SIZE
	cal_buf_rear,
#else
	NULL,
#endif
#ifdef IS_FRONT_MAX_CAL_SIZE
	cal_buf_front,
#else
	NULL,
#endif
#ifdef IS_REAR2_MAX_CAL_SIZE
	cal_buf_rear2,
#else
	NULL,
#endif
#ifdef IS_FRONT2_MAX_CAL_SIZE
	cal_buf_front2,
#else
	NULL,
#endif
#ifdef IS_REAR3_MAX_CAL_SIZE
	cal_buf_rear3,
#else
	NULL,
#endif
#ifdef IS_FRONT3_MAX_CAL_SIZE
	cal_buf_front3,
#else
	NULL,
#endif
#ifdef IS_REAR4_MAX_CAL_SIZE
	cal_buf_rear4,
#else
	NULL,
#endif
#ifdef IS_FRONT4_MAX_CAL_SIZE
	cal_buf_front4,
#else
	NULL,
#endif
};

/* cal_buf_rom_data is used for storing original rom data, before standard cal conversion */
static char *cal_buf_rom_data[SENSOR_POSITION_MAX] = {

#ifdef IS_REAR_MAX_CAL_SIZE
	cal_buf_rom_data_rear,
#else
	NULL,
#endif
#ifdef IS_FRONT_MAX_CAL_SIZE
	cal_buf_rom_data_front,
#else
	NULL,
#endif
#ifdef IS_REAR2_MAX_CAL_SIZE
	cal_buf_rom_data_rear2,
#else
	NULL,
#endif
#ifdef IS_FRONT2_MAX_CAL_SIZE
	cal_buf_rom_data_front2,
#else
	NULL,
#endif
#ifdef IS_REAR3_MAX_CAL_SIZE
	cal_buf_rom_data_rear3,
#else
	NULL,
#endif
#ifdef IS_FRONT3_MAX_CAL_SIZE
	cal_buf_rom_data_front3,
#else
	NULL,
#endif
#ifdef IS_REAR4_MAX_CAL_SIZE
	cal_buf_rom_data_rear4,
#else
	NULL,
#endif
#ifdef IS_FRONT4_MAX_CAL_SIZE
	cal_buf_rom_data_front4,
#else
	NULL,
#endif
};

static char *eeprom_cal_dump_path[SENSOR_POSITION_MAX] = {
	"dump/eeprom_rear_cal.bin",
	"dump/eeprom_front_cal.bin",
	"dump/eeprom_rear2_cal.bin",
	"dump/eeprom_front2_cal.bin",
	"dump/eeprom_rear3_cal.bin",
	"dump/eeprom_front3_cal.bin",
	"dump/eeprom_rear4_cal.bin",
	"dump/eeprom_front4_cal.bin",
	NULL,
	NULL,
};

static char *otprom_cal_dump_path[SENSOR_POSITION_MAX] = {
	"dump/otprom_rear_cal.bin",
	"dump/otprom_front_cal.bin",
	"dump/otprom_rear2_cal.bin",
	"dump/otprom_front2_cal.bin",
	"dump/otprom_rear3_cal.bin",
	"dump/otprom_front3_cal.bin",
	"dump/otprom_rear4_cal.bin",
	"dump/otprom_front4_cal.bin",
	NULL,
	NULL,
};

char loaded_fw[IS_HEADER_VER_SIZE + 1] = {0, };

static void *is_sec_search_rom_extend_data(const struct rom_extend_cal_addr *extend_data, char *name);

bool is_sec_get_force_caldata_dump(void)
{
	return force_caldata_dump;
}

int is_sec_set_force_caldata_dump(bool fcd)
{
	force_caldata_dump = fcd;
	if (fcd)
		info("forced caldata dump enabled!!\n");
	return 0;
}

int is_sec_get_max_cal_size(struct is_core *core, int position)
{
	int size = 0;
	struct is_vender_specific *specific = core->vender.private_data;

	if (!specific->rom_data[position].rom_valid) {
		err("Invalid position[%d]. This position don't have rom!\n", position);
		return size;
	}

	if (specific->rom_cal_map_addr[position] == NULL) {
		err("rom_%d: There is no cal map!\n", position);
		return size;
	}

	size = specific->rom_cal_map_addr[position]->rom_max_cal_size;

	if (!size) {
		err("Cal size is 0 (postion %d). Check cal size!", position);
	}

	return size;
}

int is_sec_get_cal_buf(int position, char **buf)
{
	*buf = cal_buf[position];

	if (*buf == NULL) {
		err("cal buf is null. position %d", position);
		return -EINVAL;
	}

	return 0;
}

int is_sec_get_cal_buf_rom_data(int position, char **buf)
{
	*buf = cal_buf_rom_data[position];

	if (*buf == NULL) {
		err("cal buf rom data is null. position %d", position);
		return -EINVAL;
	}

	return 0;
}

int is_sec_get_front_cal_buf(char **buf)
{
	is_sec_get_cal_buf(SENSOR_POSITION_FRONT, buf);
	return 0;
}

int is_sec_get_sysfs_finfo_by_position(int position, struct is_rom_info **finfo)
{
	*finfo = &sysfs_finfo[position];

	if (*finfo == NULL) {
		err("finfo addr is null. postion %d", position);
		/*WARN(true, "finfo is null\n");*/
		return -EINVAL;
	}

	return 0;
}

int is_sec_get_sysfs_finfo(struct is_rom_info **finfo)
{
	is_sec_get_sysfs_finfo_by_position(SENSOR_POSITION_REAR, finfo);
	return 0;
}

int is_sec_get_sysfs_finfo_front(struct is_rom_info **finfo)
{
	is_sec_get_sysfs_finfo_by_position(SENSOR_POSITION_FRONT, finfo);
	return 0;
}

int is_sec_get_sysfs_pinfo_by_position(int position, struct is_rom_info **pinfo)
{
	*pinfo = &sysfs_pinfo[position];

	if (*pinfo == NULL) {
		err("finfo addr is null. postion %d", position);
		/*WARN(true, "finfo is null\n");*/
		return -EINVAL;
	}

	return 0;
}

int is_sec_get_sysfs_pinfo_rear(struct is_rom_info **pinfo)
{
	is_sec_get_sysfs_pinfo_by_position(SENSOR_POSITION_REAR, pinfo);
	return 0;
}

int is_sec_get_sysfs_pinfo_front(struct is_rom_info **pinfo)
{
	is_sec_get_sysfs_pinfo_by_position(SENSOR_POSITION_FRONT, pinfo);
	return 0;
}

int is_sec_get_loaded_fw(char **buf)
{
	*buf = &loaded_fw[0];
	return 0;
}

int is_sec_set_loaded_fw(char *buf)
{
	strncpy(loaded_fw, buf, IS_HEADER_VER_SIZE);
	return 0;
}

int is_sec_set_camid(int id)
{
	cam_id = id;
	return 0;
}

int is_sec_get_camid(void)
{
	return cam_id;
}

int is_sec_get_camid_from_hal(char *fw_name, char *setf_name)
{
	err("%s: waring, you're calling the disabled func!", __func__);
	return 0;
}

int is_sec_fw_revision(char *fw_ver)
{
	int revision = 0;
	revision = revision + ((int)fw_ver[FW_PUB_YEAR] - 58) * 10000;
	revision = revision + ((int)fw_ver[FW_PUB_MON] - 64) * 100;
	revision = revision + ((int)fw_ver[FW_PUB_NUM] - 48) * 10;
	revision = revision + (int)fw_ver[FW_PUB_NUM + 1] - 48;

	return revision;
}

bool is_sec_fw_module_compare(char *fw_ver1, char *fw_ver2)
{
	if (fw_ver1[FW_CORE_VER] != fw_ver2[FW_CORE_VER]
		|| fw_ver1[FW_PIXEL_SIZE] != fw_ver2[FW_PIXEL_SIZE]
		|| fw_ver1[FW_PIXEL_SIZE + 1] != fw_ver2[FW_PIXEL_SIZE + 1]
		|| fw_ver1[FW_ISP_COMPANY] != fw_ver2[FW_ISP_COMPANY]
		|| fw_ver1[FW_SENSOR_MAKER] != fw_ver2[FW_SENSOR_MAKER]) {
		return false;
	}

	return true;
}

u8 is_sec_compare_ver(int position)
{
	u32 from_ver = 0, def_ver = 0, def_ver2 = 0, def_ver3 = 0;
	u8 ret = 0;
	char ver[3] = {'V', '0', '0'};
	char ver2[3] = {'V', 'F', '0'};
	char ver3[3] = {'V', '8', '0'};
	struct is_rom_info *finfo = NULL;

	if (is_sec_get_sysfs_finfo_by_position(position, &finfo)) {
		err("failed get finfo. plz check position %d", position);
		return 0;
	}

	def_ver = ver[0] << 16 | ver[1] << 8 | ver[2];
	def_ver2 = ver2[0] << 16 | ver2[1] << 8 | ver2[2];
	def_ver3 = ver3[0] << 16 | ver3[1] << 8 | ver3[2];
	from_ver = finfo->cal_map_ver[0] << 16 | finfo->cal_map_ver[1] << 8 | finfo->cal_map_ver[2];
	if ((from_ver == def_ver) || (from_ver == def_ver2) || (from_ver == def_ver3)) {
		return finfo->cal_map_ver[3];
	} else {
		/* Check ASCII code for dumpstate */
		if ((finfo->cal_map_ver[0]>= '0') && (finfo->cal_map_ver[0]<= 'z')
			&& (finfo->cal_map_ver[1]>= '0') && (finfo->cal_map_ver[1]<= 'z')
			&& (finfo->cal_map_ver[2]>= '0') && (finfo->cal_map_ver[2]<= 'z')
			&& (finfo->cal_map_ver[3]>= '0') && (finfo->cal_map_ver[3]<= 'z')) {
			err("FROM core version is invalid. version is %c%c%c%c",
					finfo->cal_map_ver[0], finfo->cal_map_ver[1], finfo->cal_map_ver[2], finfo->cal_map_ver[3]);
		} else {
			err("FROM core version is invalid. version is out of bounds");
		}
		return 0;
	}

	return ret;
}

bool is_sec_check_rom_ver(struct is_core *core, int position)
{
	struct is_rom_info *finfo = NULL;
	struct is_vender_specific *specific = core->vender.private_data;
	char compare_version;
	u8 from_ver;
	u8 latest_from_ver;
	int rom_position = position;

	if (specific->skip_cal_loading) {
		err("skip_cal_loading implemented");
		return false;
	}

	if (is_sec_get_sysfs_finfo_by_position(position, &finfo)) {
		err("failed get finfo. plz check position %d", position);
		return false;
	}

	if (specific->rom_share[position].check_rom_share == true) {
		if (specific->rom_share[position].share_position < SENSOR_POSITION_MAX) {
			rom_position = specific->rom_share[position].share_position;
		} else {
			err("invalid share_position (%d) plz check share_position \n", specific->rom_share[position].share_position);
			return false;
		}
	}


	if (!specific->rom_cal_map_addr[rom_position]) {
		err("failed get rom_cal_map_addr. plz check cal map addr(%d) \n", rom_position);
		return false;
	}

	latest_from_ver = specific->rom_cal_map_addr[rom_position]->cal_map_es_version;
	compare_version = specific->rom_cal_map_addr[rom_position]->camera_module_es_version;

	from_ver = is_sec_compare_ver(position);

	if ((from_ver < latest_from_ver &&
		specific->rom_cal_map_addr[rom_position]->rom_header_cal_map_ver_start_addr > 0 ) ||
		(finfo->header_ver[10] < compare_version)) {
		err("invalid from version. from_ver %c, header_ver[10] %c", from_ver, finfo->header_ver[10]);
		return false;
	} else {
		return true;
	}
}

bool is_sec_check_eeprom_crc32(char *buf, int position)
{
	u32 *buf32 = NULL;
	u32 checksum, check_base, checksum_base, check_length;
	u32 address_boundary;
	int i;
	int rom_position = position;
	bool rom_common = false;
	bool crc32_check_temp, crc32_header_temp;

	struct is_core *core;
	struct is_vender_specific *specific;
	const struct is_vender_rom_addr *rom_addr;
	struct is_rom_info *finfo = NULL;
	struct rom_standard_cal_data *standard_cal_data = NULL;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	specific = core->vender.private_data;
	buf32 = (u32 *)buf;

	if (specific->rom_share[position].check_rom_share == true) {
		rom_position = specific->rom_share[position].share_position;
		rom_common = true;
	}

	rom_addr = specific->rom_cal_map_addr[rom_position];

	info("%s E\n", __func__);
	/***** Initial Value *****/
	for (i = CRC32_CHECK_HEADER; i < CRC32_SCENARIO_MAX; i++ ) {
		crc32_check_list[position][i] = true;
	}
	crc32_check_temp = true;
	crc32_header_temp = true;

	/***** SKIP CHECK CRC *****/
#ifdef SKIP_CHECK_CRC
	pr_warning("Camera[%d]: Skip check crc32\n", position);

	crc32_check_temp = true;
	crc32_check_list[position][CRC32_CHECK] = crc32_check_temp;

	return crc32_check_temp;
#endif

	/***** START CHECK CRC *****/
	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	address_boundary = is_sec_get_max_cal_size(core, rom_position);

	/* HEADER DATA CRC CHECK */
	check_base = 0;
	checksum = 0;
	checksum_base = 0;
	check_length = rom_addr->rom_header_checksum_len;

#ifdef ROM_CRC32_DEBUG
	printk(KERN_INFO "[CRC32_DEBUG] Header CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
		check_length, finfo->header_section_crc_addr);
#endif

	checksum_base = finfo->header_section_crc_addr / 4;
	checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
	if (checksum != buf32[checksum_base]) {
		err("Camera: CRC32 error at the header (0x%08X != 0x%08X)", checksum, buf32[checksum_base]);
		crc32_check_temp = false;
		crc32_header_temp = false;
		goto out;
	} else {
		crc32_header_temp = true;
	}

	/* OEM Cal Data CRC CHECK */
	check_length = 0;
	if (rom_common == true) {
		if (rom_addr->rom_sub_oem_checksum_len > 0)
			check_length = rom_addr->rom_sub_oem_checksum_len;
	} else {
		if (rom_addr->rom_oem_checksum_len > 0)
			check_length = rom_addr->rom_oem_checksum_len;
	}

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->oem_start_addr / 4;
		checksum_base = finfo->oem_section_crc_addr / 4;

		if (rom_addr->extend_cal_addr) {
			int32_t *addr;
			addr = (int32_t *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_OEM_CHECKSUM);
			if (addr != NULL) {
				if (finfo->oem_start_addr != *addr)
					check_base = *addr / 4;
			}
		}

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] OEM CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->oem_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->oem_start_addr, finfo->oem_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: OEM address has error: start(0x%08X), end(0x%08X)",
				position, finfo->oem_start_addr, finfo->oem_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the OEM (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check oem crc32\n", position);
	}

	/* AWB Cal Data CRC CHECK */
	check_length = 0;
	if (rom_common == true) {
		if (rom_addr->rom_sub_awb_checksum_len > 0)
			check_length = rom_addr->rom_sub_awb_checksum_len;
	} else {
		if (rom_addr->rom_awb_checksum_len > 0)
			check_length = rom_addr->rom_awb_checksum_len;
	}

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->awb_start_addr / 4;
		checksum_base = finfo->awb_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] AWB CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->awb_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->awb_start_addr, finfo->awb_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: AWB address has error: start(0x%08X), end(0x%08X)",
				position, finfo->awb_start_addr, finfo->awb_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the AWB (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check awb crc32\n", position);
	}

	/* Shading Cal Data CRC CHECK*/
	check_length = 0;
	if (rom_common == true) {
		if (rom_addr->rom_sub_shading_checksum_len > 0)
			check_length = rom_addr->rom_sub_shading_checksum_len;
	} else {
		if (rom_addr->rom_shading_checksum_len > 0)
			check_length = rom_addr->rom_shading_checksum_len;
	}

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->shading_start_addr / 4;
		checksum_base = finfo->shading_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Shading CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->shading_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->shading_start_addr, finfo->shading_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: Shading address has error: start(0x%08X), end(0x%08X)",
				position, finfo->shading_start_addr, finfo->shading_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the Shading (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check shading crc32\n", position);
	}

#ifdef USE_AE_CAL
	/* AE Cal Data CRC CHECK */

	if (rom_addr->extend_cal_addr) {
		struct rom_ae_cal_data *ae_cal_data = NULL;

		ae_cal_data = (struct rom_ae_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_AE_CAL);

		check_length = 0;
		if (ae_cal_data) {
			if (ae_cal_data->rom_ae_checksum_addr > 0)
				check_length = ae_cal_data->rom_ae_checksum_len;

			if (check_length > 0) {
				checksum = 0;
				check_base = finfo->ae_cal_start_addr / 4;
				checksum_base = finfo->ae_cal_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
				printk(KERN_INFO "[CRC32_DEBUG] AE Cal Data CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
					check_length, finfo->ae_cal_section_crc_addr);
				printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
					finfo->ae_cal_start_addr, finfo->ae_cal_end_addr);
#endif

				if (check_base > address_boundary || checksum_base > address_boundary) {
					err("Camera[%d]: AE Cal Data address has error: start(0x%08X), end(0x%08X)",
						position, finfo->ae_cal_start_addr, finfo->ae_cal_end_addr);
					crc32_check_temp = false;
					goto out;
				}

				checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
				if (checksum != buf32[checksum_base]) {
					err("Camera[%d]: CRC32 error at the AE cal data (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
					crc32_check_temp = false;
					goto out;
				}
			} else {
				pr_warning("Camera[%d]: Skip to check AE Cal Data crc32\n", position);
			}

		}
	}
#endif

#ifdef SAMSUNG_LIVE_OUTFOCUS
	/* DUAL Cal Data CRC CHECK */
	check_length = 0;
	if (rom_addr->rom_dual_checksum_len > 0)
		check_length = rom_addr->rom_dual_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->dual_data_start_addr / 4;
		checksum_base = finfo->dual_data_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Dual Cal Data CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->dual_data_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->dual_data_start_addr, finfo->dual_data_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: Dual Cal Data address has error: start(0x%08X), end(0x%08X)",
				position, finfo->dual_data_start_addr, finfo->dual_data_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the DualData (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check Dual Cal Data crc32\n", position);
	}
#endif

#ifdef ENABLE_REMOSAIC_CAPTURE
	/* SENSOR Cal Data CRC CHECK */
	check_length = 0;
	if (rom_addr->rom_sensor_cal_checksum_len > 0)
		check_length = rom_addr->rom_sensor_cal_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->sensor_cal_data_start_addr / 4;
		checksum_base = finfo->sensor_cal_data_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Sensor Cal Data. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->sensor_cal_data_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->sensor_cal_data_start_addr, finfo->sensor_cal_data_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: Sensor Cal Data address has error: start(0x%08X), end(0x%08X)",
				position, finfo->sensor_cal_data_start_addr, finfo->sensor_cal_data_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the Sensor Cal Data (0x%08X != 0x%08X)",
				position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check Sensor Cal Data crc32\n", position);
	}
#endif

#ifdef USE_AP_PDAF
	/* PDAF Cal Data CRC CHECK */
	check_length = 0;
	if (rom_addr->rom_pdaf_checksum_len > 0)
		check_length = rom_addr->rom_pdaf_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->ap_pdaf_start_addr / 4;
		checksum_base = finfo->ap_pdaf_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] PDAF Cal Data. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->ap_pdaf_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->ap_pdaf_start_addr, finfo->ap_pdaf_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: PDAF Cal Data address has error: start(0x%08X), end(0x%08X)",
				position, finfo->ap_pdaf_start_addr, finfo->ap_pdaf_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: CRC32 error at the PDAF Cal Data (0x%08X != 0x%08X)",
				position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check PDAF Cal Data crc32\n", position);
	}
#endif

	/* STANDARD Cal Data CRC CHECK */
	if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_standard_cal_module_checksum_len > 0) {
			check_length = standard_cal_data->rom_standard_cal_module_checksum_len;
		
			if (check_length > 0) {
				checksum = 0;
				check_base = finfo->standard_cal_start_addr / 4;
				checksum_base = finfo->standard_cal_section_crc_addr / 4;
		
#ifdef ROM_CRC32_DEBUG
				printk(KERN_INFO "[CRC32_DEBUG] Standard Cal Data. check_length = %d, crc addr = 0x%08X\n",
					check_length, finfo->standard_cal_section_crc_addr);
				printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
					finfo->standard_cal_start_addr, finfo->standard_cal_end_addr);
#endif
		
				if (check_base > address_boundary || checksum_base > address_boundary) {
					err("Camera[%d]: Standard Cal Data address has error: start(0x%08X), end(0x%08X)",
						position, finfo->standard_cal_start_addr, finfo->standard_cal_end_addr);
					crc32_check_temp = false;
					goto out;
				}
		
				checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
				if (checksum != buf32[checksum_base]) {
					err("Camera[%d]: CRC32 error at the Standard Cal Data (0x%08X != 0x%08X)",
						position, checksum, buf32[checksum_base]);
					crc32_check_temp = false;
					goto out;
				}
			} else {
				pr_warning("Camera[%d]: Skip to check Standard Cal Data crc32\n", position);
			}
		}
	}

out:
	crc32_check_list[position][CRC32_CHECK] = crc32_check_temp;
	crc32_check_list[position][CRC32_CHECK_HEADER] = crc32_header_temp;
	info("Camera[%d]: CRC32 Check Result - crc32_header_check=%d, crc32_check=%d\n",
		position, crc32_header_temp, crc32_check_temp);

	return crc32_check_temp && crc32_header_temp;
}

bool is_sec_check_otp_crc32(char *buf, int position) {
	u32 *buf32 = NULL;
	u32 checksum, check_base, checksum_base, check_length;
	u32 address_boundary, checksumFromOTP;
	int i;
	bool crc32_check_temp, crc32_header_temp;

	struct is_core *core;
	struct is_vender_specific *specific;
	struct is_rom_info *finfo = NULL;
	const struct is_vender_rom_addr *rom_addr = NULL;
	struct rom_standard_cal_data *standard_cal_data = NULL;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	specific = core->vender.private_data;
	buf32 = (u32 *)buf;

	rom_addr = specific->rom_cal_map_addr[position];

	info("%s E\n", __func__);
	/***** Initial Value *****/
	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	address_boundary = is_sec_get_max_cal_size(core, position);

	for (i = CRC32_CHECK_HEADER; i < CRC32_SCENARIO_MAX; i++) {
		crc32_check_list[position][i] = true;
	}
	crc32_check_temp = true;
	crc32_header_temp = true;

	/***** SKIP CHECK CRC *****/
#ifdef SKIP_CHECK_CRC
	pr_warning("Camera[%d]: Skip check crc32\n", position);

	return crc32_check_temp;
#endif

	/***** HEADER checksum ****************************************************/
	/* Since start_addr and crc_addr may not be multiple of 4, should be used instead of buf32 for getCRC */
	check_length = 0;
	if (rom_addr->rom_header_checksum_len > 0)
		check_length = rom_addr->rom_header_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = 0;
		checksum_base = finfo->header_section_crc_addr;
		
#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Header CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->header_section_crc_addr);
#endif

		checksumFromOTP = buf[checksum_base] + (buf[checksum_base + 1] << 8)
						+ (buf[checksum_base + 2] << 16) + (buf[checksum_base + 3] << 24);

		checksum = (u32)getCRC((u16 *)&buf[check_base], check_length, NULL, NULL);
		if (checksum != checksumFromOTP) {
			err("Camera[%d]: CRC32 error at the Header Data (0x%08X != 0x%08X)",
				position, checksum, checksumFromOTP);
			crc32_check_temp = crc32_header_temp = false;
			goto out;
		} else {
			crc32_header_temp = true;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check Header Data crc32\n", position);
	}

	/***** OEM checksum *******************************************************/
	/* Since start_addr and crc_addr may not be multiple of 4, should be used instead of buf32 for getCRC */
	check_length = 0;
	if (rom_addr->rom_oem_checksum_len > 0)
		check_length = rom_addr->rom_oem_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->oem_start_addr;
		checksum_base = finfo->oem_section_crc_addr;

		if (rom_addr->extend_cal_addr) {
			int32_t *addr;
			addr = (int32_t *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_OEM_CHECKSUM);
			if (addr != NULL) {
				if (finfo->oem_start_addr != *addr)
					check_base = *addr;
			}
		}

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] OEM CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->oem_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->oem_start_addr, finfo->oem_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: OEM address has error: start(0x%08X), end(0x%08X)",
				position, finfo->oem_start_addr, finfo->oem_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksumFromOTP = buf[checksum_base] + (buf[checksum_base + 1] << 8)
						+ (buf[checksum_base + 2] << 16) + (buf[checksum_base + 3] << 24);

		checksum = (u32)getCRC((u16 *)&buf[check_base], check_length, NULL, NULL);
		if (checksum != checksumFromOTP) {
			err("Camera[%d]: CRC32 error at the OEM (0x%08X != 0x%08X)", position, checksum, checksumFromOTP);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check oem crc32\n", position);
	}

	/***** AWB checksum *******************************************************/
	/* Since start_addr and crc_addr may not be multiple of 4, should be used instead of buf32 for getCRC */
	check_length = 0;
	if (rom_addr->rom_awb_checksum_len > 0)
		check_length = rom_addr->rom_awb_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->awb_start_addr;
		checksum_base = finfo->awb_section_crc_addr;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] AWB CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->awb_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->awb_start_addr, finfo->awb_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: AWB address has error: start(0x%08X), end(0x%08X)",
				position, finfo->awb_start_addr, finfo->awb_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksumFromOTP = buf[checksum_base] + (buf[checksum_base + 1] << 8)
						+ (buf[checksum_base + 2] << 16) + (buf[checksum_base + 3] << 24);

		checksum = (u32)getCRC((u16 *)&buf[check_base], check_length, NULL, NULL);
		if (checksum != checksumFromOTP) {
			err("Camera[%d]: CRC32 error at the AWB (0x%08X != 0x%08X)", position, checksum, checksumFromOTP);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check awb crc32\n", position);
	}

	/***** LSC checksum *******************************************************/
	/* Since start_addr and crc_addr may not be multiple of 4, should be used instead of buf32 for getCRC */
	check_length = 0;
	if (rom_addr->rom_shading_checksum_len > 0)
		check_length = rom_addr->rom_shading_checksum_len;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->shading_start_addr;
		checksum_base = finfo->shading_section_crc_addr;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Shading CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->shading_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->shading_start_addr, finfo->shading_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: Shading address has error: start(0x%08X), end(0x%08X)",
				position, finfo->shading_start_addr, finfo->shading_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksumFromOTP = buf[checksum_base] + (buf[checksum_base + 1] << 8)
						+ (buf[checksum_base + 2] << 16) + (buf[checksum_base + 3] << 24);

		checksum = (u32)getCRC((u16 *)&buf[check_base], check_length, NULL, NULL);
		if (checksum != checksumFromOTP) {
			err("Camera[%d]: CRC32 error at the Shading (0x%08X != 0x%08X)", position, checksum, checksumFromOTP);
			crc32_check_temp = false;
			goto out;
		}
	} else {
		pr_warning("Camera[%d]: Skip to check shading crc32\n", position);
	}

	/* Standard Cal : CRC CHECK*/
	/* Since start_addr and crc_addr may not be multiple of 4, should be used instead of buf32 for getCRC */
	check_length = 0;

	if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_standard_cal_module_checksum_len > 0) {
			check_length = standard_cal_data->rom_standard_cal_module_checksum_len;
			check_base = standard_cal_data->rom_standard_cal_start_addr;
			checksum_base = standard_cal_data->rom_standard_cal_module_crc_addr;

#ifdef ROM_CRC32_DEBUG
			printk(KERN_INFO "[CRC32_DEBUG] Standard Cal CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
				check_length, standard_cal_data->rom_standard_cal_module_crc_addr);
#endif

			if (check_base > address_boundary || checksum_base > address_boundary) {
				err("Camera[%d]: Standard cal address has error: start(0x%08X), end(0x%08X)",
					position, standard_cal_data->rom_standard_cal_start_addr, standard_cal_data->rom_standard_cal_end_addr);
				crc32_check_temp = false;
				goto out;
			}

			checksumFromOTP = buf[checksum_base] + (buf[checksum_base + 1] << 8)
							+ (buf[checksum_base + 2] << 16) + (buf[checksum_base + 3] << 24);

			checksum = (u32)getCRC((u16 *)&buf[check_base], check_length, NULL, NULL);
			if (checksum != checksumFromOTP) {
				err("Camera[%d]: CRC32 error at the Standard Cal (0x%08X != 0x%08X)", position, checksum, checksumFromOTP);
				crc32_check_temp = false;
				goto out;
			} else {
				info("%s Standard cal is pass", __func__);
			}
			/* CRC32_CHECK_STANDARD_CAL is set false during ROM read for standard cal, it
			 * is set true after successful SEC2LSI conversion.
			 */
			crc32_check_list[position][CRC32_CHECK_STANDARD_CAL] = false;
		} else {
			pr_warning("Camera[%d]: Skip to check Standard Cal crc32\n", position);
		}
	}

out:
	crc32_check_list[position][CRC32_CHECK] = crc32_check_temp;
	crc32_check_list[position][CRC32_CHECK_HEADER] = crc32_header_temp;
	info("Camera[%d]: crc32_header_check=%d, crc32_check=%d\n", position,
		crc32_check_list[position][CRC32_CHECK_HEADER], crc32_check_list[position][CRC32_CHECK]);

	info("%s X\n", __func__);
	return crc32_check_temp;
}

bool is_sec_check_cal_crc32(char *buf, int id)
{
	bool ret = true;
	int rom_type = ROM_TYPE_NONE;
	struct is_core *core;
	struct is_vender_specific *specific;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	specific = core->vender.private_data;

	if (specific->rom_share[id].check_rom_share == true)
	{
		info("Camera[%d]: skip to check crc(shared rom data)\n", id);
	} else {
		rom_type = specific->rom_data[id].rom_type;

		if (rom_type == ROM_TYPE_EEPROM)
			ret = is_sec_check_eeprom_crc32(buf, id);
		else if (rom_type == ROM_TYPE_OTPROM)
			ret = is_sec_check_otp_crc32(buf, id);
		else {
			info("Camera[%d]: not support rom type(%d)\n", id, rom_type);
			ret = false;
		}
	}

	return ret;
}

void is_sec_make_crc32_table(u32 *table, u32 id)
{
	u32 i, j, k;

	for (i = 0; i < 256; ++i) {
		k = i;
		for (j = 0; j < 8; ++j) {
			if (k & 1)
				k = (k >> 1) ^ id;
			else
				k >>= 1;
		}
		table[i] = k;
	}
}


/**
 * is_sec_ldo_enabled: check whether the ldo has already been enabled.
 *
 * @ return: true, false or error value
 */
int is_sec_ldo_enabled(struct device *dev, char *name) {
	struct regulator *regulator = NULL;
	int enabled = 0;

	regulator = regulator_get_optional(dev, name);
	if (IS_ERR_OR_NULL(regulator)) {
		err("%s : regulator_get(%s) fail \n", __func__, name);
		return -EINVAL;
	}

	enabled = regulator_is_enabled(regulator);

	regulator_put(regulator);

	if (enabled == 1)
		info("%s : %s is aleady enabled !!\n", __func__, name);
	else if (enabled == 0)
		info("%s : %s is not enabled !!\n", __func__, name);

	return enabled;
}

int is_sec_ldo_enable(struct device *dev, char *name, bool on)
{
	struct regulator *regulator = NULL;
	int ret = 0;

	regulator = regulator_get_optional(dev, name);
	if (IS_ERR_OR_NULL(regulator)) {
		err("%s : regulator_get(%s) fail", __func__, name);
		return -EINVAL;
	}

	if (on) {
		if (regulator_is_enabled(regulator)) {
			pr_warning("%s: regulator is already enabled\n", name);
			goto exit;
		}

		ret = regulator_enable(regulator);
		if (ret) {
			err("%s : regulator_enable(%s) fail", __func__, name);
			goto exit;
		}
	} else {
		if (!regulator_is_enabled(regulator)) {
			pr_warning("%s: regulator is already disabled\n", name);
			goto exit;
		}

		ret = regulator_disable(regulator);
		if (ret) {
			err("%s : regulator_disable(%s) fail", __func__, name);
			goto exit;
		}
	}

exit:
	regulator_put(regulator);

	return ret;
}

int is_sec_rom_power_on(struct is_core *core, int position)
{
	int ret = 0;
	struct exynos_platform_is_module *module_pdata;
	struct is_module_enum *module = NULL;
	int i = 0;

	info("%s: Sensor position = %d\n", __func__, position);

	for (i = 0; i < SENSOR_POSITION_MAX; i++) {
		is_search_sensor_module_with_position(&core->sensor[i], position, &module);
		if (module)
			break;
	}

	if (!module) {
		err("%s: Could not find sensor id.", __func__);
		ret = -EINVAL;
		goto p_err;
	}

	module_pdata = module->pdata;

	if (!module_pdata->gpio_cfg) {
		err("gpio_cfg is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	ret = module_pdata->gpio_cfg(module, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_ON);
	if (ret) {
		err("gpio_cfg is fail(%d)", ret);
		goto p_err;
	}

p_err:
	return ret;
}

int is_sec_rom_power_off(struct is_core *core, int position)
{
	int ret = 0;
	struct exynos_platform_is_module *module_pdata;
	struct is_module_enum *module = NULL;
	int i = 0;

	info("%s: Sensor position = %d\n", __func__, position);

	for (i = 0; i < SENSOR_POSITION_MAX; i++) {
		is_search_sensor_module_with_position(&core->sensor[i], position, &module);
		if (module)
			break;
	}

	if (!module) {
		err("%s: Could not find sensor id.", __func__);
		ret = -EINVAL;
		goto p_err;
	}

	module_pdata = module->pdata;

	if (!module_pdata->gpio_cfg) {
		err("gpio_cfg is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	ret = module_pdata->gpio_cfg(module, SENSOR_SCENARIO_READ_ROM, GPIO_SCENARIO_OFF);
	if (ret) {
		err("gpio_cfg is fail(%d)", ret);
		goto p_err;
	}

p_err:
	return ret;
}

#ifdef SUPPORT_SENSOR_DUALIZATION
int is_sec_check_is_sensor(struct is_core *core, int position, int nextSensorId, bool *bVerified) {
	int ret;
	struct is_device_sensor_peri *sensor_peri = NULL;
	struct v4l2_subdev *subdev_cis = NULL;
	struct is_vender_specific *specific = core->vender.private_data;
	int i;
	u32 i2c_channel;
	struct exynos_platform_is_module *module_pdata;
	struct is_module_enum *module = NULL;
	const u32 scenario = SENSOR_SCENARIO_NORMAL;

	for (i = 0; i < IS_SENSOR_COUNT; i++) {
		is_search_sensor_module_with_position(&core->sensor[i], position, &module);
		if (module)
			break;
	}

	if (!module) {
		err("%s: Could not find sensor id.", __func__);
		ret = -EINVAL;
		goto p_err;
	}
	module_pdata = module->pdata;
	if (!module_pdata->gpio_cfg) {
		err("gpio_cfg is NULL");
		ret = -EINVAL;
		goto p_err;
	}
	ret = module_pdata->gpio_cfg(module, scenario, GPIO_SCENARIO_ON);
	if (ret) {
		err("gpio_cfg is fail(%d)", ret);
	} else {
		sensor_peri = (struct is_device_sensor_peri *)module->private_data;
		if (sensor_peri->subdev_cis) {
			i2c_channel = module_pdata->sensor_i2c_ch;
			if (i2c_channel < SENSOR_CONTROL_I2C_MAX) {
				sensor_peri->cis.i2c_lock = &core->i2c_lock[i2c_channel];
			} else {
				warn("%s: wrong cis i2c_channel(%d)", __func__, i2c_channel);
				ret = -EINVAL;
				goto p_err;
			}
		} else {
			err("%s: subdev cis is NULL. dual_sensor check failed", __func__);
			ret = -EINVAL;
			goto p_err;
		}

		subdev_cis = sensor_peri->subdev_cis;
		ret = CALL_CISOPS(&sensor_peri->cis, cis_check_rev_on_init, subdev_cis);

		if (ret < 0) {
			*bVerified = false;
			specific->sensor_id[position] = nextSensorId;
			warn("%s CIS active test failed", __func__);
		} else {
			*bVerified = true;
			info("%s CIS test passed", __func__);
		}
		ret = module_pdata->gpio_cfg(module, scenario, GPIO_SCENARIO_OFF);
		if (ret) {
			err("%s gpio_cfg is fail(%d)", __func__, ret);
		}

		/* Requested by HQE to meet the power guidance, add 20ms delay */
#ifdef PUT_30MS_BETWEEN_EACH_CAL_LOADING
		msleep(30);
#else
		msleep(20);
#endif
	}
p_err:
	return ret;
}

static int is_sec_update_dualized_sensor(struct is_core *core, int position)
{
	int ret = 0;
	int sensorid_1st, sensorid_2nd;
	struct is_vender_specific *specific = core->vender.private_data;

	sensorid_1st = specific->sensor_id[position];
	sensorid_2nd = is_vender_get_dualized_sensorid(position);

	if (sensorid_2nd != SENSOR_NAME_NOTHING && !is_check_dualized_sensor[position]) {
#define COUNT_DUALIZATION_CHECK 6
		int count_check = COUNT_DUALIZATION_CHECK;
		bool bVerified = false;

		while (count_check-- > 0) {
			ret = is_sec_check_is_sensor(core, position, sensorid_2nd , &bVerified);
			if (ret) {
				err("%s: Failed to check corresponding sensor equipped", __func__);
				break;
			}

			if (bVerified) {
				is_check_dualized_sensor[position] = true;
				break;
			}

			sensorid_2nd = sensorid_1st;
			sensorid_1st = specific->sensor_id[position];

			/* Update specific for dualized for OTPROM */
			if (specific->rom_data[position].rom_type == ROM_TYPE_OTPROM) {
				specific->rom_client[position] = specific->dualized_rom_client[position];
				specific->rom_cal_map_addr[position] = specific->dualized_rom_cal_map_addr[position];
			}

			if (count_check < COUNT_DUALIZATION_CHECK - 2) {
				err("Failed to find out both sensors on this device !!! (count : %d)", count_check);
				if (count_check <= 0)
					err("None of camera was equipped (sensorid : %d, %d)", sensorid_1st, sensorid_2nd);
			}
		}
	}

	return ret;
}
#endif

int is_sec_check_caldata_reload(struct is_core *core)
{
	struct file *reload_key_fp = NULL;
	struct file *supend_resume_key_fp = NULL;
	mm_segment_t old_fs;
	struct is_vender_specific *specific = core->vender.private_data;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	reload_key_fp = filp_open("/data/vendor/camera/reload/r1e2l3o4a5d.key", O_RDONLY, 0);
	if (IS_ERR(reload_key_fp)) {
		reload_key_fp = NULL;
		info("Reload KEY does not exist\n");
	} else {
		info("Reload KEY exist, reload cal data.\n");
		force_caldata_dump = true;
		sec2lsi_reload = true;
		specific->suspend_resume_disable = true;
	}

	if (reload_key_fp)
		filp_close(reload_key_fp, current->files);

	supend_resume_key_fp = filp_open("/data/vendor/camera/i1s2p3s4r.key", O_RDONLY, 0);
	if (IS_ERR(supend_resume_key_fp)) {
		supend_resume_key_fp = NULL;
	} else {
		info("Supend_resume KEY exist, disable runtime supend/resume. \n");
		specific->suspend_resume_disable = true;
	}

	if (supend_resume_key_fp)
		filp_close(supend_resume_key_fp, current->files);

	set_fs(old_fs);

	return 0;
}

bool is_sec_readcal_dump(struct is_vender_specific *specific, char **buf, int size, int position)
{
	int ret = false;
	int rom_position = position;
	int rom_type = ROM_TYPE_NONE;
	int cal_size = 0;
	bool rom_valid = false;
	struct file *key_fp = NULL;
	struct file *dump_fp = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	key_fp = filp_open("/data/vendor/camera/1q2w3e4r.key", O_RDONLY, 0);
	if (IS_ERR(key_fp)) {
		info("KEY does not exist.\n");
		key_fp = NULL;
		goto key_err;
	}

	dump_fp = filp_open("/data/vendor/camera/dump", O_RDONLY, 0);
	if (IS_ERR(dump_fp)) {
		info("dump folder does not exist.\n");
		dump_fp = NULL;
		goto key_err;
	}

	if (specific->rom_share[position].check_rom_share == true)
		rom_position = specific->rom_share[position].share_position;

	rom_valid = specific->rom_data[rom_position].rom_valid;
	rom_type = specific->rom_data[rom_position].rom_type;
	cal_size = specific->rom_cal_map_addr[rom_position]->rom_max_cal_size;

	if (rom_valid == true) {
		char path[50] = IS_SETFILE_SDCARD_PATH;

		if (rom_type == ROM_TYPE_EEPROM) {
			info("dump folder exist, Dump EEPROM cal data.\n");

			strcat(path, eeprom_cal_dump_path[position]);
			if (write_data_to_file(path, buf[0], size, &pos) < 0) {
				info("Failed to rear dump cal data.\n");
				goto dump_err;
			}

			ret = true;
		} else if (rom_type == ROM_TYPE_OTPROM) {
			info("dump folder exist, Dump OTPROM cal data.\n");

			strcat(path, otprom_cal_dump_path[position]);
#ifdef SENSOR_OTP_5E9
			if (write_data_to_file(path, buf[1], 0xf, &pos) < 0) {
				info("Failed to dump cal data.\n");
				goto dump_err;
			}
			if (write_data_to_file_append(path, buf[0], cal_size, &pos) < 0) {
				info("Failed to dump cal data.\n");
				goto dump_err;
			}
#else
			if (write_data_to_file(path, buf[0], cal_size, &pos) < 0) {
				info("Failed to dump cal data.\n");
				goto dump_err;
			}
#endif
			ret = true;
		}
	}

dump_err:
	if (dump_fp)
		filp_close(dump_fp, current->files);
key_err:
	if (key_fp)
		filp_close(key_fp, current->files);

	set_fs(old_fs);

	return ret;
}

#ifdef USES_STANDARD_CAL_RELOAD
bool is_sec_sec2lsi_check_cal_reload(void) {
	info("is_sec_sec2lsi_check_cal_reload=%d\n", sec2lsi_reload);
	return sec2lsi_reload;
}

bool is_sec_reload_cal(struct is_core *core, int position) {
	struct is_module_enum *module = NULL;
	struct is_rom_info *default_finfo = NULL;
	struct is_vender_specific *specific = core->vender.private_data;
	int ret = 0;

	is_sec_get_sysfs_finfo_by_position(SENSOR_POSITION_REAR, &default_finfo);

	if (default_finfo->is_check_cal_reload == true && specific->running_camera[position] == true) {
		int i;
		for (i = 0; i < SENSOR_POSITION_MAX; i++) {
			is_search_sensor_module_with_position(&core->sensor[i], position, &module);
			if (module)
				break;
		}

		if (!module) {
			err("%s: Could not find sensor id.", __func__);
			goto p_err;
		}

		info("%s: position(%d) running_camera(%d) cal_reload(%d)",
				__func__, position, specific->running_camera[position], default_finfo->is_check_cal_reload);

		ret =  is_vender_cal_load(&core->vender, module);
		if (ret < 0) {
			err("(%s) Unable to sync cal, is_vender_cal_load failed\n", __func__);
		}
	}

p_err:
	return ret;
}
#endif

bool is_sec_readcal_dump_post_sec2lsi(struct is_core *core, char *buf, int position)
{
	int ret = false;
	int rom_position = position;
	int rom_type = ROM_TYPE_NONE;
	int cal_size = 0;
	bool rom_valid = false;
	struct file *key_fp = NULL;
	struct file *dump_fp = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	struct is_vender_specific *specific = core->vender.private_data;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	key_fp = filp_open("/data/vendor/camera/1q2w3e4r.key", O_RDONLY, 0);
	if (IS_ERR(key_fp)) {
		info("KEY does not exist.\n");
		key_fp = NULL;
		goto key_err;
	}

	dump_fp = filp_open("/data/vendor/camera/dump", O_RDONLY, 0);
	if (IS_ERR(dump_fp)) {
		info("dump folder does not exist.\n");
		dump_fp = NULL;
		goto key_err;
	}

	if (specific->rom_share[position].check_rom_share == true)
		rom_position = specific->rom_share[position].share_position;

	rom_valid = specific->rom_data[rom_position].rom_valid;
	rom_type = specific->rom_data[rom_position].rom_type;
	cal_size = specific->rom_cal_map_addr[rom_position]->rom_max_cal_size;

	if (rom_valid == true) {
		char path[100] = IS_SETFILE_SDCARD_PATH;

		if (rom_type == ROM_TYPE_EEPROM) {
			info("dump folder exist, Dump EEPROM cal data.\n");

			strcat(path, eeprom_cal_dump_path[position]);
#ifdef USE_AP2AP_CAL_CONVERSION
			strcat(path, ".post_ap2ap.bin");
#else
			strcat(path, ".post_sec2lsi.bin");
#endif
			if (write_data_to_file(path, buf, cal_size, &pos) < 0) {
				info("Failed to rear dump cal data.\n");
				goto dump_err;
			}

			ret = true;
		} else if (rom_type == ROM_TYPE_OTPROM) {
			info("dump folder exist, Dump OTPROM cal data.\n");

			strcat(path, otprom_cal_dump_path[position]);
#ifdef USE_AP2AP_CAL_CONVERSION
			strcat(path, ".post_ap2ap.bin");
#else
			strcat(path, ".post_sec2lsi.bin");
#endif
			if (write_data_to_file(path, buf, cal_size, &pos) < 0) {
				info("Failed to dump cal data.\n");
				goto dump_err;
			}
			ret = true;
		}
	}

dump_err:
	if (dump_fp)
		filp_close(dump_fp, current->files);
key_err:
	if (key_fp)
		filp_close(key_fp, current->files);

	set_fs(old_fs);

	return ret;
}

bool is_sec_check_awb_lsc_crc32_post_sec2lsi(char* buf, int position, int awb_length, int lsc_length)
{
	u32 *buf32 = NULL;
	u32 checksum, check_base, checksum_base, check_length;
	u32 address_boundary;
	int rom_position = position;
	bool rom_common = false;
	bool crc32_check_temp = true;

	struct is_core *core;
	struct is_vender_specific *specific;
	const struct is_vender_rom_addr *rom_addr;
	struct is_rom_info *finfo = NULL;
	struct is_rom_info *default_finfo = NULL;
	struct is_module_enum *module = NULL;
	int i = 0;
	int ret = 0;

	core = (struct is_core *)dev_get_drvdata(is_dev);
	specific = core->vender.private_data;
	buf32 = (u32 *)buf;

	if (specific->rom_share[position].check_rom_share == true) {
		rom_position = specific->rom_share[position].share_position;
		rom_common = true;
	}

	rom_addr = specific->rom_cal_map_addr[rom_position];

	info("%s E\n", __func__);

	/***** START CHECK CRC *****/
	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	address_boundary = is_sec_get_max_cal_size(core, rom_position);
	
	check_length = 0;

	/* AWB Cal Data CRC CHECK */
	check_length = awb_length;

	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->awb_start_addr / 4;
		checksum_base = finfo->awb_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] AWB CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->awb_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->awb_start_addr, finfo->awb_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: AWB address has error: start(0x%08X), end(0x%08X)",
				position, finfo->awb_start_addr, finfo->awb_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: AWB address has error: start(0x%08X), end(0x%08X) Crc address(0x%08X)",
				position, finfo->awb_start_addr, finfo->awb_end_addr, finfo->awb_section_crc_addr);
			err("Camera[%d]: CRC32 error at the AWB (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		} else{
			info("%s  AWB CRC is pass! ",__func__);
		}
	} else {
		pr_warning("Camera[%d]: Skip to check awb crc32\n", position);
	}

	check_length = 0;

	/* Shading Cal Data CRC CHECK*/
	check_length = lsc_length;
	if (check_length > 0) {
		checksum = 0;
		check_base = finfo->shading_start_addr / 4;
		checksum_base = finfo->shading_section_crc_addr / 4;

#ifdef ROM_CRC32_DEBUG
		printk(KERN_INFO "[CRC32_DEBUG] Shading CRC32 Check. check_length = %d, crc addr = 0x%08X\n",
			check_length, finfo->shading_section_crc_addr);
		printk(KERN_INFO "[CRC32_DEBUG] start = 0x%08X, end = 0x%08X\n",
			finfo->shading_start_addr, finfo->shading_end_addr);
#endif

		if (check_base > address_boundary || checksum_base > address_boundary) {
			err("Camera[%d]: Shading address has error: start(0x%08X), end(0x%08X)",
				position, finfo->shading_start_addr, finfo->shading_end_addr);
			crc32_check_temp = false;
			goto out;
		}

		checksum = (u32)getCRC((u16 *)&buf32[check_base], check_length, NULL, NULL);
		if (checksum != buf32[checksum_base]) {
			err("Camera[%d]: Shading address has error: start(0x%08X), end(0x%08X) Crc address(0x%08X)",
				position, finfo->shading_start_addr, finfo->shading_end_addr, finfo->shading_section_crc_addr);
			err("Camera[%d]: CRC32 error at the Shading (0x%08X != 0x%08X)", position, checksum, buf32[checksum_base]);
			crc32_check_temp = false;
			goto out;
		} else {
			info("%s  LSC CRC is pass! ", __func__);
		}
	} else {
		pr_warning("Camera[%d]: Skip to check shading crc32\n", position);
	}

out:
	crc32_check_list[position][CRC32_CHECK_STANDARD_CAL] = crc32_check_temp;
	crc32_check_list[position][CRC32_CHECK] = crc32_check_temp;
	/* Sync DDK Cal with cal_buf during cal reload */
	is_sec_get_sysfs_finfo_by_position(SENSOR_POSITION_REAR, &default_finfo);
	info("%s: Sensor running = %d\n", __func__, specific->running_camera[position]);
	if (crc32_check_temp && default_finfo->is_check_cal_reload == true && specific->running_camera[position] == true) {
		for (i = 0; i < SENSOR_POSITION_MAX; i++) {
			is_search_sensor_module_with_position(&core->sensor[i], position, &module);
			if (module)
				break;
		}

		if (!module) {
			err("%s: Could not find sensor id.", __func__);
			crc32_check_temp = false;
			goto out;
		}

		ret =  is_vender_cal_load(&core->vender, module);
		if (ret < 0) {
			err("(%s) Unable to sync cal, is_vender_cal_load failed\n", __func__);
		}
	}
	info("%s X\n", __func__);
	return crc32_check_temp;
}

int is_sec_read_eeprom_header(struct device *dev, int position)
{
	int ret = 0;
	int rom_position = position;
	int32_t rom_header_ver_addr;
	u8 header_version[IS_HEADER_VER_SIZE + 1] = {0, };

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;
	struct i2c_client *client = NULL;;

	is_sec_get_sysfs_finfo_by_position(position, &finfo);

	if (specific->rom_share[position].check_rom_share == true) {
		rom_position = specific->rom_share[position].share_position;
	}

	client = specific->rom_client[rom_position];
	if (!client) {
		err("(%s)eeprom i2c client is NULL\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	if (specific->rom_cal_map_addr[rom_position] == NULL) {
		err("(%s)rom_cal_map(%d) is NULL\n", __func__, rom_position);
		ret = -EINVAL;
		goto exit;
	}

	rom_header_ver_addr = specific->rom_cal_map_addr[rom_position]->rom_header_main_module_info_start_addr;
	/* if it is used common rom */
	if (position == SENSOR_POSITION_REAR2 || position == SENSOR_POSITION_FRONT2) {
		if (specific->rom_cal_map_addr[rom_position]->rom_header_sub_module_info_start_addr > 0) {
			rom_header_ver_addr = specific->rom_cal_map_addr[rom_position]->rom_header_sub_module_info_start_addr;
		}
	}

	if (rom_header_ver_addr < 0) {
		err("(%s)rom_header_ver_addr is NULL(%d)\n", __func__, rom_header_ver_addr);
		goto exit;
	}

	is_i2c_config(client, true);

	ret = is_i2c_read(client, header_version, (u32)rom_header_ver_addr, IS_HEADER_VER_SIZE);

	//is_i2c_config(client, false); /* not used 'default' */

	if (unlikely(ret)) {
		err("failed to is_i2c_read for header version (%d)\n", ret);
		ret = -EINVAL;
	}

	memcpy(finfo->header_ver, header_version, IS_HEADER_VER_SIZE);
	finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';

exit:
	return ret;
}

int is_sec_readcal_eeprom(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	int rom_position = position;
	int cal_size = 0;
	char *buf = NULL;
	char *cal_buf[2] = {NULL, NULL};
	bool rom_common = false;

#ifdef USE_AE_CAL
	struct rom_ae_cal_data *ae_cal_data = NULL;
#endif

	int32_t cal_map_ver_start_addr, header_version_start_addr;
	int32_t temp_start_addr = 0, temp_end_addr = 0;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;
	struct i2c_client *client = NULL;

	const struct is_vender_rom_addr *rom_addr = NULL;
	
	struct rom_standard_cal_data *standard_cal_data = NULL;

	if (specific->rom_share[position].check_rom_share == true) {
		rom_position = specific->rom_share[position].share_position;
		rom_common = true;
	}

	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	is_sec_get_cal_buf(position, &buf);

	cal_size = is_sec_get_max_cal_size(core, rom_position);

	if (!cal_size) {
		err("(%s) rom_[%d] cal_size is zero\n", __func__, rom_position);
		ret = -EINVAL;
		goto exit;
	}

	client = specific->rom_client[rom_position];
	if (!client) {
		err("(%s)eeprom i2c client is NULL\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	is_i2c_config(client, true);
	rom_addr = specific->rom_cal_map_addr[rom_position];

	cal_map_ver_start_addr = rom_addr->rom_header_cal_map_ver_start_addr;

	if (rom_common == true) {
		header_version_start_addr = rom_addr->rom_header_sub_module_info_start_addr;
	} else {
		header_version_start_addr = rom_addr->rom_header_main_module_info_start_addr;
	}

	info("Camera[%d]: cal_map_ver_addr = %#x, header_version_addr = %#x\n",
		position, cal_map_ver_start_addr, header_version_start_addr);

	if (cal_map_ver_start_addr < 0 || header_version_start_addr < 0)
		goto exit;

	ret = is_i2c_read(client, finfo->cal_map_ver, (u32)cal_map_ver_start_addr, IS_CAL_MAP_VER_SIZE);
	ret = is_i2c_read(client, finfo->header_ver, (u32)header_version_start_addr, IS_HEADER_VER_SIZE);

	//is_i2c_config(client, false); /* not used 'default' */

	if (unlikely(ret)) {
		err("failed to is_i2c_read (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
	info("Camera[%d]: EEPROM Cal map_version = %c%c%c%c\n", position,
		finfo->cal_map_ver[0], finfo->cal_map_ver[1], finfo->cal_map_ver[2], finfo->cal_map_ver[3]);

	if (!is_sec_check_rom_ver(core, position)) {
		info("Camera[%d]: Do not read eeprom cal data. EEPROM version is low.\n", position);
		return 0;
	}

crc_retry:
	info("Camera[%d]: I2C read cal data\n", position);
	is_i2c_config(client, true);

	ret = is_i2c_read(client, buf, 0x0, cal_size);
	if (ret) {
		err("failed to is_i2c_read (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	info("Camera[%d]: EEPROM Header Version = %s\n", position, finfo->header_ver);

/////////////////////////////////////////////////////////////////////////////////////
/* Header Data */
/////////////////////////////////////////////////////////////////////////////////////
	/* Header Data: OEM */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_oem_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_oem_start_addr;
			temp_end_addr = rom_addr->rom_header_sub_oem_end_addr;
		}
	} else {
		if (rom_addr->rom_header_main_oem_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_oem_start_addr;
			temp_end_addr = rom_addr->rom_header_main_oem_end_addr;
		}
	}

	if (temp_start_addr >= 0) {
		finfo->oem_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->oem_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("OEM start = 0x%08x, end = 0x%08x\n", finfo->oem_start_addr, finfo->oem_end_addr);
	}

	/* Header Data: AWB */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_awb_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_awb_start_addr;
			temp_end_addr = rom_addr->rom_header_sub_awb_end_addr;
		}
	} else {
		if (rom_addr->rom_header_main_awb_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_awb_start_addr;
			temp_end_addr = rom_addr->rom_header_main_awb_end_addr;
		}
	}

	if (temp_start_addr >= 0) {
		finfo->awb_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->awb_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("AWB start = 0x%08x, end = 0x%08x\n", finfo->awb_start_addr, finfo->awb_end_addr);
	}
	else if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_awb_start_addr >= 0) {
			finfo->awb_start_addr = standard_cal_data->rom_awb_start_addr;
			finfo->awb_end_addr = standard_cal_data->rom_awb_end_addr;
			info("AWB start = 0x%08x, end = 0x%08x\n", finfo->awb_start_addr, finfo->awb_end_addr);
		}
	}

	/* Header Data: Shading */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_shading_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_shading_start_addr;
			temp_end_addr = rom_addr->rom_header_sub_shading_end_addr;
		}
	} else {
		if (rom_addr->rom_header_main_shading_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_shading_start_addr;
			temp_end_addr = rom_addr->rom_header_main_shading_end_addr;
		}
	}

	if (temp_start_addr >= 0) {
		finfo->shading_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->shading_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("Shading start = 0x%08x, end = 0x%08x\n", finfo->shading_start_addr, finfo->shading_end_addr);
		if (finfo->shading_end_addr > 0x3AFF) {
			err("Shading end_addr has error!! 0x%08x", finfo->shading_end_addr);
			finfo->shading_end_addr = 0x3AFF;
		}
	}
	else if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_shading_start_addr >= 0) {
			finfo->shading_start_addr = standard_cal_data->rom_shading_start_addr;
			finfo->shading_end_addr = standard_cal_data->rom_shading_end_addr;
			info("Shading start = 0x%08x, end = 0x%08x\n", finfo->shading_start_addr, finfo->shading_end_addr);
		}
	}

#ifdef USE_AE_CAL
	/* Header Data: AE CAL DATA */
	if (rom_addr->extend_cal_addr) {
		ae_cal_data = (struct rom_ae_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_AE_CAL);
		if (ae_cal_data) {
			temp_start_addr = temp_end_addr = -1;
			if (ae_cal_data->rom_header_main_ae_start_addr >= 0) {
				temp_start_addr = ae_cal_data->rom_header_main_ae_start_addr;
				temp_end_addr = ae_cal_data->rom_header_main_ae_end_addr;
			}

			if (temp_start_addr >= 0) {
				finfo->ae_cal_start_addr = *((u32 *)&buf[temp_start_addr]);
				finfo->ae_cal_end_addr = *((u32 *)&buf[temp_end_addr]);
				info("AE Cal Data start = 0x%08x, end = 0x%08x\n", finfo->ae_cal_start_addr, finfo->ae_cal_end_addr);
			}
		}
	}
#endif

#ifdef ENABLE_REMOSAIC_CAPTURE
	/* Header Data: Sensor CAL (CrossTalk & LSC) */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_main_sensor_cal_start_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_main_sensor_cal_start_addr;
		temp_end_addr = rom_addr->rom_header_main_sensor_cal_end_addr;
	}

	if (temp_start_addr >= 0) {
		finfo->sensor_cal_data_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->sensor_cal_data_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("Sensor Cal Data start = 0x%08x, end = 0x%08x\n",
			finfo->sensor_cal_data_start_addr, finfo->sensor_cal_data_end_addr);
	}
#endif

#ifdef SAMSUNG_LIVE_OUTFOCUS
	/* Header Data: Dual CAL */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_dual_cal_start_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_dual_cal_start_addr;
		temp_end_addr = rom_addr->rom_header_dual_cal_end_addr;
	}

	if (temp_start_addr >= 0) {
		finfo->dual_data_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->dual_data_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("Dual Cal Data start = 0x%08x, end = 0x%08x\n", finfo->dual_data_start_addr, finfo->dual_data_end_addr);
	}
#endif

#ifdef USE_AP_PDAF
	/* Header Data: Dual CAL */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_pdaf_cal_start_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_pdaf_cal_start_addr;
		temp_end_addr = rom_addr->rom_header_pdaf_cal_end_addr;
	}

	if (temp_start_addr >= 0) {
		finfo->ap_pdaf_start_addr = *((u32 *)&buf[temp_start_addr]);
		finfo->ap_pdaf_end_addr = *((u32 *)&buf[temp_end_addr]);
		info("PDAF Cal Data start = 0x%08x, end = 0x%08x\n", finfo->ap_pdaf_start_addr, finfo->ap_pdaf_end_addr);
	}
#endif

	/* Header Data: Standard CAL */
	if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_standard_cal_start_addr >= 0) {
			finfo->standard_cal_start_addr = standard_cal_data->rom_standard_cal_start_addr;
			finfo->standard_cal_end_addr = standard_cal_data->rom_standard_cal_end_addr;
			finfo->standard_cal_section_crc_addr = standard_cal_data->rom_standard_cal_module_crc_addr;
			info("%s sec2lsi  finfo->standard_cal_start_addr is 0x%08X", __func__, finfo->standard_cal_start_addr);
		}
	}

	/* Header Data: Header Module Info */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_module_info_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_module_info_start_addr;
		}
	} else {
		if (rom_addr->rom_header_main_module_info_start_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_module_info_start_addr;
		}
	}

	if (temp_start_addr >= 0) {
		memcpy(finfo->header_ver, &buf[temp_start_addr], IS_HEADER_VER_SIZE);
		finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';
	}

	/* Header Data: CAL MAP Version */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_cal_map_ver_start_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_cal_map_ver_start_addr;

		memcpy(finfo->cal_map_ver, &buf[temp_start_addr], IS_CAL_MAP_VER_SIZE);
	}

	/* Header Data: PROJECT NAME */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_project_name_start_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_project_name_start_addr;

		memcpy(finfo->project_name, &buf[temp_start_addr], IS_PROJECT_NAME_SIZE);
		finfo->project_name[IS_PROJECT_NAME_SIZE] = '\0';
	}

	/* Header Data: MODULE ID */
	temp_start_addr = temp_end_addr = -1;
	if (rom_addr->rom_header_module_id_addr >= 0) {
		temp_start_addr = rom_addr->rom_header_module_id_addr;

		memcpy(finfo->rom_module_id, &buf[temp_start_addr], IS_MODULE_ID_SIZE);
	} else {
		memset(finfo->rom_module_id, 0x0, IS_MODULE_ID_SIZE);
	}

	/* Header Data: SENSOR ID */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_sensor_id_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_sensor_id_addr;
		}
	} else {
		if (rom_addr->rom_header_main_sensor_id_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_sensor_id_addr;
		}
	}

	if (temp_start_addr >= 0) {
		memcpy(finfo->rom_sensor_id, &buf[temp_start_addr], IS_SENSOR_ID_SIZE);
		finfo->rom_sensor_id[IS_SENSOR_ID_SIZE] = '\0';
	}

	/* Header Data: MTF Data (Resolution) */
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		if (rom_addr->rom_header_sub_mtf_data_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_sub_mtf_data_addr;
		}
	} else {
		if (rom_addr->rom_header_main_mtf_data_addr >= 0) {
			temp_start_addr = rom_addr->rom_header_main_mtf_data_addr;
		}
	}

	if (temp_start_addr >= 0)
		finfo->mtf_data_addr = temp_start_addr;

	/* Header Data: HEADER CAL CHECKSUM */
	if (rom_addr->rom_header_checksum_addr >= 0)
		finfo->header_section_crc_addr = rom_addr->rom_header_checksum_addr;

/////////////////////////////////////////////////////////////////////////////////////
/* OEM Data: OEM Module Info */
/////////////////////////////////////////////////////////////////////////////////////
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		temp_start_addr = rom_addr->rom_sub_oem_module_info_start_addr;
		temp_end_addr = rom_addr->rom_sub_oem_checksum_addr;
	} else {
		temp_start_addr = rom_addr->rom_oem_module_info_start_addr;
		temp_end_addr = rom_addr->rom_oem_checksum_addr;
	}

	if (temp_start_addr >= 0) {
		memcpy(finfo->oem_ver, &buf[temp_start_addr], IS_OEM_VER_SIZE);
		finfo->oem_ver[IS_OEM_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->oem_section_crc_addr = temp_end_addr;
	}

	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		temp_start_addr = rom_addr->rom_sub_oem_af_inf_position_addr;
		temp_end_addr = rom_addr->rom_sub_oem_af_macro_position_addr;
	} else {
		temp_start_addr = rom_addr->rom_oem_af_inf_position_addr;
		temp_end_addr = rom_addr->rom_oem_af_macro_position_addr;
	}

	if (temp_start_addr >= 0)
		finfo->af_cal_pan = *((u32 *)&buf[temp_start_addr]);
	if (temp_end_addr >= 0)
		finfo->af_cal_macro = *((u32 *)&buf[temp_end_addr]);

/////////////////////////////////////////////////////////////////////////////////////
/* AWB Data: AWB Module Info */
/////////////////////////////////////////////////////////////////////////////////////
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		temp_start_addr = rom_addr->rom_sub_awb_module_info_start_addr;
		temp_end_addr = rom_addr->rom_sub_awb_checksum_addr;
	} else {
		temp_start_addr = rom_addr->rom_awb_module_info_start_addr;
		temp_end_addr = rom_addr->rom_awb_checksum_addr;
	}

	if (temp_start_addr >= 0) {
		memcpy(finfo->awb_ver, &buf[temp_start_addr], IS_AWB_VER_SIZE);
		finfo->awb_ver[IS_AWB_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->awb_section_crc_addr = temp_end_addr;
	}

/////////////////////////////////////////////////////////////////////////////////////
/* SHADING Data: Shading Module Info */
/////////////////////////////////////////////////////////////////////////////////////
	temp_start_addr = temp_end_addr = -1;
	if (rom_common == true) {
		temp_start_addr = rom_addr->rom_sub_shading_module_info_start_addr;
		temp_end_addr = rom_addr->rom_sub_shading_checksum_addr;
	} else {
		temp_start_addr = rom_addr->rom_shading_module_info_start_addr;
		temp_end_addr = rom_addr->rom_shading_checksum_addr;
	}

	if (temp_start_addr >= 0) {
		memcpy(finfo->shading_ver, &buf[temp_start_addr], IS_SHADING_VER_SIZE);
		finfo->shading_ver[IS_SHADING_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->shading_section_crc_addr = temp_end_addr;
	}

/////////////////////////////////////////////////////////////////////////////////////
/* AE Data: AE Module Info */
/////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_AE_CAL
	temp_start_addr = temp_end_addr = -1;
	if (ae_cal_data) {
		temp_start_addr = ae_cal_data->rom_ae_module_info_start_addr;
		temp_end_addr = ae_cal_data->rom_ae_checksum_addr;

		if (temp_start_addr >= 0) {
			memcpy(finfo->ae_cal_ver, &buf[temp_start_addr], IS_AE_CAL_VER_SIZE);
			finfo->ae_cal_ver[IS_AE_CAL_VER_SIZE] = '\0';
		}
		if (temp_end_addr >= 0) {
			finfo->ae_cal_section_crc_addr = temp_end_addr;
		}
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/* Dual CAL Data: Dual cal Module Info */
/////////////////////////////////////////////////////////////////////////////////////
#ifdef SAMSUNG_LIVE_OUTFOCUS
	temp_start_addr = temp_end_addr = -1;
	temp_start_addr = rom_addr->rom_dual_module_info_start_addr;
	temp_end_addr = rom_addr->rom_dual_checksum_addr;

	if (temp_start_addr >= 0) {
		memcpy(finfo->dual_data_ver, &buf[temp_start_addr], IS_DUAL_CAL_VER_SIZE);
		finfo->dual_data_ver[IS_DUAL_CAL_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->dual_data_section_crc_addr = temp_end_addr;
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/* Sensor CAL Data: Sensor CAL Module Info */
/////////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_REMOSAIC_CAPTURE
	temp_start_addr = temp_end_addr = -1;
	temp_start_addr = rom_addr->rom_sensor_cal_module_info_start_addr;
	temp_end_addr = rom_addr->rom_sensor_cal_checksum_addr;

	if (temp_start_addr >= 0) {
		memcpy(finfo->sensor_cal_data_ver, &buf[temp_start_addr], IS_SENSOR_CAL_DATA_VER_SIZE);
		finfo->sensor_cal_data_ver[IS_SENSOR_CAL_DATA_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->sensor_cal_data_section_crc_addr = temp_end_addr;
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/* PDAF CAL Data: PDAF Module Info */
/////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_AP_PDAF
	temp_start_addr = temp_end_addr = -1;
	temp_start_addr = rom_addr->rom_pdaf_module_info_start_addr;
	temp_end_addr = rom_addr->rom_pdaf_checksum_addr;

	if (temp_start_addr >= 0) {
		memcpy(finfo->ap_pdaf_ver, &buf[temp_start_addr], IS_AP_PDAF_VER_SIZE);
		finfo->ap_pdaf_ver[IS_AP_PDAF_VER_SIZE] = '\0';
	}
	if (temp_end_addr >= 0) {
		finfo->ap_pdaf_section_crc_addr = temp_end_addr;
	}
#endif

	/* debug info dump */
#if defined(ROM_DEBUG)
{
	u32 i = 1;
	info("++++ EEPROM[%d] data info\n", position);
	info("1. Header info\n");
	info(" Module info : %s\n", finfo->header_ver);
	info(" ID : %c\n", finfo->header_ver[FW_CORE_VER]);
	info(" Pixel num : %c%c\n", finfo->header_ver[FW_PIXEL_SIZE], finfo->header_ver[FW_PIXEL_SIZE + 1]);
	info(" ISP ID : %c\n", finfo->header_ver[FW_ISP_COMPANY]);
	info(" Sensor Maker : %c\n", finfo->header_ver[FW_SENSOR_MAKER]);
	info(" Year : %c\n", finfo->header_ver[FW_PUB_YEAR]);
	info(" Month : %c\n", finfo->header_ver[FW_PUB_MON]);
	info(" Release num : %c%c\n", finfo->header_ver[FW_PUB_NUM], finfo->header_ver[FW_PUB_NUM + 1]);
	info(" Manufacturer ID : %c\n", finfo->header_ver[FW_MODULE_COMPANY]);
	info(" Module ver : %c\n", finfo->header_ver[FW_VERSION_INFO]);
	info(" project_name : %s\n", finfo->project_name);
	info(" Cal data map ver : %s\n", finfo->cal_map_ver);
	info(" Module ID : %c%c%c%c%c%02X%02X%02X%02X%02X\n",
		finfo->rom_module_id[0], finfo->rom_module_id[1], finfo->rom_module_id[2],
		finfo->rom_module_id[3], finfo->rom_module_id[4], finfo->rom_module_id[5],
		finfo->rom_module_id[6], finfo->rom_module_id[7], finfo->rom_module_id[8],
		finfo->rom_module_id[9]);

	if (rom_addr->rom_oem_module_info_start_addr >= 0) {
		info("%d. OEM info\n", ++i);
		info(" Module info : %s\n", finfo->oem_ver);
	}
	if (rom_addr->rom_awb_module_info_start_addr >= 0) {
		info("%d. AWB info\n", ++i);
		info(" Module info : %s\n", finfo->awb_ver);
	}
	if (rom_addr->rom_shading_module_info_start_addr >= 0) {
		info("%d. Shading info\n", ++i);
		info(" Module info : %s\n", finfo->shading_ver);
	}

#ifdef USE_AE_CAL
	if (ae_cal_data) {
		if (ae_cal_data->rom_ae_module_info_start_addr >= 0) {
			info("%d. AE Data info\n", ++i);
			info(" Module info : %s\n", finfo->ae_cal_ver);
		}
	}
#endif

#ifdef SAMSUNG_LIVE_OUTFOCUS
	if (rom_addr->rom_dual_module_info_start_addr >= 0) {
		info("%d. Dual Data info\n", ++i);
		info(" Module info : %s\n", finfo->dual_data_ver);
	}
#endif

#ifdef ENABLE_REMOSAIC_CAPTURE
	if (rom_addr->rom_sensor_cal_module_info_start_addr >= 0) {
		info("%d. Sensor Cal data info\n", ++i);
		info(" Module info : %s\n", finfo->sensor_cal_data_ver);
	}
#endif

#ifdef USE_AP_PDAF
	if (rom_addr->rom_pdaf_module_info_start_addr >= 0) {
		info("%d. PDAF info\n", ++i);
		info(" Module info : %s\n", finfo->ap_pdaf_ver);
	}
#endif
}
#endif	//ROM_DEBUG

	info("---- EEPROM[%d] data info\n", position);

	/* CRC check */
	if (!is_sec_check_cal_crc32(buf, position) && (retry > 0)) {
		retry--;
		goto crc_retry;
	}

	if (finfo->header_ver[3] == 'L' || finfo->header_ver[3] == 'E')
		crc32_check_list[position][CRC32_CHECK_FACTORY] = crc32_check_list[position][CRC32_CHECK];
	else
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;


	if (specific->use_module_check) {
		/* Check this module is latest */
		if (sysfs_finfo[position].header_ver[10] >= rom_addr->camera_module_es_version) {
			check_latest_cam_module[position] = true;
		} else {
			check_latest_cam_module[position] = false;
		}
		/* Check this module is final for manufacture */
		if (sysfs_finfo[position].header_ver[10] == IS_LATEST_ROM_VERSION_M) {
			check_final_cam_module[position] = true;
		} else {
			check_final_cam_module[position] = false;
		}
	} else {
		check_latest_cam_module[position] = true;
		check_final_cam_module[position] = true;
	}

	cal_buf[0] = buf;
	is_sec_readcal_dump(specific, cal_buf, cal_size, position);

exit:
	return ret;
}

#if defined(SENSOR_OTP_5E9)
int otprom_5e9_check_to_read(struct i2c_client *client)
{
	u8 data8=0;
	int ret;
	ret = is_sensor_write8(client, 0x0A00, 0x01);
	msleep(1);
	ret = is_sensor_read8(client, 0x0A01, &data8);
	return ret;
}

int is_i2c_read_otp_5e9(struct is_core *core, struct i2c_client *client,
							void *buf, u32 start_addr, size_t size, int position)
{
	int page_num = 0;
	int reg_count = 0;
	int index = 0;
	int ret = 0;
	int32_t header_start_addr = 0;

	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		return ret;
	}

	header_start_addr = rom_addr->rom_header_cal_data_start_addr;

	page_num = OTP_5E9_GET_PAGE(start_addr, OTP_PAGE_START_ADDR, header_start_addr);
	reg_count = OTP_5E9_GET_REG(start_addr, OTP_PAGE_START_ADDR, header_start_addr);
	is_sensor_write8(client, OTP_PAGE_ADDR, page_num);
	ret = otprom_5e9_check_to_read(client);

	for (index = 0; index < size ; index++)
	{
		if (reg_count >= 64)
		{
			page_num++;
			reg_count = 0;
			is_sensor_write8(client, OTP_PAGE_ADDR, page_num);
			ret = otprom_5e9_check_to_read(client);
		}
		is_sensor_read8(client, OTP_REG_ADDR_START+reg_count, buf+index);
		reg_count++;
	}

	return ret;
}

int is_sec_readcal_otprom_5e9(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	int cal_size = 0;
	char *buf = NULL;
	char temp_cal_buf[0x10] = {0};
	char *cal_buf[2] = {NULL, NULL};
	u8 otp_bank = 0;
	u16 start_addr = 0;

	struct is_core *core = dev_get_drvdata(dev);
	struct is_rom_info *finfo = NULL;
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("is_sec_readcal_otprom_5e9 E\n");

	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	is_sec_get_cal_buf(position, &buf);
	cal_size = is_sec_get_max_cal_size(core, position);

	client = specific->rom_client[position];

	if (!client) {
		err("cis i2c client is NULL\n");
		return -EINVAL;
	}

	is_i2c_config(client, true);
	msleep(10);

	/* 0. write Sensor Init(global) */
	ret = is_sec_set_registers(client, sensor_Global, sensor_Global_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	/* 1. write stream on */
	is_sensor_write8(client, 0x0100, 0x01);
	msleep(50);

	/* 2. write OTP page */
	is_sensor_write8(client, OTP_PAGE_ADDR, 0x11);
	ret = otprom_5e9_check_to_read(client);

	is_sensor_read8(client, OTP_REG_ADDR_START, &otp_bank);
	info("otp_bank = %d\n", otp_bank);

	/* 3. selected page setting */
	switch(otp_bank) {
	case 0x1 :
		start_addr = OTP_START_ADDR;
		break;
	case 0x3 :
		start_addr = OTP_START_ADDR_BANK2;
		break;
	case 0x7 :
		start_addr = OTP_START_ADDR_BANK3;
		break;
	default :
		start_addr = OTP_START_ADDR;
		break;
	}

	info("otp_start_addr = %x\n", start_addr);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	ret = is_sec_set_registers(client, OTP_Init_reg, OTP_Init_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

crc_retry:

	/* read cal data
	 * 5E9 use page per 64byte */
	info("I2C read cal data\n\n");
	is_i2c_read_otp_5e9(core, client, buf, start_addr, OTP_USED_CAL_SIZE, position);

	/* HEARDER Data : Module/Manufacturer Information */
	memcpy(finfo->header_ver, &buf[rom_addr->rom_header_main_module_info_start_addr], IS_HEADER_VER_SIZE);
	finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';
	/* HEARDER Data : Cal Map Version */
	memcpy(finfo->cal_map_ver, &buf[rom_addr->rom_header_cal_map_ver_start_addr], IS_CAL_MAP_VER_SIZE);

	info("OTPROM header version = %s\n", finfo->header_ver);

	if (rom_addr->rom_header_main_oem_start_addr > 0) {
		finfo->oem_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_oem_start_addr]) - start_addr;
		finfo->oem_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_oem_end_addr]) - start_addr;
		info("OEM start = 0x%08x, end = 0x%08x\n", (finfo->oem_start_addr), (finfo->oem_end_addr));
	}

	if (rom_addr->rom_header_main_awb_start_addr > 0) {
		finfo->awb_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_awb_start_addr]) - start_addr;
		finfo->awb_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_awb_end_addr]) - start_addr;
		info("AWB start = 0x%08x, end = 0x%08x\n", (finfo->awb_start_addr), (finfo->awb_end_addr));
	}

	if (rom_addr->rom_header_main_shading_start_addr > 0) {
		finfo->shading_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_shading_start_addr]) - start_addr;
		finfo->shading_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_shading_start_addr]) - start_addr;
		info("Shading start = 0x%08x, end = 0x%08x\n", (finfo->shading_start_addr), (finfo->shading_end_addr));
	}

	if (rom_addr->rom_header_module_id_addr > 0) {
		memcpy(finfo->rom_module_id, &buf[rom_addr->rom_header_module_id_addr], IS_MODULE_ID_SIZE);
		finfo->rom_module_id[IS_MODULE_ID_SIZE] = '\0';
	}

	if (rom_addr->rom_header_main_sensor_id_addr > 0) {
		memcpy(finfo->rom_sensor_id, &buf[rom_addr->rom_header_main_sensor_id_addr], IS_SENSOR_ID_SIZE);
		finfo->rom_sensor_id[IS_SENSOR_ID_SIZE] = '\0';
	}

	if (rom_addr->rom_header_project_name_start_addr > 0) {
		memcpy(finfo->project_name, &buf[rom_addr->rom_header_project_name_start_addr], IS_PROJECT_NAME_SIZE);
		finfo->project_name[IS_PROJECT_NAME_SIZE] = '\0';
	}

	if (rom_addr->rom_header_checksum_addr > 0) {
		finfo->header_section_crc_addr = rom_addr->rom_header_checksum_addr;
	}

	if (rom_addr->rom_header_main_mtf_data_addr > 0) {
		finfo->mtf_data_addr = rom_addr->rom_header_main_mtf_data_addr;
	}

	if (rom_addr->rom_oem_module_info_start_addr > 0) {
		memcpy(finfo->oem_ver, &buf[rom_addr->rom_oem_module_info_start_addr], IS_OEM_VER_SIZE);
		finfo->oem_ver[IS_OEM_VER_SIZE] = '\0';
		finfo->oem_section_crc_addr = rom_addr->rom_oem_checksum_addr;
	}

	if (rom_addr->rom_awb_module_info_start_addr > 0) {
		memcpy(finfo->awb_ver, &buf[rom_addr->rom_awb_module_info_start_addr], IS_AWB_VER_SIZE);
		finfo->awb_ver[IS_AWB_VER_SIZE] = '\0';
		finfo->awb_section_crc_addr = rom_addr->rom_awb_checksum_addr;
	}

	if (rom_addr->rom_shading_module_info_start_addr > 0) {
		memcpy(finfo->shading_ver, &buf[rom_addr->rom_shading_module_info_start_addr], IS_SHADING_VER_SIZE);
		finfo->shading_ver[IS_SHADING_VER_SIZE] = '\0';
		finfo->shading_section_crc_addr = rom_addr->rom_shading_checksum_addr;
	}

	if (rom_addr->rom_oem_af_inf_position_addr && rom_addr->rom_oem_af_macro_position_addr) {
		finfo->af_cal_pan = *((u32 *)&buf[rom_addr->rom_oem_af_inf_position_addr]);
		finfo->af_cal_macro = *((u32 *)&buf[rom_addr->rom_oem_af_macro_position_addr]);
	}

	if (finfo->cal_map_ver[0] != 'V') {
		info("Camera: Cal Map version read fail or there's no available data.\n");
		/* it for case of CRC fail at re-work module  */
		if (retry == IS_CAL_RETRY_CNT && otp_bank != 0x1) {
			start_addr -= 0xf;
			info("%s : change start address(%x)\n",__func__, start_addr);
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;

		goto exit;
	}

	printk(KERN_INFO "Camera[%d]: OTPROM Cal map_version = %c%c%c%c\n", position,
		finfo->cal_map_ver[0], finfo->cal_map_ver[1], finfo->cal_map_ver[2], finfo->cal_map_ver[3]);

	/* debug info dump */
#ifdef ROM_DEBUG
	info("++++ OTPROM data info\n");
	info(" Header info\n");
	info(" Module info : %s\n", finfo->header_ver);
	info(" ID : %c\n", finfo->header_ver[FW_CORE_VER]);
	info(" Pixel num : %c%c\n", finfo->header_ver[FW_PIXEL_SIZE], finfo->header_ver[FW_PIXEL_SIZE + 1]);
	info(" ISP ID : %c\n", finfo->header_ver[FW_ISP_COMPANY]);
	info(" Sensor Maker : %c\n", finfo->header_ver[FW_SENSOR_MAKER]);
	info(" Year : %c\n", finfo->header_ver[FW_PUB_YEAR]);
	info(" Month : %c\n", finfo->header_ver[FW_PUB_MON]);
	info(" Release num : %c%c\n", finfo->header_ver[FW_PUB_NUM], finfo->header_ver[FW_PUB_NUM + 1]);
	info(" Manufacturer ID : %c\n", finfo->header_ver[FW_MODULE_COMPANY]);
	info(" Module ver : %c\n", finfo->header_ver[FW_VERSION_INFO]);
	info(" Module ID : %c%c%c%c%c%X%X%X%X%X\n",
			finfo->rom_module_id[0], finfo->rom_module_id[1], finfo->rom_module_id[2],
			finfo->rom_module_id[3], finfo->rom_module_id[4], finfo->rom_module_id[5],
			finfo->rom_module_id[6], finfo->rom_module_id[7], finfo->rom_module_id[8],
			finfo->rom_module_id[9]);
#endif
	info("---- OTPROM data info\n");

	/* CRC check */
	if (!is_sec_check_cal_crc32(buf, position) && (retry > 0)) {
		retry--;
		goto crc_retry;
	}

	/* 6. return to original mode */
	ret = is_sec_set_registers(client,
	sensor_mode_change_from_OTP_reg, sensor_mode_change_from_OTP_reg_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	if (finfo->header_ver[3] == 'L' || finfo->header_ver[3] == 'E')
		crc32_check_list[position][CRC32_CHECK_FACTORY] = crc32_check_list[position][CRC32_CHECK];
	else
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;

	if (specific->use_module_check) {
		/* Check this module is latest */
		if (sysfs_finfo[position].header_ver[10] >= rom_addr->camera_module_es_version) {
			check_latest_cam_module[position] = true;
		} else {
			check_latest_cam_module[position] = false;
		}
		/* Check this module is final for manufacture */
		if (sysfs_finfo[position].header_ver[10] == IS_LATEST_ROM_VERSION_M) {
			check_final_cam_module[position] = true;
		} else {
			check_final_cam_module[position] = false;
		}
	} else {
		check_latest_cam_module[position] = true;
		check_final_cam_module[position] = true;
	}

	/* For CAL DUMP */
	is_i2c_read_otp_5e9(core, client, temp_cal_buf, OTP_PAGE_START_ADDR, 0xf, position);
	cal_buf[0] = buf;
	cal_buf[1] = temp_cal_buf;
	is_sec_readcal_dump(specific, cal_buf, cal_size, position);

exit:
	//is_i2c_config(client, false);  /* not used 'default' */

	info("%s X\n", __func__);
	return ret;
}
#else //SENSOR_OTP_5E9 end

/* 'is_sec_readcal_otprom_legacy' is not modified to MCD_V2 yet.
 *  If it is used to this fuction, must modify */
#if 0
int is_sec_readcal_otprom_legacy(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	int cal_size = 0;
	char *buf = NULL;
	char *cal_buf[2] = {NULL, NULL};
	struct is_core *core = dev_get_drvdata(dev);
	struct is_rom_info *finfo = NULL;
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;

#ifdef OTP_BANK
	u8 data8 = 0;
	int otp_bank = 0;
#endif

#ifdef OTP_SINGLE_READ_ADDR
	int i = 0;
	u8 start_addr_h = 0;
	u8 start_addr_l= 0;
#endif
	u16 start_addr = 0;

	info("is_sec_readcal_otprom E\n");

	is_sec_get_sysfs_finfo_by_position(position, &finfo);
	is_sec_get_cal_buf(position, &buf);
	cal_size = is_sec_get_max_cal_size(core, position);

	client = specific->rom_client[position];

	if (!client) {
		err("eeprom i2c client is NULL\n");
		return -EINVAL;
	}

	is_i2c_config(client, true);
	msleep(10);

#if defined(OTP_NEED_INIT_SETTING)
	/* 0. sensor init */
	if (!force_caldata_dump) {
		ret = specific->cis_init_reg_write();
		if (unlikely(ret)) {
			err("failed to is_i2c_write (%d)\n", ret);
			ret = -EINVAL;
			goto exit;
		}
	}
#endif

#if defined(OTP_NEED_INIT_DIRECT)
	ret = is_sec_set_registers(client,
	sensor_Global, sensor_Global_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
#endif


#if defined(OTP_MODE_CHANGE)
	/* 1. mode change to OTP */
	ret = is_sec_set_registers(client,
	sensor_mode_change_to_OTP_reg, sensor_mode_change_to_OTP_reg_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
#endif

#if defined(OTP_BANK)
#if defined(OTP_SINGLE_READ_ADDR)
	/* 2. single read OTP Bank */
	is_sensor_write8(client, OTP_START_ADDR_HIGH, OTP_BANK_ADDR_HIGH);
	is_sensor_write8(client, OTP_START_ADDR_LOW, OTP_BANK_ADDR_LOW);
	is_sensor_write8(client, OTP_SINGLE_READ, 0x01);
	is_sensor_read8(client, OTP_SINGLE_READ_ADDR, &data8);
	otp_bank = data8;
	info("Camera: otp_bank = %d\n", otp_bank);

	/* 3. selected page setting */
	switch(otp_bank) {
	case 1 :
		start_addr_h = OTP_BANK1_START_ADDR_HIGH;
		start_addr_l = OTP_BANK1_START_ADDR_LOW;
		break;
	case 3 :
		start_addr_h = OTP_BANK2_START_ADDR_HIGH;
		start_addr_l = OTP_BANK2_START_ADDR_LOW;
		break;
	case 7 :
		start_addr_h = OTP_BANK3_START_ADDR_HIGH;
		start_addr_l = OTP_BANK3_START_ADDR_LOW;
		break;
	default :
		start_addr_h = OTP_BANK1_START_ADDR_HIGH;
		start_addr_l = OTP_BANK1_START_ADDR_LOW;
		break;
	}
	start_addr = ((start_addr_h << 8)&0xff00) | (start_addr_l&0xff);
#else
	/* 2. read OTP Bank */
#if defined(SENSOR_OTP_HYNIX)
	is_sensor_write8(client, OTP_READ_START_ADDR_HIGH, (OTP_BANK_ADDR>>8)&0xff);
	is_sensor_write8(client, OTP_READ_START_ADDR_LOW, (OTP_BANK_ADDR)&0xff);
	is_sensor_write8(client, OTP_READ_MODE_ADDR, 0x01);
	is_sensor_read8(client, OTP_READ_ADDR, &data8);
#else
	is_sensor_read8(client, OTP_BANK_ADDR, &data8);
#endif
	otp_bank = data8;
	info("Camera: otp_bank = %d\n", otp_bank);
#if defined(SENSOR_OTP_HYNIX)
    /* 3. selected page setting */
	switch(otp_bank) {
	case 0x1 :
		start_addr = OTP_START_ADDR;
		break;
	case 0x13 :
		start_addr = OTP_START_ADDR_BANK2;
		break;
	case 0x37 :
		start_addr = OTP_START_ADDR_BANK3;
		break;
	default :
		start_addr = OTP_START_ADDR;
		break;
	}

	info("Camera: otp_start_addr = %x\n", start_addr);

	ret = is_i2c_read_burst(client, finfo->cal_map_ver,
					OTP_HEADER_CAL_MAP_VER_START_ADDR_FRONT+start_addr,
					IS_CAL_MAP_VER_SIZE);
	ret = is_i2c_read_burst(client, finfo->header_ver,
					OTP_HEADER_VERSION_START_ADDR_FRONT+start_addr,
					IS_HEADER_VER_SIZE);

#else
	start_addr = OTP_START_ADDR;

	/* 3. selected page setting */
	switch(otp_bank) {
	case 1 :
		ret = is_sec_set_registers(client,
		OTP_first_page_select_reg, OTP_first_page_select_reg_size);
		break;
	case 3 :
		ret = is_sec_set_registers(client,
		OTP_second_page_select_reg, OTP_second_page_select_reg_size);
		break;
	default :
		ret = is_sec_set_registers(client,
		OTP_first_page_select_reg, OTP_first_page_select_reg_size);
		break;
	}

	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
#endif
#endif
#endif

crc_retry:

#if defined(OTP_BANK)
#if defined(OTP_SINGLE_READ_ADDR)
	is_sensor_write8(client, OTP_START_ADDR_HIGH, start_addr_h);
	is_sensor_write8(client, OTP_START_ADDR_LOW, start_addr_l);
	is_sensor_write8(client, OTP_SINGLE_READ, 0x01);

	/* 5. full read cal data */
	info("Camera: I2C read full cal data\n\n");
	for (i = 0; i < OTP_USED_CAL_SIZE; i++) {
		is_sensor_read8(client, OTP_SINGLE_READ_ADDR, &data8);
		buf[i] = data8;
	}
#else
	/* read cal data */
	info("Camera: I2C read cal data\n\n");
#if defined(SENSOR_OTP_HYNIX)
	is_i2c_read_burst(client, buf, start_addr, OTP_USED_CAL_SIZE);
#else
	is_i2c_read(client, buf, start_addr, OTP_USED_CAL_SIZE);
#endif
#endif
#endif

	if (position == SENSOR_POSITION_FRONT) {
#if defined(CONFIG_CAMERA_OTPROM_SUPPORT_FRONT)
		info("FRONT OTPROM header version = %s\n", finfo->header_ver);
#if defined(OTP_HEADER_OEM_START_ADDR_FRONT)
		finfo->oem_start_addr = *((u32 *)&buf[OTP_HEADER_OEM_START_ADDR_FRONT]) - start_addr;
		finfo->oem_end_addr = *((u32 *)&buf[OTP_HEADER_OEM_END_ADDR_FRONT]) - start_addr;
		info("OEM start = 0x%08x, end = 0x%08x\n",
			(finfo->oem_start_addr), (finfo->oem_end_addr));
#endif
#if defined(OTP_HEADER_AWB_START_ADDR_FRONT)
#ifdef OTP_HEADER_DIRECT_ADDR_FRONT
		finfo->awb_start_addr = OTP_HEADER_AWB_START_ADDR_FRONT - start_addr;
		finfo->awb_end_addr = OTP_HEADER_AWB_END_ADDR_FRONT - start_addr;
#else
		finfo->awb_start_addr = *((u32 *)&buf[OTP_HEADER_AWB_START_ADDR_FRONT]) - start_addr;
		finfo->awb_end_addr = *((u32 *)&buf[OTP_HEADER_AWB_END_ADDR_FRONT]) - start_addr;
#endif
		info("AWB start = 0x%08x, end = 0x%08x\n",
			(finfo->awb_start_addr), (finfo->awb_end_addr));
#endif
#if defined(OTP_HEADER_SHADING_START_ADDR_FRONT)
		finfo->shading_start_addr = *((u32 *)&buf[OTP_HEADER_AP_SHADING_START_ADDR_FRONT]) - start_addr;
		finfo->shading_end_addr = *((u32 *)&buf[OTP_HEADER_AP_SHADING_END_ADDR_FRONT]) - start_addr;
		info("Shading start = 0x%08x, end = 0x%08x\n",
			(finfo->shading_start_addr), (finfo->shading_end_addr));
		if (finfo->shading_end_addr > 0x3AFF) {
			err("Shading end_addr has error!! 0x%08x", finfo->shading_end_addr);
			finfo->shading_end_addr = 0x3AFF;
		}
#endif
		/* HEARDER Data : Module/Manufacturer Information */
		memcpy(finfo->header_ver, &buf[OTP_HEADER_VERSION_START_ADDR_FRONT], IS_HEADER_VER_SIZE);
		finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';
		/* HEARDER Data : Cal Map Version */
		memcpy(finfo->cal_map_ver,
			   &buf[OTP_HEADER_CAL_MAP_VER_START_ADDR_FRONT], IS_CAL_MAP_VER_SIZE);
#if defined(OTP_HEADER_MODULE_ID_ADDR_FRONT)
		memcpy(finfo->eeprom_front_module_id,
			   &buf[OTP_HEADER_MODULE_ID_ADDR_FRONT], IS_MODULE_ID_SIZE);
		finfo->eeprom_front_module_id[IS_MODULE_ID_SIZE] = '\0';
#endif

#if defined(OTP_HEADER_PROJECT_NAME_START_ADDR_FRONT)
		memcpy(finfo->project_name,
			   &buf[OTP_HEADER_PROJECT_NAME_START_ADDR_FRONT], IS_PROJECT_NAME_SIZE);
		finfo->project_name[IS_PROJECT_NAME_SIZE] = '\0';
#endif
		finfo->header_section_crc_addr = OTP_CHECKSUM_HEADER_ADDR_FRONT;
#if defined(OTP_HEADER_OEM_START_ADDR_FRONT)
		/* OEM Data : Module/Manufacturer Information */
		memcpy(finfo->oem_ver, &buf[OTP_OEM_VER_START_ADDR_FRONT], IS_OEM_VER_SIZE);
		finfo->oem_ver[IS_OEM_VER_SIZE] = '\0';
		finfo->oem_section_crc_addr = OTP_CHECKSUM_OEM_ADDR_FRONT;
#endif
#if defined(OTP_AWB_VER_START_ADDR_FRONT)
		/* AWB Data : Module/Manufacturer Information */
		memcpy(finfo->awb_ver, &buf[OTP_AWB_VER_START_ADDR_FRONT], IS_AWB_VER_SIZE);
		finfo->awb_ver[IS_AWB_VER_SIZE] = '\0';
		finfo->awb_section_crc_addr = OTP_CHECKSUM_AWB_ADDR_FRONT;
#endif
#if defined(OTP_AP_SHADING_VER_START_ADDR_FRONT)
		/* SHADING Data : Module/Manufacturer Information */
		memcpy(finfo->shading_ver, &buf[OTP_AP_SHADING_VER_START_ADDR_FRONT], IS_SHADING_VER_SIZE);
		finfo->shading_ver[IS_SHADING_VER_SIZE] = '\0';
		finfo->shading_section_crc_addr = OTP_CHECKSUM_AP_SHADING_ADDR_FRONT;
#endif
#endif
	} else {
#if defined(CONFIG_CAMERA_OTPROM_SUPPORT_REAR)
		finfo->oem_start_addr = *((u32 *)&buf[OTP_HEADER_OEM_START_ADDR]) - start_addr;
		finfo->oem_end_addr = *((u32 *)&buf[OTP_HEADER_OEM_END_ADDR]) - start_addr;
		info("OEM start = 0x%08x, end = 0x%08x\n",
			(finfo->oem_start_addr), (finfo->oem_end_addr));
#if defined(OTP_HEADER_AWB_START_ADDR)
		finfo->awb_start_addr = *((u32 *)&buf[OTP_HEADER_AWB_START_ADDR]) - start_addr;
		finfo->awb_end_addr = *((u32 *)&buf[OTP_HEADER_AWB_END_ADDR]) - start_addr;
		info("AWB start = 0x%08x, end = 0x%08x\n",
			(finfo->awb_start_addr), (finfo->awb_end_addr));
#endif
#if defined(OTP_HEADER_AP_SHADING_START_ADDR)
		finfo->shading_start_addr = *((u32 *)&buf[OTP_HEADER_AP_SHADING_START_ADDR]) - start_addr;
		finfo->shading_end_addr = *((u32 *)&buf[OTP_HEADER_AP_SHADING_END_ADDR]) - start_addr;
		if (finfo->shading_end_addr > 0x1fff) {
			err("Shading end_addr has error!! 0x%08x", finfo->shading_end_addr);
			finfo->setfile_end_addr = 0x1fff;
		}
		info("Shading start = 0x%08x, end = 0x%08x\n",
			(finfo->shading_start_addr), (finfo->shading_end_addr));
#endif
		/* HEARDER Data : Module/Manufacturer Information */
		memcpy(finfo->header_ver, &buf[OTP_HEADER_VERSION_START_ADDR], IS_HEADER_VER_SIZE);
		finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';
		/* HEARDER Data : Cal Map Version */
		memcpy(finfo->cal_map_ver, &buf[OTP_HEADER_CAL_MAP_VER_START_ADDR], IS_CAL_MAP_VER_SIZE);

#if defined(OTP_HEADER_PROJECT_NAME_START_ADDR)
		memcpy(finfo->project_name, &buf[OTP_HEADER_PROJECT_NAME_START_ADDR], IS_PROJECT_NAME_SIZE);
		finfo->project_name[IS_PROJECT_NAME_SIZE] = '\0';
		finfo->header_section_crc_addr = OTP_CHECKSUM_HEADER_ADDR;
#endif
#if defined(OTP_OEM_VER_START_ADDR)
		/* OEM Data : Module/Manufacturer Information */
		memcpy(finfo->oem_ver, &buf[OTP_OEM_VER_START_ADDR], IS_OEM_VER_SIZE);
		finfo->oem_ver[IS_OEM_VER_SIZE] = '\0';
		finfo->oem_section_crc_addr = OTP_CHECKSUM_OEM_ADDR;
#endif
#if defined(OTP_AWB_VER_START_ADDR)
		/* AWB Data : Module/Manufacturer Information */
		memcpy(finfo->awb_ver, &buf[OTP_AWB_VER_START_ADDR], IS_AWB_VER_SIZE);
		finfo->awb_ver[IS_AWB_VER_SIZE] = '\0';
		finfo->awb_section_crc_addr = OTP_CHECKSUM_AWB_ADDR;
#endif
#if defined(OTP_AP_SHADING_VER_START_ADDR)
		/* SHADING Data : Module/Manufacturer Information */
		memcpy(finfo->shading_ver, &buf[OTP_AP_SHADING_VER_START_ADDR], IS_SHADING_VER_SIZE);
		finfo->shading_ver[IS_SHADING_VER_SIZE] = '\0';
		finfo->shading_section_crc_addr = OTP_CHECKSUM_AP_SHADING_ADDR;

		finfo->af_cal_pan = *((u32 *)&buf[OTPROM_AF_CAL_PAN_ADDR]);
		finfo->af_cal_macro = *((u32 *)&buf[OTPROM_AF_CAL_MACRO_ADDR]);
#endif
#endif
	}

	if (finfo->cal_map_ver[0] != 'V') {
		info("Camera: Cal Map version read fail or there's no available data.\n");
		crc32_check_factory_front = false;
		goto exit;
	}

	info("Camera: OTPROM Cal map_version = %c%c%c%c\n", finfo->cal_map_ver[0],
			finfo->cal_map_ver[1], finfo->cal_map_ver[2], finfo->cal_map_ver[3]);

	/* debug info dump */
	info("++++ OTPROM data info\n");
	info(" Header info\n");
	info(" Module info : %s\n", finfo->header_ver);
	info(" ID : %c\n", finfo->header_ver[FW_CORE_VER]);
	info(" Pixel num : %c%c\n", finfo->header_ver[FW_PIXEL_SIZE], finfo->header_ver[FW_PIXEL_SIZE + 1]);
	info(" ISP ID : %c\n", finfo->header_ver[FW_ISP_COMPANY]);
	info(" Sensor Maker : %c\n", finfo->header_ver[FW_SENSOR_MAKER]);
	info(" Year : %c\n", finfo->header_ver[FW_PUB_YEAR]);
	info(" Month : %c\n", finfo->header_ver[FW_PUB_MON]);
	info(" Release num : %c%c\n", finfo->header_ver[FW_PUB_NUM], finfo->header_ver[FW_PUB_NUM + 1]);
	info(" Manufacturer ID : %c\n", finfo->header_ver[FW_MODULE_COMPANY]);
	info(" Module ver : %c\n", finfo->header_ver[FW_VERSION_INFO]);
	info(" Module ID : %c%c%c%c%c%X%X%X%X%X\n",
			finfo->eeprom_front_module_id[0], finfo->eeprom_front_module_id[1], finfo->eeprom_front_module_id[2],
			finfo->eeprom_front_module_id[3], finfo->eeprom_front_module_id[4], finfo->eeprom_front_module_id[5],
			finfo->eeprom_front_module_id[6], finfo->eeprom_front_module_id[7], finfo->eeprom_front_module_id[8],
			finfo->eeprom_front_module_id[9]);

	info("---- OTPROM data info\n");

	/* CRC check */
	if (position == SENSOR_POSITION_FRONT || position == SENSOR_POSITION_FRONT2) {
		if (!is_sec_check_front_otp_crc32(buf) && (retry > 0)) {
			retry--;
			goto crc_retry;
		}
	} else {
		if (!is_sec_check_cal_crc32(buf, position) && (retry > 0)) {
			retry--;
			goto crc_retry;
		}
	}

#if defined(OTP_MODE_CHANGE)
	/* 6. return to original mode */
	ret = is_sec_set_registers(client,
	sensor_mode_change_from_OTP_reg, sensor_mode_change_from_OTP_reg_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
#endif

	if (position == SENSOR_POSITION_FRONT) {
		if (finfo->header_ver[3] == 'L' || finfo->header_ver[3] == 'E') {
			crc32_check_factory_front = crc32_check_front;
		} else {
			crc32_check_factory_front = false;
		}
	} else {
		if (finfo->header_ver[3] == 'L' || finfo->header_ver[3] == 'E') {
			crc32_check_factory = crc32_check;
		} else {
			crc32_check_factory = false;
		}
	}

	if (!specific->use_module_check) {
		is_latest_cam_module = true;
	} else {
#if defined(CAMERA_MODULE_ES_VERSION_REAR)
		if (sysfs_finfo.header_ver[10] >= CAMERA_MODULE_ES_VERSION_REAR) {
			is_latest_cam_module = true;
		} else
#endif
		{
			is_latest_cam_module = false;
		}
	}

	if (position == SENSOR_POSITION_REAR) {
		if (specific->use_module_check) {
			if (finfo->header_ver[10] == IS_LATEST_FROM_VERSION_M) {
				is_final_cam_module = true;
			} else {
				is_final_cam_module = false;
			}
		} else {
			is_final_cam_module = true;
		}
	} else {
		if (specific->use_module_check) {
			if (finfo->header_ver[10] == IS_LATEST_FROM_VERSION_M) {
				is_final_cam_module_front = true;
			} else {
				is_final_cam_module_front = false;
			}
		} else {
			is_final_cam_module_front = true;
		}
	}

	cal_buf[0] = buf;
	is_sec_readcal_dump(specific, cal_buf, cal_size, position);

exit:
	//is_i2c_config(client, false);  /* not used 'default' */

	info("is_sec_readcal_otprom X\n");

	return ret;
}
#endif //if 0
#endif //!SENSOR_OTP_5E9

int is_sec_readcal_jdm_checksum_and_dump(char * buf, int position)
{
	int ret = 0;
	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];
	struct rom_ap2ap_standard_cal_data *ap2ap_standard_cal_data;

	int cal_size = 0;
	char *m_cal_buf[2] = {NULL, NULL};
	int32_t seg_dump_addr[JDM_MAX_SEG];
	int i, j;
	int32_t curr_addr = 0x00;
	int checksum = 0;
	int seg_checksum = 0;	/* checksum of every segment */
	int total_checksum = 0;

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n",__func__ );

	cal_size = is_sec_get_max_cal_size(core, position);

	if (rom_addr->extend_cal_addr) {
		ap2ap_standard_cal_data = (struct rom_ap2ap_standard_cal_data *)is_sec_search_rom_extend_data(
			rom_addr->extend_cal_addr, EXTEND_AP2AP_STANDARD_CAL);
		if (ap2ap_standard_cal_data) {
			if ((ap2ap_standard_cal_data->rom_orig_end_addr != -1) && (ap2ap_standard_cal_data->rom_orig_start_addr != -1))
				cal_size = ap2ap_standard_cal_data->rom_orig_end_addr - ap2ap_standard_cal_data->rom_orig_start_addr + 1;
		} else {
			err("ap2ap_standard_cal_data is NULL");
			ret = -EINVAL;
			goto exit;
		}
	}

	/* Checksum check */
	for (i = 0; i < ap2ap_standard_cal_data->rom_num_of_segments; i++) {
		seg_dump_addr[i] = curr_addr;
		curr_addr += ap2ap_standard_cal_data->rom_seg_size[i];
	}

	for (i = 0; i < ap2ap_standard_cal_data->rom_num_of_segments; i++) {
		checksum = 0;
		for (j = 0; j < ap2ap_standard_cal_data->rom_seg_checksum_len[i]; j++) {
			checksum = (checksum + buf[seg_dump_addr[i] + j]) % 0xFF;
		}
		seg_checksum = buf[seg_dump_addr[i] + j];
		total_checksum = (total_checksum + checksum + seg_checksum) % 0xFF;
		if (checksum + 1 != seg_checksum) {
			err("Camera[%d]: Checksum error at the segment %d (0x%02X != 0x%02X)", position, i + 1,  checksum + 1, seg_checksum);
			ret = -EINVAL;
			goto exit;
		}
		else {
			info("Checksum success for segment %d", i + 1);
		}
	}

	/* Total checksum check */
	if (total_checksum + 1 != buf[curr_addr]) {
		err("Camera[%d]: Total Checksum Error (0x%02X != 0x%02X)", position, total_checksum + 1, buf[curr_addr]);
		ret = -EINVAL;
		goto exit;
	}
	else{
		info("Total checksum success");
	}

	/* Cal dump */
	m_cal_buf[0] = buf;
	is_sec_readcal_dump(specific, m_cal_buf, cal_size, position);

exit:
	return ret;
}

int is_sec_readcal_otprom_buffer(char * buf, int position)
{
	int ret = 0;
	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];
	struct is_rom_info *finfo = NULL;
#ifndef USE_AP2AP_CAL_CONVERSION
	char *m_cal_buf[2] = {NULL, NULL};
	int cal_size = 0;
#endif

	struct rom_standard_cal_data *standard_cal_data = NULL;

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n",__func__ );

	is_sec_get_sysfs_finfo_by_position(position, &finfo);

	/* HEADER Data : Module/Manufacturer Information */
	memcpy(finfo->header_ver, &buf[rom_addr->rom_header_main_module_info_start_addr], IS_HEADER_VER_SIZE);
	finfo->header_ver[IS_HEADER_VER_SIZE] = '\0';

	/* HEADER Data : Cal Map Version */
	if (rom_addr->rom_header_cal_map_ver_start_addr > 0) {
		memcpy(finfo->cal_map_ver, &buf[rom_addr->rom_header_cal_map_ver_start_addr], IS_CAL_MAP_VER_SIZE);
	}

	info("OTPROM header version = %s\n", finfo->header_ver);

	if (rom_addr->rom_header_module_id_addr > 0) {
		memcpy(finfo->rom_module_id, &buf[rom_addr->rom_header_module_id_addr], IS_MODULE_ID_SIZE);
		finfo->rom_module_id[IS_MODULE_ID_SIZE] = '\0';
	} else {
		/* Use ASCII char '0' for initial 5 characters according to module id format */
		memcpy(finfo->rom_module_id, "00000", 0x5);
		finfo->rom_module_id[IS_MODULE_ID_SIZE] = '\0';
	}

	if (rom_addr->rom_header_main_sensor_id_addr > 0) {
		memcpy(finfo->rom_sensor_id, &buf[rom_addr->rom_header_main_sensor_id_addr], IS_SENSOR_ID_SIZE);
		finfo->rom_sensor_id[IS_SENSOR_ID_SIZE] = '\0';
	}

	if (rom_addr->rom_header_project_name_start_addr > 0) {
		memcpy(finfo->project_name, &buf[rom_addr->rom_header_project_name_start_addr], IS_PROJECT_NAME_SIZE);
		finfo->project_name[IS_PROJECT_NAME_SIZE] = '\0';
	}

	if (rom_addr->rom_header_checksum_addr > 0) {
		finfo->header_section_crc_addr = rom_addr->rom_header_checksum_addr;
	}

	if (rom_addr->rom_header_cal_map_ver_start_addr > 0) {
		if (finfo->cal_map_ver[0] == 'V') {
			printk(KERN_INFO "Camera[%d]: OTPROM Cal map_version = %c%c%c%c\n", position,
				finfo->cal_map_ver[0], finfo->cal_map_ver[1], finfo->cal_map_ver[2], finfo->cal_map_ver[3]);
		} else {
			err("Camera: Cal Map version read fail or there's no available data.\n");
			ret = -EINVAL;
			goto exit;
		}
	} else {
		info("Camera: Cal Map version not defined in rom_addr\n");
	}

	/* debug info dump */
#ifdef ROM_DEBUG
	info("++++ OTPROM data info\n");
	info(" Header info\n");
	info(" Module info : %s\n", finfo->header_ver);
	info(" ID : %c\n", finfo->header_ver[FW_CORE_VER]);
	info(" Pixel num : %c%c\n", finfo->header_ver[FW_PIXEL_SIZE], finfo->header_ver[FW_PIXEL_SIZE + 1]);
	info(" ISP ID : %c\n", finfo->header_ver[FW_ISP_COMPANY]);
	info(" Sensor Maker : %c\n", finfo->header_ver[FW_SENSOR_MAKER]);
	info(" Year : %c\n", finfo->header_ver[FW_PUB_YEAR]);
	info(" Month : %c\n", finfo->header_ver[FW_PUB_MON]);
	info(" Release num : %c%c\n", finfo->header_ver[FW_PUB_NUM], finfo->header_ver[FW_PUB_NUM + 1]);
	info(" Manufacturer ID : %c\n", finfo->header_ver[FW_MODULE_COMPANY]);
	info(" Module ver : %c\n", finfo->header_ver[FW_VERSION_INFO]);
	if (rom_addr->rom_header_module_id_addr > 0) {
		info(" Module ID : %c%c%c%c%c%X%X%X%X%X\n",
				finfo->rom_module_id[0], finfo->rom_module_id[1], finfo->rom_module_id[2],
				finfo->rom_module_id[3], finfo->rom_module_id[4], finfo->rom_module_id[5],
				finfo->rom_module_id[6], finfo->rom_module_id[7], finfo->rom_module_id[8],
				finfo->rom_module_id[9]);
	} else {
		info(" Module ID : NOT DEFINED\n");
	}
#endif
	info("---- OTPROM data info\n");

/////////////////////////////////////////////////////////////////////////////////////
/* Header Data */
/////////////////////////////////////////////////////////////////////////////////////

	/* Header Data: OEM */
	if (rom_addr->rom_header_main_oem_start_addr >= 0) {
		finfo->oem_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_oem_start_addr]);
		finfo->oem_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_oem_end_addr]);
		finfo->oem_section_crc_addr = rom_addr->rom_oem_checksum_addr;
	}

	/* Header Data: AWB */
	if (rom_addr->rom_header_main_awb_start_addr >= 0) {
		finfo->awb_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_awb_start_addr]);
		finfo->awb_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_awb_end_addr]);
		finfo->awb_section_crc_addr = rom_addr->rom_awb_checksum_addr;
	}

	/* Header Data: Shading */
	if (rom_addr->rom_header_main_shading_start_addr >= 0) {
		finfo->shading_start_addr = *((u32 *)&buf[rom_addr->rom_header_main_shading_start_addr]);
		finfo->shading_end_addr = *((u32 *)&buf[rom_addr->rom_header_main_shading_end_addr]);
		finfo->shading_section_crc_addr = rom_addr->rom_shading_checksum_addr;
	}

	if (rom_addr->extend_cal_addr) {
		standard_cal_data = (struct rom_standard_cal_data *)is_sec_search_rom_extend_data(rom_addr->extend_cal_addr, EXTEND_STANDARD_CAL);
		if (standard_cal_data && standard_cal_data->rom_awb_start_addr >= 0) {
			finfo->awb_start_addr = standard_cal_data->rom_awb_start_addr;
			finfo->awb_end_addr = standard_cal_data->rom_awb_end_addr;
			finfo->awb_section_crc_addr = rom_addr->rom_awb_checksum_addr;
			info("%s before sec2lsi  finfo->awb_start_addr is 0x%08X", __func__, finfo->awb_start_addr);
		}
		if (standard_cal_data && standard_cal_data->rom_shading_start_addr >= 0) {
			finfo->shading_start_addr = standard_cal_data->rom_shading_start_addr;
			finfo->shading_end_addr = standard_cal_data->rom_shading_end_addr;
			finfo->shading_section_crc_addr = rom_addr->rom_shading_checksum_addr;
			info("%s before sec2lsi  finfo->shading_start_addr is 0x%08X", __func__, finfo->shading_start_addr);
		}
	}

	/* Header Data: MTF Data (Resolution) */
	if (rom_addr->rom_header_main_mtf_data_addr >= 0) { 
		finfo->mtf_data_addr = rom_addr->rom_header_main_mtf_data_addr;
	}

	/* CRC check */
	if (!is_sec_check_cal_crc32(buf, position)) {
		ret = -EINVAL;
		goto exit;
	}

	if (finfo->header_ver[3] == 'L' || finfo->header_ver[3] == 'E' || finfo->header_ver[3] == 'J')
		crc32_check_list[position][CRC32_CHECK_FACTORY] = crc32_check_list[position][CRC32_CHECK];
	else
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;

	if (specific->use_module_check) {
		/* Check this module is latest */
		if (sysfs_finfo[position].header_ver[10] >= rom_addr->camera_module_es_version) {
			check_latest_cam_module[position] = true;
		} else {
			check_latest_cam_module[position] = false;
		}
		/* Check this module is final for manufacture */
		if (sysfs_finfo[position].header_ver[10] == IS_LATEST_ROM_VERSION_M) {
			check_final_cam_module[position] = true;
		} else {
			check_final_cam_module[position] = false;
		}
	} else {
		check_latest_cam_module[position] = true;
		check_final_cam_module[position] = true;
	}
#ifndef USE_AP2AP_CAL_CONVERSION
	/* For JDM sensor, cal dump is handled by is_sec_readcal_jdm_checksum_and_dump() */
	cal_size = is_sec_get_max_cal_size(core, position);

	m_cal_buf[0] = buf;
	is_sec_readcal_dump(specific, m_cal_buf, cal_size, position);
#endif
exit:
	return ret;
}

#if defined(SENSOR_OTP_SR846)
int is_i2c_read_otp_sr846(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	int ret = 0;
	int index = 0;
	u16 curr_addr = start_addr;
	u8 start_addr_h = 0;
	u8 start_addr_l = 0;

	for (index = 0; index < size ; index++) {
		start_addr_h = ((curr_addr >> 8) & 0x1F);
		start_addr_l = (curr_addr & 0xFF);
		is_sensor_write8(client, SR846_OTP_ACCESS_ADDR_H, start_addr_h); /* upper 16bit */
		is_sensor_write8(client, SR846_OTP_ACCESS_ADDR_L, start_addr_l); /* lower 16bit */
		is_sensor_write8(client, SR846_OTP_MODE_ADDR, 0x01); /* OTP continuous read mode */
		ret = is_sensor_read8(client, SR846_OTP_READ_ADDR, &buf[index]); /* OTP read */
		if (unlikely(ret)) {
			err("failed to is_sensor_addr8_read8 (%d)\n", ret);
			goto exit;
		}
		curr_addr++;
	}

exit:
	return ret;
}

int is_sec_readcal_otprom_sr846(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	int i2c_retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u16 start_addr = 0;
	u8 otp_bank;

	struct is_core *core = dev_get_drvdata(dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_sec_get_cal_buf(position, &buf);

	client = specific->rom_client[position];

	if (!client) {
		err("cis i2c client is NULL\n");
		return -EINVAL;
	}

	is_i2c_config(client, true);
	msleep(10);

	/* 1. OTP Initial Settings */
	i2c_retry=IS_CAL_RETRY_CNT;
i2c_write_retry_initial:
	ret = is_sec_set_registers(client, sensor_otp_sr846_initial, sensor_otp_sr846_initial_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		if (i2c_retry >= 0) {
			i2c_retry--;
			msleep(50);
			goto i2c_write_retry_initial;
		}
		ret = -EINVAL;
		goto exit;
	}

	is_sensor_write8(client, SR846_CP_TRIM_H_ADDR, 0x01); /* CP TRIM_H */
	is_sensor_write8(client, SR846_IPGM_TRIM_H_ADDR, 0x09); /* IPGM TRIM_H */
	is_sensor_write8(client, SR846_FSYNC_OUTPUT_ENABLE_ADDR, 0x00); /* Fsync(OTP busy) output enable */
	is_sensor_write8(client, SR846_FSYNC_OUTPUT_DRIVABILITY_ADDR, 0x07); /* Fsync(OTP busy) output drivability */
	is_sensor_write8(client, SR846_OTP_R_W_MODE_ADDR, 0x10); /* OTP R/W mode */
	is_sensor_write8(client, SR846_STANDBY_ADDR, 0x01); /* standby off */
	msleep(10); /* sleep 10msec */

	/* Read OTP page */
	is_sensor_write8(client, SR846_OTP_ACCESS_ADDR_H, ((SR846_OTP_BANK_SELECT_ADDR >> 8) & 0x1F));
	is_sensor_write8(client, SR846_OTP_ACCESS_ADDR_L, (SR846_OTP_BANK_SELECT_ADDR & 0xFF));
	is_sensor_write8(client, SR846_OTP_MODE_ADDR, 0x01);

	is_sensor_read8(client, SR846_OTP_READ_ADDR, &otp_bank);
	info("%s: otp_bank = %d\n", __func__, otp_bank);

	/* select start address */
	switch (otp_bank) {
	case 0x01 :
		start_addr = SR846_OTP_START_ADDR_BANK1;
		break;
	case 0x03 :
		start_addr = SR846_OTP_START_ADDR_BANK2;
		break;
	default :
		start_addr = SR846_OTP_START_ADDR_BANK1;
		break;
	}

	info("%s: otp_start_addr = %x\n", __func__, start_addr);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}
	
crc_retry:
	/* 3. Read OTP Cal Data */
	info("I2C read cal data\n");
	is_i2c_read_otp_sr846(client, buf, start_addr, SR846_OTP_USED_CAL_SIZE);

	/* 4. Parse Cal */
	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;
	}

exit:
	/* 5. OTP read off */
	is_sensor_write8(client, SR846_STREAM_OFF_ADDR, 0x0A); /* stream off */
	is_sensor_write8(client, SR846_STANDBY_ADDR, 0x00);
	msleep(10); /* sleep 10msec */
	is_sensor_write8(client, SR846_OTP_MODE_ADDR, 0x00);
	is_sensor_write8(client, SR846_OTP_R_W_MODE_ADDR, 0x00); /* display mode */
	msleep(1); /* sleep 1msec */
	info("%s X\n", __func__);
	return ret;
}
#endif /* SENSOR_OTP_SR846 */

#if defined(SENSOR_OTP_4HA)
u16 is_i2c_select_otp_bank_4ha(struct i2c_client *client) {
	int ret = 0;
	u8 otp_bank = 0;
	u16 curr_page = 0;
	
	//The Bank details itself is present in bank-1 page-0
	is_sensor_write8(client, S5K4HA_OTP_PAGE_SELECT_ADDR, S5K4HA_OTP_START_PAGE_BANK1);
	ret = is_sensor_read8(client, S5K4HA_OTP_BANK_SELECT, &otp_bank);
	if (unlikely(ret)) {
		err("failed to is_sensor_read8 (%d). OTP Bank selection failed\n", ret);
		goto exit;
	}
	info("%s otp_bank = %d\n", __func__, otp_bank);

	switch(otp_bank) {
	case 0x01 :
		curr_page = S5K4HA_OTP_START_PAGE_BANK1;
		break;
	case 0x03 :
		curr_page = S5K4HA_OTP_START_PAGE_BANK2;
		break;
	default :
		curr_page = S5K4HA_OTP_START_PAGE_BANK1;
		break;
	}
	is_sensor_write8(client, S5K4HA_OTP_PAGE_SELECT_ADDR, curr_page);
exit:
	return curr_page;
}

int is_i2c_read_otp_4ha(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	int ret = 0;
	int index = 0;
	u16 curr_addr = start_addr;
	u16 curr_page;

	curr_page = is_i2c_select_otp_bank_4ha(client);
	for (index = 0; index < size ; index++) {
		ret = is_sensor_read8(client, curr_addr, &buf[index]); /* OTP read */
		if (unlikely(ret)) {
			err("failed to is_sensor_addr8_read8 (%d)\n", ret);
			goto exit;
		}
		curr_addr++;
		if (curr_addr > S5K4HA_OTP_PAGE_ADDR_H) {
			curr_addr = S5K4HA_OTP_PAGE_ADDR_L;
			curr_page++;
			is_sensor_write8(client, S5K4HA_OTP_PAGE_SELECT_ADDR, curr_page);
		}
	}

exit:
	return ret;
}

int is_sec_readcal_otprom_4ha(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	int i2c_retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 read_command_complete_check = 0;

	struct is_core *core = dev_get_drvdata(dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_sec_get_cal_buf(position, &buf);

	client = specific->rom_client[position];

	if (!client) {
		err("cis i2c client is NULL\n");
		return -EINVAL;
	}

	is_i2c_config(client, true);
	msleep(10);

	/* 0. Sensor Initial Settings (Global) */
i2c_write_retry_global:
	if (specific->running_camera[position] == false) {
		ret = is_sec_set_registers(client, sensor_otp_4ha_global, sensor_otp_4ha_global_size);
		if (unlikely(ret)) {
			err("failed to is_sec_set_registers (%d)\n", ret);
			if (i2c_retry >= 0) {
				i2c_retry--;
				msleep(50);
				goto i2c_write_retry_global;
			}
			ret = -EINVAL;
			goto exit;
		}
	}

	/* 1. Stream On */
	is_sensor_write8(client, 0x0100, 0x01);
	msleep(50);

	is_sensor_write8(client, S5K4HA_STANDBY_ADDR, 0x00); /* standby on */
	msleep(10); /* sleep 10msec */

	is_sensor_write8(client, S5K4HA_OTP_R_W_MODE_ADDR, 0x01); //write "read" command

	retry = IS_CAL_RETRY_CNT;
	/* confirm write complete*/
write_complete_retry:
	msleep(1);
	is_sensor_read8(client, S5K4HA_OTP_CHECK_ADDR, &read_command_complete_check);
	if (read_command_complete_check!=1) {
		if (retry>=0) {
			retry--;
			goto write_complete_retry;
		}
		goto exit;
	}
	retry = IS_CAL_RETRY_CNT;

crc_retry:
	/* 3. Read OTP Cal Data */
	info("I2C read cal data\n");
	is_i2c_read_otp_4ha(client, buf, S5K4HA_OTP_START_ADDR, S5K4HA_OTP_USED_CAL_SIZE);

	/* 4. Parse Cal */
	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;
	}
exit:
	/* 5. streaming mode change */
	is_sensor_write8(client, S5K4HA_OTP_R_W_MODE_ADDR, 0x04); /* clear error bit */
	msleep(1); /* sleep 1msec */
	is_sensor_write8(client, S5K4HA_OTP_R_W_MODE_ADDR, 0x00); /* initial command */
	info("%s X\n", __func__);
	return ret;
}
#endif /* SENSOR_OTP_4HA */

#if defined(SENSOR_OTP_GC5035)
int is_i2c_read_otp_gc5035(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	u16 curr_addr = start_addr;
	u8 start_addr_h = 0;
	u8 start_addr_l = 0;
	u8 busy_flag = 0;
	int retry = 8;
	int ret = 0;
	int index;

	for (index = 0; index < size; index++)
	{
		start_addr_h = ((curr_addr>>8) & 0x1F);
		start_addr_l = (curr_addr & 0xFF);
		is_sensor_write8(client, GC5035_OTP_PAGE_ADDR, GC5035_OTP_PAGE);
		is_sensor_addr8_write8(client, GC5035_OTP_ACCESS_ADDR_HIGH, start_addr_h);
		is_sensor_addr8_write8(client, GC5035_OTP_ACCESS_ADDR_LOW, start_addr_l);
		is_sensor_addr8_write8(client, GC5035_OTP_MODE_ADDR, 0x20);
		is_sensor_addr8_read8(client, GC5035_OTP_BUSY_ADDR, &busy_flag); 
		while((busy_flag&0x2)>0 && retry > 0 ) {
			is_sensor_addr8_read8(client, GC5035_OTP_BUSY_ADDR, &busy_flag); 
			retry--;
			msleep(1);
		}

		if ((busy_flag & 0x1))
		{
			err("Sensor OTP_check_flag failed\n");
			goto exit;
		}

		ret = is_sensor_addr8_read8(client, GC5035_OTP_READ_ADDR, &buf[index]);
		if (unlikely(ret))
		{
			err("failed to is_sensor_addr8_read8 (%d)\n", ret);
			goto exit;
		}
		curr_addr += 8;
	}

exit:
	return ret;
}


int is_sec_readcal_otprom_gc5035(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 otp_bank = 0;
	u16 start_addr = 0;
	u8 busy_flag = 0;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

	if (specific->running_camera[position] == false) {
		ret = is_sec_set_registers(client, sensor_global_gc5035, sensor_global_gc5035_size);
		if (unlikely(ret)) {
			err("failed to is_sec_set_registers (%d)\n", ret);
			ret = -EINVAL;
		}
	}

	is_sec_set_registers(client, sensor_mode_read_initial_setting_gc5035, sensor_mode_read_initial_setting_gc5035_size);

	/* Read OTP page */
	is_sensor_addr8_write8(client, GC5035_OTP_PAGE_ADDR, GC5035_OTP_PAGE);
	is_sensor_addr8_write8(client, GC5035_OTP_ACCESS_ADDR_HIGH, ((GC5035_BANK_SELECT_ADDR>>8) & 0x1F));
	is_sensor_addr8_write8(client, GC5035_OTP_ACCESS_ADDR_LOW, (GC5035_BANK_SELECT_ADDR & 0xFF));
	is_sensor_addr8_write8(client, GC5035_OTP_MODE_ADDR, 0x20);

	is_sensor_addr8_read8(client, GC5035_OTP_BUSY_ADDR, &busy_flag); 
	while((busy_flag&0x2)>0 && retry > 0 ) {
		is_sensor_addr8_read8(client, GC5035_OTP_BUSY_ADDR, &busy_flag); 
		retry--;
		msleep(1);
	}

	if ((busy_flag & 0x1))
	{
		err("Sensor OTP_check_flag failed\n");
		goto exit;
	}

	is_sensor_addr8_read8(client, GC5035_OTP_READ_ADDR, &otp_bank);
	info("%s: otp_bank = %d\n", __func__, otp_bank);

	/* select start address */
	switch(otp_bank) {
	case 0x01 :
		start_addr = GC5035_OTP_START_ADDR_BANK1;
		break;
	case 0x03 :
		start_addr = GC5035_OTP_START_ADDR_BANK2;
		break;
	default :
		start_addr = GC5035_OTP_START_ADDR_BANK1;
		break;
	}

	info("%s: otp_start_addr = %x\n", __func__, start_addr);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

crc_retry:
	/* read cal data */
	info("I2C read cal data\n");
	is_i2c_read_otp_gc5035(client, buf, start_addr, GC5035_OTP_USED_CAL_SIZE);

	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;
	}

exit:
	info("%s X\n", __func__);
	return ret;
}
#endif //SENSOR_OTP_GC5035

#if defined(SENSOR_OTP_HI556)
#if defined(SENSOR_OTP_HI556_STANDARD_CAL)
int is_i2c_read_otp_hi556(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	u16 curr_addr = start_addr;
	u8 start_addr_h = 0;
	u8 start_addr_l = 0;
	int ret = 0;
	int index;

	for (index = 0; index < size; index++) {
		start_addr_h = ((curr_addr>>8) & 0xFF);
		start_addr_l = (curr_addr & 0xFF);
		is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_HIGH, start_addr_h);
		is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_LOW, start_addr_l);
		is_sensor_write8(client, HI556_OTP_MODE_ADDR, 0x01);

		ret = is_sensor_read8(client, HI556_OTP_READ_ADDR, &buf[index]);
		if (unlikely(ret)) {
			err("failed to is_sensor_read8 (%d)\n", ret);
			goto exit;
		}
		curr_addr ++;
	}

exit:
	return ret;
}

int is_sec_readcal_otprom_hi556(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 otp_bank = 0;
	u16 start_addr = 0;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

	/* OTP read only setting */
	is_sec_set_registers(client, otp_read_initial_setting_hi556, otp_read_initial_setting_hi556_size);

	/* OTP mode on */
	msleep(10);
	is_sensor_write8(client, 0x0A02, 0x01);
	is_sensor_write8(client, 0x0114, 0x00);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x00);
	msleep(10);
	is_sec_set_registers(client, otp_mode_on_setting_hi556, otp_mode_on_setting_hi556_size);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x01);
	msleep(10);

	/* Select the bank */

	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_HIGH, ((HI556_BANK_SELECT_ADDR >> 8) & 0xFF));
	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_LOW, (HI556_BANK_SELECT_ADDR & 0xFF));
	is_sensor_write8(client, HI556_OTP_MODE_ADDR, 0x01);
	ret = is_sensor_read8(client, HI556_OTP_READ_ADDR, &otp_bank);
	if (unlikely(ret)) {
		err("failed to read otp_bank data from bank select address (%d)\n", ret);
		ret = -EINVAL;
	}

	info("%s: otp_bank = %d\n", __func__, otp_bank);

	/* select start address */
	switch (otp_bank) {
	case 0x01:
		start_addr = HI556_OTP_START_ADDR_BANK1;
		break;
	case 0x03:
		start_addr = HI556_OTP_START_ADDR_BANK2;
		break;
	case 0x07:
		start_addr = HI556_OTP_START_ADDR_BANK3;
		break;
	case 0x0F:
		start_addr = HI556_OTP_START_ADDR_BANK4;
		break;
	case 0x01F:
		start_addr = HI556_OTP_START_ADDR_BANK5;
		break;
	default:
		start_addr = HI556_OTP_START_ADDR_BANK1;
		break;
	}

	info("%s: otp_start_addr = %x\n", __func__, start_addr);

crc_retry:
	info("%s I2C read cal data", __func__);
	is_i2c_read_otp_hi556(client, buf, start_addr, HI556_OTP_USED_CAL_SIZE);

	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
	}

	/* OTP mode off*/
	is_sensor_write8(client, 0x0114, 0x00);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x00);
	msleep(10);
	is_sec_set_registers(client, otp_mode_off_setting_hi556, otp_mode_off_setting_hi556_size);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x01);
	msleep(10);

exit:
	info("%s X\n", __func__);
	return ret;
}
#else
int is_i2c_read_otp_hi556(struct i2c_client *client, char *buf, int group, const struct is_vender_rom_addr *rom_addr)
{
	int ret = 0;
	int segment, offset;
	int addr_buf = 0;
	struct rom_ap2ap_standard_cal_data *ap2ap_standard_cal_data;

	if (rom_addr->extend_cal_addr) {
		ap2ap_standard_cal_data = (struct rom_ap2ap_standard_cal_data *)is_sec_search_rom_extend_data(
			rom_addr->extend_cal_addr, EXTEND_AP2AP_STANDARD_CAL);
		if (ap2ap_standard_cal_data == NULL) {
			err("ap2ap_standard_cal_data is NULL");
			ret = -EINVAL;
			goto exit;
		}
	}

	/* read from the rom values to the buffer */
	for (segment = 0; segment < ap2ap_standard_cal_data->rom_num_of_segments; segment++) {
		is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_HIGH, ((ap2ap_standard_cal_data->rom_bank_start_addr[segment][group]) >> 8));
		is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_LOW, ap2ap_standard_cal_data->rom_bank_start_addr[segment][group] & 0xFF);
		is_sensor_write8(client, HI556_OTP_MODE_ADDR, 0x01);

		for (offset = 0; offset < ap2ap_standard_cal_data->rom_seg_size[segment]; offset++) {
			ret = is_sensor_read8(client, HI556_OTP_READ_ADDR, &buf[addr_buf++]);
			if (unlikely(ret)) {
				err("failed to is_sensor_read8 (%d)\n", ret);
				goto exit;
			}
		}
	}

	/* read the total checksum */
	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_HIGH, ((ap2ap_standard_cal_data->rom_total_checksum_addr[group]) >> 8));
	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_LOW, ap2ap_standard_cal_data->rom_total_checksum_addr[group] & 0xFF);
	is_sensor_write8(client, HI556_OTP_MODE_ADDR, 0x01);
	ret = is_sensor_read8(client, HI556_OTP_READ_ADDR, &buf[addr_buf++]);
	if (unlikely(ret)) {
		err("failed to is_sensor_read8 (%d)\n", ret);
		goto exit;
	}
exit:
	return ret;
}

int is_sec_readcal_otprom_hi556(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 group_value = 0;
	int group = 0;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

	/* OTP read only setting */
	is_sec_set_registers(client, otp_read_initial_setting_hi556, otp_read_initial_setting_hi556_size);

	/* OTP mode on */
	msleep(10);
	is_sensor_write8(client, 0x0A02, 0x01);
	is_sensor_write8(client, 0x0114, 0x00);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x00);
	msleep(10);
	is_sec_set_registers(client, otp_mode_on_setting_hi556, otp_mode_on_setting_hi556_size);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x01);
	msleep(10);

	/* Refer to JDM Cal data to select group and the header addresses */
	/* Read OTP data */

	/* read the group */
	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_HIGH, (0x0401 >> 8));
	is_sensor_write8(client, HI556_OTP_ACCESS_ADDR_LOW, 0x0401 & 0xFF);
	is_sensor_write8(client, HI556_OTP_MODE_ADDR, 0x01);
	ret = is_sensor_read8(client, HI556_OTP_READ_ADDR, &group_value);
	if (unlikely(ret)) {
		err("failed to read group_value data from 0x0401 (%d)\n", ret);
		ret = -EINVAL;
	}
	switch (group_value) {
	case 0x01:
		group = 0;
		break;
	
	case 0x13:
		group = 1;
		break;

	case 0x37:
		group = 2;
		break;

	default:
		group = 0;
		err("%s failed to choose jdm group for group value %X", __func__, group_value);
	}

	info("%s Group value: %X, Group selected : %d\n", __func__, group_value, group);

crc_retry:
	info("%s I2C read cal data", __func__);
	is_i2c_read_otp_hi556(client, buf, group, rom_addr);

	ret = is_sec_readcal_jdm_checksum_and_dump(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
	}

	/* OTP mode off*/
	is_sensor_write8(client, 0x0114, 0x00);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x00);
	msleep(10);
	is_sec_set_registers(client, otp_mode_off_setting_hi556, otp_mode_off_setting_hi556_size);
	msleep(10);
	is_sensor_write8(client, 0x0A00, 0x01);
	msleep(10);

exit:
	info("%s X\n", __func__);
	return ret;
}
#endif //SENSOR_OTP_HI556_STANDARD_CAL
#endif //SENSOR_OTP_HI556

#if defined(SENSOR_OTP_SC501)
/*
 * Summarize the exceptions
 * 1. Find which bank is used by reading first group address
 * 2. Pages are divided in 2 parts. When it switchs to the other page, page setting should be required
 * 3. Bank1's LSC size has 12 bytes more than bank2's because there is unused area in the middle of LSC area
 */
int is_setup_area1_sc501(struct i2c_client *client) {
	int ret = 0;
	u8 status = SC501_STATUS_LOAD_ONGOING;
	int retries = SC501_RETRIES_COUNT;

	info("Otprom valid area is changed to 0x0x8000 ~ 0x87FF");

	/* Write threshold value */
	ret = is_sensor_write8(client, 0x36B0, 0x4C);
	ret |= is_sensor_write8(client, 0x36B1, 0xD8);
	ret |= is_sensor_write8(client, 0x36B2, 0x01);
	if (unlikely(ret)) {
		err("Failed to write threshold value (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	/* Write read start address */
	is_sensor_write8(client, 0x4408, 0x00);
	is_sensor_write8(client, 0x4409, 0x00);

	/* Write read end address */
	is_sensor_write8(client, 0x440A, 0x07);
	is_sensor_write8(client, 0x440B, 0xFF);

	/* ld_setting */
	is_sensor_write8(client, 0x4401, 0x1F);

	/* Manual load enable */
	is_sensor_write8(client, 0x4400, 0x11);

	/* Read completed check */
	status = SC501_STATUS_LOAD_ONGOING;
	while (status) {
		ret = is_sensor_read8(client, 0x4420, &status);
		if (unlikely(ret)) {
			err("Failed to read completed check (%d)\n", ret);
			ret = -EINVAL;
			break;
		}

		status &= 0x1;
		msleep(1);
		if (retries-- < 0) {
			err("Failed to update otp load status");
			ret = -EINVAL;
			break;
		}
	}
exit:
	return ret;
}

int is_setup_area2_sc501(struct i2c_client *client) {
	int ret = 0;
	u8 status = SC501_STATUS_LOAD_ONGOING;
	int retries = SC501_RETRIES_COUNT;

	info("Otprom valid area is changed to 0x8800 ~ 0x8FFF");

	/* Write threshold value */
	ret = is_sensor_write8(client, 0x36B0, 0x4C);
	ret |= is_sensor_write8(client, 0x36B1, 0xD8);
	ret |= is_sensor_write8(client, 0x36B2, 0x01);
	if (unlikely(ret)) {
		err("Failed to write threshold value (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	/* Write read start address */
	is_sensor_write8(client, 0x4408, 0x08);
	is_sensor_write8(client, 0x4409, 0x00);

	/* Write read end address */
	is_sensor_write8(client, 0x440A, 0x0F);
	is_sensor_write8(client, 0x440B, 0xFF);

	/* ld_setting */
	is_sensor_write8(client, 0x4401, 0x1E);

	/* Manual load enable */
	is_sensor_write8(client, 0x4400, 0x11);

	/* Read completed check */
	status = SC501_STATUS_LOAD_ONGOING;
	while (status) {
		ret = is_sensor_read8(client, 0x4420, &status);
		if (unlikely(ret)) {
			err("Failed to read completed check (%d)\n", ret);
			ret = -EINVAL;
			break;
		}

		status &= 0x1;
		msleep(1);
		if (retries-- < 0) {
			err("Failed to update otp load status");
			ret = -EINVAL;
			break;
		}
	}
exit:
	return ret;
}

int is_i2c_read_otp_sc501(struct i2c_client *client, int position, char *buf, const struct is_vender_rom_addr *rom_addr)
{
	int ret = 0;
	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct rom_ap2ap_standard_cal_data *ap2ap_standard_cal_data;
	int segment, bank, i;
	u16 addr_buf = 0x0, addr_otp = 0x0;
	u8 value = 0;

	if (rom_addr->extend_cal_addr) {
		ap2ap_standard_cal_data = (struct rom_ap2ap_standard_cal_data *)is_sec_search_rom_extend_data(
			rom_addr->extend_cal_addr, EXTEND_AP2AP_STANDARD_CAL);
		if (ap2ap_standard_cal_data == NULL) {
			err("ap2ap_standard_cal_data is NULL");
			ret = -EINVAL;
			goto exit;
		}
	}

	/* Find the bank */
	addr_buf = 0x0;
	segment = 0;
	for (bank = SC501_FRONT_GROUP1; bank < ap2ap_standard_cal_data->rom_num_of_banks; bank++) {
		if (ap2ap_standard_cal_data->rom_bank_start_addr[segment][bank] < SC501_AREA2_START_ADDR) {
			if (is_setup_area1_sc501(client) < 0) {
				err("Failed to setup area1 (%d)\n", ret);
				ret = -EINVAL;
				goto exit;
			}
		} else {
			if (is_setup_area2_sc501(client) < 0) {
				err("Failed to setup area1 (%d)\n", ret);
				ret = -EINVAL;
				goto exit;
			}
		}

		ret = is_sensor_read8(client, ap2ap_standard_cal_data->rom_bank_start_addr[segment][bank], &value);
		if (unlikely(ret)) {
			err("Failed to is_sensor_read8 (%d)\n", ret);
			ret = -EINVAL;
			goto exit;
		}

		if (value == SC501_FLAG_BANK_VALID) {
			specific->rom_bank[position] = bank;
			break;
		}
	}

	for (segment = SC501_FRONT_SEGMENT_MODULE; segment < ap2ap_standard_cal_data->rom_num_of_segments; segment++) {
		addr_otp = ap2ap_standard_cal_data->rom_bank_start_addr[segment][bank];

		if (bank == SC501_FRONT_GROUP2 && segment == SC501_FRONT_SEGMENT_LSC) {
			ap2ap_standard_cal_data->rom_seg_size[segment] = SC501_FRONT_BANK2_LSC_LEN;
			ap2ap_standard_cal_data->rom_seg_checksum_len[segment] = SC501_FRONT_BANK2_LSC_LEN - 1;
		}
		info("Bank : %d, Segment : %d (addr_otp : 0x%04X, addr_buf : 0x%04X, seg_size : %d)",
				bank, segment, addr_otp, addr_buf, ap2ap_standard_cal_data->rom_seg_size[segment]); 

		for (i = 0; i < ap2ap_standard_cal_data->rom_seg_size[segment]; i++) {
			if (addr_otp == SC501_AREA2_START_ADDR) {
				if (is_setup_area2_sc501(client) < 0) {
					err("Failed to setup area1 (%d)\n", ret);
					ret = -EINVAL;
					goto exit;
				}
			}
			ret = is_sensor_read8(client, addr_otp, &buf[addr_buf++]);
			if (unlikely(ret)) {
				err("Failed to is_sensor_read8 (%d)\n", ret);
				ret = -EINVAL;
				goto exit;
			}
			addr_otp++;
		}
	}

	addr_otp = ap2ap_standard_cal_data->rom_total_checksum_addr[bank];

	ret = is_sensor_read8(client, addr_otp, &buf[addr_buf++]);
	if (unlikely(ret)) {
		err("Failed to is_sensor_read8 (%d)\n", ret);
		goto exit;
	}

exit:
	return ret;
}

int is_sec_readcal_otprom_sc501(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;

	char *buf = NULL;
	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

crc_retry:
	info("%s I2C read cal data", __func__);
	is_i2c_read_otp_sc501(client, position, buf, rom_addr);

	ret = is_sec_readcal_jdm_checksum_and_dump(buf, position);
	if (unlikely(ret)) {
		if (retry-- > 0)
			goto crc_retry;
	}

	/* OTP read finishing */ 
	is_sensor_write8(client, 0x4424, 0x01);

	is_sensor_write8(client, 0x4408, 0x00);
	is_sensor_write8(client, 0x4409, 0x00);
	is_sensor_write8(client, 0x440A, 0x07);
	is_sensor_write8(client, 0x440B, 0xFF);

	is_sensor_write8(client, 0x4401, 0x1F);

exit:
	info("%s X\n", __func__);
	return ret;
}
#endif //SENSOR_OTP_SC501

#if defined(SENSOR_OTP_GC02M1)
int is_i2c_read_otp_gc02m1(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	u16 curr_addr = start_addr;
	int ret = 0;
	int index;

	for (index = 0; index < size; index++) {
		is_sensor_addr8_write8(client, GC02M1_OTP_PAGE_ADDR, GC02M1_OTP_PAGE);
		is_sensor_addr8_write8(client, GC02M1_OTP_ACCESS_ADDR, curr_addr);
		is_sensor_addr8_write8(client, GC02M1_OTP_MODE_ADDR, 0x34); /* Read Pulse */
		
		ret = is_sensor_addr8_read8(client, GC02M1_OTP_READ_ADDR, &buf[index]);
		if (unlikely(ret)) {
			err("failed to is_sensor_addr8_read8 (%d)\n", ret);
			break;
		}
		curr_addr += 8;
	}

	return ret;
}

int is_sec_readcal_otprom_gc02m1(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];
	if (!client) {
		err("%s: cis i2c client is NULL\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

	if (specific->running_camera[position] == false) {
		ret = is_sec_set_registers(client, sensor_global_gc02m1, sensor_global_gc02m1_size);
		if (unlikely(ret)) {
			err("failed to is_sec_set_registers (%d)\n", ret);
			ret = -EINVAL;
		}
	}

	ret = is_sec_set_registers(client, sensor_mode_read_initial_setting_gc02m1, sensor_mode_read_initial_setting_gc02m1_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
	}
	
crc_retry:
	/* read cal data */
	info("I2C read cal data\n");
	is_i2c_read_otp_gc02m1(client, buf, GC02M1_OTP_START_ADDR, GC02M1_OTP_USED_CAL_SIZE);

	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;
	}

exit:
	info("%s X\n", __func__);
	return ret;
}
#endif //SENSOR_OTP_GC02M1

#if defined(SENSOR_OTP_GC08A3)
int is_i2c_read_otp_gc08a3(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	u16 curr_addr = start_addr;
	u8 start_addr_h = 0;
	u8 start_addr_l = 0;
	int ret = 0;
	int index;

	for (index = 0; index < size; index++) {
		start_addr_h = ((curr_addr>>8) & 0xFF);
		start_addr_l = (curr_addr & 0xFF);
		is_sensor_write8(client, GC08A3_OTP_ACCESS_ADDR_HIGH, start_addr_h);
		is_sensor_write8(client, GC08A3_OTP_ACCESS_ADDR_LOW, start_addr_l);
		is_sensor_write8(client, GC08A3_OTP_MODE_SEL_ADDR, GC08A3_READ_MODE);

		ret = is_sensor_read8(client, GC08A3_OTP_READ_ADDR, &buf[index]);
		if (unlikely(ret)) {
			err("failed to is_sensor_addr16_read16 (%d)\n", ret);
			goto exit;
		}
		curr_addr += 8;
	}

exit:
	return ret;
}


int is_sec_readcal_otprom_gc08a3(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 otp_bank = 0;
	u16 start_addr = 0;

	struct is_core *core = dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	is_sec_get_cal_buf(position, &buf);
	client = specific->rom_client[position];

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_i2c_config(client, true);

	if (specific->running_camera[position] == false) {
		ret = is_sec_set_registers(client, sensor_otp_gc08a3_global, sensor_otp_gc08a3_global_size);
		if (unlikely(ret)) {
			err("failed to is_sec_set_registers (%d)\n", ret);
			ret = -EINVAL;
		}
	}

	ret = is_sec_set_registers(client, sensor_otp_gc08a3_initial, sensor_otp_gc08a3_initial_size);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
	}

	msleep(10); /* 10ms delay */

	/* Read OTP page */
	is_sensor_write8(client, GC08A3_OTP_ACCESS_ADDR_HIGH, ((GC08A3_BANK_SELECT_ADDR>>8) & 0xFF));
	is_sensor_write8(client, GC08A3_OTP_ACCESS_ADDR_LOW, (GC08A3_BANK_SELECT_ADDR & 0xFF));

	is_sensor_write8(client, GC08A3_OTP_MODE_SEL_ADDR, GC08A3_READ_MODE);

	is_sensor_read8(client, GC08A3_OTP_READ_ADDR, &otp_bank);
	info("%s: otp_bank = %d\n", __func__, otp_bank);

	/* select start address */
	switch (otp_bank) {
	case 0x01:
		start_addr = GC08A3_OTP_START_ADDR_BANK1;
		break;
	case 0x03:
		start_addr = GC08A3_OTP_START_ADDR_BANK2;
		break;
	case 0x07:
		start_addr = GC08A3_OTP_START_ADDR_BANK3;
		break;
	case 0x0F:
		start_addr = GC08A3_OTP_START_ADDR_BANK4;
		break;
	case 0x01F:
		start_addr = GC08A3_OTP_START_ADDR_BANK5;
		break;
	default:
		start_addr = GC08A3_OTP_START_ADDR_BANK1;
		break;
	}

	info("%s: otp_start_addr = %x\n", __func__, start_addr);
	if (unlikely(ret)) {
		err("failed to is_sec_set_registers (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

crc_retry:
	/* read cal data */
	info("I2C read cal data\n");
	is_i2c_read_otp_gc08a3(client, buf, start_addr, GC08A3_OTP_USED_CAL_SIZE);

	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
		crc32_check_list[position][CRC32_CHECK_FACTORY] = false;
	}

exit:
	info("%s X\n", __func__);
	return ret;
}
#endif //SENSOR_OTP_GC08A3

#if defined(SENSOR_OTP_HI1336B)
int is_i2c_read_otp_hi1336(struct i2c_client *client, char *buf, u16 start_addr, size_t size)
{
	u16 curr_addr = start_addr;
	u8 start_addr_h = 0;
	u8 start_addr_l = 0;
	int ret = 0;
	int index;

	for (index = 0; index < size; index++) {
		start_addr_h = ((curr_addr>>8) & 0xFF);
		start_addr_l = (curr_addr & 0xFF);
		is_sensor_write8(client, HI1336B_OTP_ACCESS_ADDR_HIGH, start_addr_h);
		is_sensor_write8(client, HI1336B_OTP_ACCESS_ADDR_LOW, start_addr_l);
		is_sensor_write8(client, HI1336B_OTP_MODE_ADDR, 0x01);

		ret = is_sensor_read8(client, HI1336B_OTP_READ_ADDR, &buf[index]);
		if (unlikely(ret)) {
			err("failed to is_sensor_read8 (%d)\n", ret);
			goto exit;
		}
		curr_addr ++;
	}

exit:
	return ret;
}

int is_sec_readcal_otprom_hi1336(struct device *dev, int position)
{
	int ret = 0;
	int retry = IS_CAL_RETRY_CNT;
	char *buf = NULL;
	u8 otp_bank = 0;
	u16 start_addr = 0;

	struct is_core *core = dev_get_drvdata(dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct i2c_client *client = NULL;
	const struct is_vender_rom_addr *rom_addr = specific->rom_cal_map_addr[position];

	struct is_device_sensor_peri *sensor_peri = NULL;
	struct v4l2_subdev *subdev_cis = NULL;
	int i;
	u32 i2c_channel;
	struct is_module_enum *module = NULL;

	for (i = 0; i < IS_SENSOR_COUNT; i++) {
		is_search_sensor_module_with_position(&core->sensor[i], position, &module);
		if (module)
			break;
	}

	sensor_peri = (struct is_device_sensor_peri *)module->private_data;
	subdev_cis = sensor_peri->subdev_cis;

	if (!rom_addr) {
		err("%s: otp_%d There is no cal map\n", __func__, position);
		ret = -EINVAL;
		goto exit;
	}

	info("%s E\n", __func__);

	is_sec_get_cal_buf(position, &buf);

	client = specific->rom_client[position];

	if (!client) {
		err("cis i2c client is NULL\n");
		return -EINVAL;
	}

	is_i2c_config(client, true);
	msleep(10);

	/* Sensor Initial Settings (Global) */
i2c_write_retry_global:
	if (specific->running_camera[position] == false) {
		i2c_channel = module->pdata->sensor_i2c_ch;
		if (i2c_channel < SENSOR_CONTROL_I2C_MAX) {
			sensor_peri->cis.i2c_lock = &core->i2c_lock[i2c_channel];
		} else {
			warn("%s: wrong cis i2c_channel(%d)", __func__, i2c_channel);
			ret = -EINVAL;
			goto exit;
		}
		/* sensor global settings */
		ret = CALL_CISOPS(&sensor_peri->cis, cis_set_global_setting, subdev_cis);

		if (unlikely(ret)) {
			err("failed to apply global settings (%d)\n", ret);
			if (retry >= 0) {
				retry--;
				msleep(50);
				goto i2c_write_retry_global;
			}
			ret = -EINVAL;
			goto exit;
		}
	}

	ret = CALL_CISOPS(&sensor_peri->cis, cis_mode_change, subdev_cis, 1);
	if (unlikely(ret)) {
		err("failed to apply cis_mode_change (%d)\n", ret);
		ret = -EINVAL;
		goto exit;
	}

	retry = IS_CAL_RETRY_CNT;

crc_retry:
	/* stream on */
	ret = is_sensor_write8(client, 0x0808, 0x01);
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0808, 0x01);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0B02, 0x01); /* fast standby on */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0B02, 0x01);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0809, 0x00); /* stream off */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0809, 0x00);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0B00, 0x00); /* stream off */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0B00, 0x00);
		return ret;
	}
	msleep(10); /* sleep 10msec */
	ret = is_sensor_write8(client, 0x0260, 0x10); /* OTP test mode enable */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0260, 0x10);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0809, 0x01); /* stream on */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0809, 0x01);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0b00, 0x01); /* stream on */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0b00, 0x01);
		return ret;
	}

	msleep(1); /* sleep 1msec */

	/* read otp bank */
	is_sensor_write8(client, HI1336B_OTP_ACCESS_ADDR_HIGH, ((HI1336B_BANK_SELECT_ADDR >> 8) & 0xFF));
	is_sensor_write8(client, HI1336B_OTP_ACCESS_ADDR_LOW, (HI1336B_BANK_SELECT_ADDR & 0xFF));
	is_sensor_write8(client, HI1336B_OTP_MODE_ADDR, 0x01);
	ret = is_sensor_read8(client, HI1336B_OTP_READ_ADDR, &otp_bank);
	if (unlikely(ret)) {
		err("failed to read otp_bank data from bank select address (%d)\n", ret);
		ret = -EINVAL;
	}

	info("%s: otp_bank = %d\n", __func__, otp_bank);

	/* select start address */
	switch (otp_bank) {
	case 0x01:
		start_addr = HI1336B_OTP_START_ADDR_BANK1;
		break;
	case 0x03:
		start_addr = HI1336B_OTP_START_ADDR_BANK2;
		break;
	case 0x07:
		start_addr = HI1336B_OTP_START_ADDR_BANK3;
		break;
	case 0x0F:
		start_addr = HI1336B_OTP_START_ADDR_BANK4;
		break;
	case 0x01F:
		start_addr = HI1336B_OTP_START_ADDR_BANK5;
		break;
	default:
		start_addr = HI1336B_OTP_START_ADDR_BANK1;
		break;
	}

	info("%s: otp_start_addr = %x\n", __func__, start_addr);

	info("%s I2C read cal data", __func__);
	is_i2c_read_otp_hi1336(client, buf, start_addr, HI1336B_OTP_USED_CAL_SIZE);

	ret = is_sec_readcal_otprom_buffer(buf, position);
	if (unlikely(ret)) {
		if (retry >= 0) {
			retry--;
			goto crc_retry;
		}
	}

exit:
	/* streaming mode change */
	ret = is_sensor_write8(client, 0x0809, 0x00); /* stream off */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0809, 0x00);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0b00, 0x00); /* stream off */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0b00, 0x00);
		return ret;
	}
	msleep(10); /* sleep 10msec */
	ret = is_sensor_write8(client, 0x0260, 0x00); /* OTP mode display */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0260, 0x00);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0809, 0x01); /* stream on */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0809, 0x01);
		return ret;
	}
	ret = is_sensor_write8(client, 0x0b00, 0x01); /* stream on */
	if (ret < 0) {
		err("failed to is_sensor_write8, ret(%d), addr(%#x), data(%#x)\n",
			ret, 0x0b00, 0x01);
		return ret;
	}
	msleep(1); /* sleep 1msec */
	return ret;
}
#endif //SENSOR_OTP_HI1336B

int is_sec_select_otprom(struct device *dev, int position, int sensor_id)
{
	int ret = -1;
	switch (sensor_id) {
#if defined(SENSOR_OTP_5E9)
	case SENSOR_NAME_S5K5E9:
		ret = is_sec_readcal_otprom_5e9(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_SR846)
	case SENSOR_NAME_SR846:
		ret = is_sec_readcal_otprom_sr846(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_GC5035)
	case SENSOR_NAME_GC5035:
		ret = is_sec_readcal_otprom_gc5035(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_HI556)
	case SENSOR_NAME_HI556:
		ret = is_sec_readcal_otprom_hi556(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_SC501)
	case SENSOR_NAME_SC501:
		ret = is_sec_readcal_otprom_sc501(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_GC02M1)
	case SENSOR_NAME_GC02M1:
		ret = is_sec_readcal_otprom_gc02m1(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_4HA)
	case SENSOR_NAME_S5K4HA:
		ret = is_sec_readcal_otprom_4ha(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_GC08A3)
	case SENSOR_NAME_GC08A3:
		ret = is_sec_readcal_otprom_gc08a3(dev, position);
		break;
#endif
#if defined(SENSOR_OTP_HI1336B)
	case SENSOR_NAME_HI1336:
		ret = is_sec_readcal_otprom_hi1336(dev, position);
		break;
#endif
	default:
		err("%s OTP not supported for sensor_id=%d at position=%d", sensor_id, position);
	}
	return ret;
}

int is_sec_readcal_otprom(struct device *dev, int position)
{
	int ret = 0;
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	u32 sensor_id = specific->sensor_id[position];
#ifdef USE_DUALIZED_OTPROM_SENSOR
	int dualized_sensor_id = specific->dualized_sensor_id[position];
	struct i2c_client* reuse_rom_client = specific->rom_client[position];
	const struct is_vender_rom_addr* reuse_rom_cal_map_addr = specific->rom_cal_map_addr[position];
#endif

	ret = is_sec_select_otprom(dev, position, sensor_id);

#ifdef USE_DUALIZED_OTPROM_SENSOR
	if (ret != 0 && dualized_sensor_id > 0) {
		warn("%s OTP calread failed for first sensor", __func__);
		/* power off previous sensor rom */
		is_sec_rom_power_off(core, position);

		/* change to dualized sensor */
		specific->sensor_id[position] = dualized_sensor_id;
		specific->rom_client[position] = specific->dualized_rom_client[position];
		specific->rom_cal_map_addr[position] = specific->dualized_rom_cal_map_addr[position];
		/* power on current sensor rom */
		is_sec_rom_power_on(core, position); 

		ret = is_sec_select_otprom(dev, position, dualized_sensor_id);
		if (ret == 0) {
			info("%s Using dualized sensor", __func__);
		} else {
			err("%s OTP calread fails for both dualized sensors", __func__);
			is_sec_rom_power_off(core, position);

			/* change specific information to first sensor */
			specific->sensor_id[position] = sensor_id;
			specific->rom_client[position] = reuse_rom_client;
			specific->rom_cal_map_addr[position] = reuse_rom_cal_map_addr;
			is_sec_rom_power_on(core, position);
		}
	}
#endif

	return ret;
}

int is_sec_fw_find_rear(struct is_core *core, int position)
{
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;
	u32 sensor_id = specific->sensor_id[position];

	is_sec_get_sysfs_finfo_by_position(position, &finfo);

	if (position == SENSOR_POSITION_REAR) {
		snprintf(finfo->load_fw_name, sizeof(IS_DDK), "%s", IS_DDK);
#if defined(USE_RTA_BINARY)
		snprintf(finfo->load_rta_fw_name, sizeof(IS_RTA), "%s", IS_RTA);
#endif
	}

	/* default firmware and setfile */
	if (sensor_id == SENSOR_NAME_IMX576) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX576_SETF), "%s", IS_IMX576_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K4HA) {
		snprintf(finfo->load_setfile_name, sizeof(IS_4HA_SETF), "%s", IS_4HA_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K5E9) {
		snprintf(finfo->load_setfile_name, sizeof(IS_5E9_SETF), "%s", IS_5E9_SETF);
	} else if (sensor_id == SENSOR_NAME_GC5035) {
#ifdef REAR_MACRO_CAMERA
		if (position == REAR_MACRO_CAMERA)
			snprintf(finfo->load_setfile_name, sizeof(IS_GC5035_MACRO_SETF), "%s", IS_GC5035_MACRO_SETF);
		else
#endif
			snprintf(finfo->load_setfile_name, sizeof(IS_GC5035_SETF), "%s", IS_GC5035_SETF);
	} else if (sensor_id == SENSOR_NAME_IMX582) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX582_SETF), "%s", IS_IMX582_SETF);
	} else if (sensor_id == SENSOR_NAME_HI1336) {
		snprintf(finfo->load_setfile_name, sizeof(IS_HI1336_SETF), "%s", IS_HI1336_SETF);
	} else if (sensor_id == SENSOR_NAME_IMX586) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX586_SETF), "%s", IS_IMX586_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K3L6) {
		snprintf(finfo->load_setfile_name, sizeof(IS_3L6_SETF), "%s", IS_3L6_SETF);
	} else if (sensor_id == SENSOR_NAME_S5KGM2) {
		snprintf(finfo->load_setfile_name, sizeof(IS_GM2_SETF), "%s", IS_GM2_SETF);
	} else if (sensor_id == SENSOR_NAME_S5KGW1) {
		snprintf(finfo->load_setfile_name, sizeof(IS_GW1_SETF), "%s", IS_GW1_SETF);
	} else if (sensor_id == SENSOR_NAME_IMX686) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX686_SETF), "%s", IS_IMX686_SETF);
	} else if (sensor_id == SENSOR_NAME_GC02M1) {
#ifdef REAR_MACRO_CAMERA
		if (position == REAR_MACRO_CAMERA)
			snprintf(finfo->load_setfile_name, sizeof(IS_GC02M1_MACRO_SETF), "%s", IS_GC02M1_MACRO_SETF);
		else
#endif
			snprintf(finfo->load_setfile_name, sizeof(IS_GC02M1_SETF), "%s", IS_GC02M1_SETF);
	} else if (sensor_id == SENSOR_NAME_GC02M2) {
#ifdef REAR_MACRO_CAMERA
		if (position == REAR_MACRO_CAMERA)
			snprintf(finfo->load_setfile_name, sizeof(IS_GC02M2_MACRO_SETF), "%s", IS_GC02M2_MACRO_SETF);
#endif
	} else if (sensor_id == SENSOR_NAME_SC201) {
#ifdef REAR_MACRO_CAMERA
		if (position == REAR_MACRO_CAMERA)
			snprintf(finfo->load_setfile_name, sizeof(IS_SC201_MACRO_SETF), "%s", IS_SC201_MACRO_SETF);
		else
#endif
			snprintf(finfo->load_setfile_name, sizeof(IS_SC201_SETF), "%s", IS_SC201_SETF);
	} else if (sensor_id == SENSOR_NAME_SR846) {
		snprintf(finfo->load_setfile_name, sizeof(IS_SR846_SETF), "%s", IS_SR846_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K2P6) {
		snprintf(finfo->load_setfile_name, sizeof(IS_2P6_SETF), "%s", IS_2P6_SETF);
	} else if (sensor_id == SENSOR_NAME_IMX355) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX355_SETF), "%s", IS_IMX355_SETF);
	} else if (sensor_id == SENSOR_NAME_S5KJN1) {
		snprintf(finfo->load_setfile_name, sizeof(IS_JN1_SETF), "%s", IS_JN1_SETF);
	} else {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX576_SETF), "%s", IS_IMX576_SETF);
	}

	info("Camera[%d]%s: sensor id [%d]. load setfile [%s]\n",
			position, __func__, sensor_id, finfo->load_setfile_name);

	return 0;
}

int is_sec_fw_find_front(struct is_core *core, int position)
{
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;
	u32 sensor_id = specific->sensor_id[SENSOR_POSITION_FRONT];

	is_sec_get_sysfs_finfo_by_position(position, &finfo);

	/* default firmware and setfile */
	if (sensor_id == SENSOR_NAME_S5K2X5) {
		snprintf(finfo->load_setfile_name, sizeof(IS_2X5_SETF), "%s", IS_2X5_SETF);
	} else if (sensor_id == SENSOR_NAME_IMX616) {
		snprintf(finfo->load_setfile_name, sizeof(IS_IMX616_SETF), "%s", IS_IMX616_SETF);
	} else if (sensor_id == SENSOR_NAME_HI1336) {
		snprintf(finfo->load_setfile_name, sizeof(IS_HI1336_SETF), "%s", IS_HI1336_SETF);
	} else if (sensor_id == SENSOR_NAME_HI556) {
		snprintf(finfo->load_setfile_name, sizeof(IS_HI556_SETF), "%s", IS_HI556_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K3P8SP) {
		snprintf(finfo->load_setfile_name, sizeof(IS_3P8SP_SETF), "%s", IS_3P8SP_SETF);
	} else if (sensor_id == SENSOR_NAME_SR846) {
		snprintf(finfo->load_setfile_name, sizeof(IS_SR846_SETF), "%s", IS_SR846_SETF);
	} else if (sensor_id == SENSOR_NAME_S5K4HA) {
		snprintf(finfo->load_setfile_name, sizeof(IS_4HA_SETF), "%s", IS_4HA_SETF);
	} else if (sensor_id == SENSOR_NAME_GC5035) {
		snprintf(finfo->load_setfile_name, sizeof(IS_GC5035_SETF), "%s", IS_GC5035_SETF);
	} else if (sensor_id == SENSOR_NAME_HI556) {
		snprintf(finfo->load_setfile_name, sizeof(IS_HI556_SETF), "%s", IS_HI556_SETF);
	} else if (sensor_id == SENSOR_NAME_GC08A3) {
		snprintf(finfo->load_setfile_name, sizeof(IS_GC08A3_SETF), "%s", IS_GC08A3_SETF);
	} else if (sensor_id == SENSOR_NAME_SC501) {
		snprintf(finfo->load_setfile_name, sizeof(IS_SC501_SETF), "%s", IS_SC501_SETF);
	} else {
		snprintf(finfo->load_setfile_name, sizeof(IS_2X5_SETF), "%s", IS_2X5_SETF);
	}

	info("Camera[%d]%s: sensor id [%d]. load setfile [%s]\n",
			position, __func__, sensor_id, finfo->load_setfile_name);
	return 0;
}

int is_sec_fw_find(struct is_core *core, int position)
{
	int ret = 0;


	if (position == SENSOR_POSITION_FRONT || position == SENSOR_POSITION_FRONT2) {
		ret = is_sec_fw_find_front(core, position);
	}
	else {
		ret = is_sec_fw_find_rear(core, position);
	}

	return ret;
}

int is_sec_run_fw_sel(struct device *dev, int position)
{
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;
	struct is_rom_info *default_finfo = NULL;
	char stat_buf[16], allstat_buf[128];
	int i, ret = 0;
	int rom_position = position;

	if (is_sec_get_sysfs_finfo_by_position(position, &finfo)) {
		err("failed get finfo. plz check position %d", position);
		return -ENXIO;
	}
	is_sec_get_sysfs_finfo_by_position(SENSOR_POSITION_REAR, &default_finfo);

	/* Check reload cal data enabled */
	if (!default_finfo->is_check_cal_reload) {
		if (is_sec_file_exist("/data/vendor/camera/")) {
			is_sec_check_caldata_reload(core);
			default_finfo->is_check_cal_reload = true;
		}
	}

	if (position == SENSOR_POSITION_SECURE) {
		err("Not Support CAL ROM. [%d]", position);
		return 0;
	}

	if (specific->rom_share[position].check_rom_share == true)
		rom_position = specific->rom_share[position].share_position;

	if ((default_finfo->is_caldata_read == false || force_caldata_dump)
#ifndef USES_STANDARD_CAL_RELOAD
		&& sec2lsi_reload
#endif
	) {
		ret = is_sec_run_fw_sel_from_rom(dev, SENSOR_POSITION_REAR, true);
		if (ret < 0) {
			err("failed to select firmware (%d)", ret);
			goto p_err;
		}
	}

	if (specific->rom_data[position].is_rom_read == false || force_caldata_dump) {
#ifdef USES_STANDARD_CAL_RELOAD
		if (sec2lsi_reload) {
			if (position == 0) {
				for (i = SENSOR_POSITION_REAR; i < SENSOR_POSITION_MAX; i++) {
					if (specific->rom_data[i].rom_valid == true || specific->rom_share[i].check_rom_share == true) {
						ret = is_sec_run_fw_sel_from_rom(dev, i, false);
						sec2lsi_conversion_done[i] = false;
						info("sec2lsi reload for rom %d", i);
						if (ret) {
							err("is_sec_run_fw_sel for [%d] is fail(%d)", i, ret);
							goto p_err;
						}
					}
				}
			}
		}
		else
#endif
		{
			ret = is_sec_run_fw_sel_from_rom(dev, position, false);
			if (ret < 0) {
				err("failed to select firmware (%d)", ret);
				goto p_err;
			}
		}
	}

	memset(allstat_buf, 0x0, sizeof(allstat_buf));
	for (i = SENSOR_POSITION_REAR; i < SENSOR_POSITION_MAX; i++) {
		if (specific->rom_data[i].rom_valid == true || specific->rom_share[i].check_rom_share == true) {
			snprintf(stat_buf, sizeof(stat_buf), "[%d - %d] ", i, specific->rom_data[i].is_rom_read);
			strcat(allstat_buf, stat_buf);
		}
	}
	info("%s: Position status %s", __func__, allstat_buf);

p_err:
	if (specific->check_sensor_vendor) {
		if (is_sec_check_rom_ver(core, position)) {
			if (finfo->header_ver[3] != 'L' && finfo->header_ver[3] != 'E') {
				err("Not supported module(position %d). Module ver = %s", position, finfo->header_ver);
				return -EIO;
			}
		}
	}

	return ret;
}

int is_sec_write_phone_firmware(int id)
{
	int ret = 0;

	struct file *fp = NULL;
	mm_segment_t old_fs;
	long fsize, nread;
	u8 *read_buf = NULL;
	u8 *temp_buf = NULL;
	char fw_path[100];
	char phone_fw_version[IS_HEADER_VER_SIZE + 1] = {0, };

	struct is_rom_info *finfo = NULL;

	is_sec_get_sysfs_finfo_by_position(id, &finfo);

	snprintf(fw_path, sizeof(fw_path), "%s%s", IS_FW_PATH, finfo->load_fw_name);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(fw_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		err("Camera: Failed open phone firmware");
		ret = -EIO;
		fp = NULL;
		goto read_phone_fw_exit;
	}

	fsize = fp->f_path.dentry->d_inode->i_size;
	info("start, file path %s, size %ld Bytes\n",
		fw_path, fsize);

	{
		info("Phone FW size is larger than FW buffer. Use vmalloc.\n");
		read_buf = vmalloc(fsize);
		if (!read_buf) {
			err("failed to allocate memory");
			ret = -ENOMEM;
			goto read_phone_fw_exit;
		}
		temp_buf = read_buf;
	}
	nread = vfs_read(fp, (char __user *)temp_buf, fsize, &fp->f_pos);
	if (nread != fsize) {
		err("failed to read firmware file, %ld Bytes", nread);
		ret = -EIO;
		goto read_phone_fw_exit;
	}

	strncpy(phone_fw_version, temp_buf + nread - (IS_HEADER_VER_SIZE + IS_SIGNATURE_LEN), IS_HEADER_VER_SIZE);
	strncpy(sysfs_pinfo[id].header_ver, temp_buf + nread - (IS_HEADER_VER_SIZE + IS_SIGNATURE_LEN), IS_HEADER_VER_SIZE);
	info("Camera[%d]: phone fw version: %s\n", id, phone_fw_version);

read_phone_fw_exit:
	if (read_buf) {
		vfree(read_buf);
		read_buf = NULL;
		temp_buf = NULL;
	}

	if (fp) {
		filp_close(fp, current->files);
		fp = NULL;
	}

	set_fs(old_fs);

	return ret;
}

int is_get_dual_cal_buf(int slave_position, char **buf, int *size)
{
	int ret = -1;
	char *cal_buf;
	u32 rom_dual_cal_start_addr;
	u32 rom_dual_cal_size;
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = NULL;

	if (specific->skip_cal_loading) {
		err("[%s]: skip cal loading - get_cal_buf fail", __func__);
		return ret;
	}

	ret = is_sec_get_cal_buf(SENSOR_POSITION_REAR, &cal_buf);
	if (ret < 0) {
		err("[%s]: get_cal_buf fail", __func__);
		return ret;
	}

	rom_addr = specific->rom_cal_map_addr[SENSOR_POSITION_REAR];
	if (rom_addr == NULL) {
		err("[%s]: rom_cal_map is NULL\n", __func__);
		return -EINVAL;
	}

	rom_dual_cal_start_addr = rom_addr->rom_dual_cal_data2_start_addr;
	rom_dual_cal_size       = rom_addr->rom_dual_cal_data2_size;

	*buf  = &cal_buf[rom_dual_cal_start_addr];
	*size = rom_dual_cal_size;

	return 0;
}

int is_get_remosaic_cal_buf(int slave_position, char **buf, int *size)
{
	int ret = -1;
	char *cal_buf;
	u32 start_addr, end_addr;
	u32 cal_size = 0;

	struct is_rom_info *finfo;

#ifdef DEBUG_XTALK_CAL_SIZE
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = NULL;
#endif
#ifdef REAR_XTALK_CAL_DATA_SIZE
	if (slave_position == SENSOR_POSITION_REAR)
		cal_size = REAR_XTALK_CAL_DATA_SIZE;
#endif
#ifdef FRONT_XTALK_CAL_DATA_SIZE
	if (slave_position == SENSOR_POSITION_FRONT)
		cal_size = FRONT_XTALK_CAL_DATA_SIZE;
#endif

	ret = is_sec_get_cal_buf(slave_position, &cal_buf);
	if (ret < 0) {
		err("[%s]: get_cal_buf fail", __func__);
		return ret;
	}

	ret = is_sec_get_sysfs_finfo_by_position(slave_position, &finfo);
	if (ret < 0) {
		err("[%s]: get_sysfs_finfo fail", __func__);
		return -EINVAL;
	}

#ifdef ENABLE_REMOSAIC_CAPTURE
	start_addr = finfo->sensor_cal_data_start_addr;
	end_addr = finfo->sensor_cal_data_end_addr;
#endif

#ifdef DEBUG_XTALK_CAL_SIZE
	rom_addr = specific->rom_cal_map_addr[slave_position];
	if (rom_addr == NULL) {
		err("[%s]: rom_cal_map is NULL\n", __func__);
		return -EINVAL;
	}

	info("[%s]: start_addr(0x%08X) end_addr(0x%08X) cal_size(%d) checksum(%d)\n",
		__func__, start_addr, end_addr, cal_size, rom_addr->rom_sensor_cal_checksum_len);
#endif

	if (cal_size <= 0 || end_addr < (start_addr + cal_size)) {
		err("[%s]: invalid start_addr(0x%08X) end_addr(0x%08X) cal_size(%d) \n",
			__func__, start_addr, end_addr, cal_size);
		return -EINVAL;
	}

	*buf  = &cal_buf[start_addr];
	*size = cal_size;

	return 0;
}

#ifdef JN1_MODIFY_REMOSAIC_CAL_ORDER
int is_get_jn1_remosaic_cal_buf(int slave_position, char **buf, int *size)
{
	int ret = -1;
	char *cal_buf;
	char *modified_cal_buf;
	u32 start_addr, end_addr;
	u32 cal_size = 0;
	u32 curr_pos = 0;

	struct is_rom_info *finfo;

#ifdef DEBUG_XTALK_CAL_SIZE
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	const struct is_vender_rom_addr *rom_addr = NULL;
#endif
#ifdef REAR_XTALK_CAL_DATA_SIZE
	if (slave_position == SENSOR_POSITION_REAR)
		cal_size = REAR_XTALK_CAL_DATA_SIZE;
#endif

	ret = is_sec_get_cal_buf(slave_position, &cal_buf);
	if (ret < 0) {
		err("[%s]: get_cal_buf fail", __func__);
		return ret;
	}

	ret = is_sec_get_sysfs_finfo_by_position(slave_position, &finfo);
	if (ret < 0) {
		err("[%s]: get_sysfs_finfo fail", __func__);
		return -EINVAL;
	}

#ifdef ENABLE_REMOSAIC_CAPTURE
	start_addr = finfo->sensor_cal_data_start_addr;
	end_addr = finfo->sensor_cal_data_end_addr;
#endif

#ifdef DEBUG_XTALK_CAL_SIZE
	rom_addr = specific->rom_cal_map_addr[slave_position];
	if (rom_addr == NULL) {
		err("[%s]: rom_cal_map is NULL\n", __func__);
		return -EINVAL;
	}

	info("[%s]: start_addr(0x%08X) end_addr(0x%08X) cal_size(%d) checksum(%d)\n",
		__func__, start_addr, end_addr, cal_size, rom_addr->rom_sensor_cal_checksum_len);
#endif

	if (cal_size <= 0 || end_addr < (start_addr + cal_size)) {
		err("[%s]: invalid start_addr(0x%08X) end_addr(0x%08X) cal_size(%d) \n",
			__func__, start_addr, end_addr, cal_size);
		return -EINVAL;
	}

	cal_size = REAR_REMOSAIC_TETRA_XTC_SIZE + REAR_REMOSAIC_SENSOR_XTC_SIZE + REAR_REMOSAIC_PDXTC_SIZE
		+ REAR_REMOSAIC_SW_GGC_SIZE;
	
	modified_cal_buf = (char *)kmalloc(cal_size * sizeof(char), GFP_KERNEL);
	
	memcpy(&modified_cal_buf[curr_pos], &cal_buf[REAR_REMOSAIC_TETRA_XTC_START_ADDR], REAR_REMOSAIC_TETRA_XTC_SIZE);
	curr_pos += REAR_REMOSAIC_TETRA_XTC_SIZE;

	memcpy(&modified_cal_buf[curr_pos], &cal_buf[REAR_REMOSAIC_SENSOR_XTC_START_ADDR], REAR_REMOSAIC_SENSOR_XTC_SIZE);
	curr_pos += REAR_REMOSAIC_SENSOR_XTC_SIZE;

	memcpy(&modified_cal_buf[curr_pos], &cal_buf[REAR_REMOSAIC_PDXTC_START_ADDR], REAR_REMOSAIC_PDXTC_SIZE);
	curr_pos += REAR_REMOSAIC_PDXTC_SIZE;

	memcpy(&modified_cal_buf[curr_pos], &cal_buf[REAR_REMOSAIC_SW_GGC_START_ADDR], REAR_REMOSAIC_SW_GGC_SIZE);

	*buf  = &modified_cal_buf[0];
	*size = cal_size;

	return 0;
}
#endif

int is_sec_run_fw_sel_from_rom(struct device *dev, int id, bool headerOnly)
{
	int i, ret = 0;

	int rom_position = id;
	int rom_type = ROM_TYPE_NONE;
	bool rom_valid = false;
	bool is_running_camera = false;
	char *buf = NULL;
	char *buf_rom_data = NULL;
	struct is_core *core = (struct is_core *)dev_get_drvdata(is_dev);
	struct is_vender_specific *specific = core->vender.private_data;
	struct is_rom_info *finfo = NULL;

#ifdef SUPPORT_SENSOR_DUALIZATION
	ret = is_sec_update_dualized_sensor(core, rom_position);
#endif

	is_sec_get_sysfs_finfo_by_position(id, &finfo);

	/* Use mutex for cal data rom */
	mutex_lock(&specific->rom_lock);

	for (i = SENSOR_POSITION_REAR; i < SENSOR_POSITION_MAX; i++) {
		if (specific->running_camera[i] == true) {
			info("Camera Running Check: %d\n", specific->running_camera[i]);
			is_running_camera = true;
			break;
		}
	}

	if (specific->rom_share[id].check_rom_share == true)
		rom_position = specific->rom_share[id].share_position;

	if ((finfo->is_caldata_read == false) || (force_caldata_dump == true)) {
		is_dumped_fw_loading_needed = false;
		if (force_caldata_dump)
			info("Forced Cal data dump!!! CAMERA_POSITION=%d\n", id);

		rom_type = specific->rom_data[rom_position].rom_type;
		rom_valid = specific->rom_data[rom_position].rom_valid;

		if (rom_valid == true) {
			if (specific->running_camera[rom_position] == false) {
				is_sec_rom_power_on(core, rom_position);
			}

			if (rom_type == ROM_TYPE_EEPROM) {
				info("Camera: Read Cal data from EEPROM[%d]\n", id);

				if (headerOnly) {
					info("Camera: Only Read Header[%d]\n", id);
					is_sec_read_eeprom_header(dev, id);
				} else {
					if (!is_sec_readcal_eeprom(dev, id)) {
						finfo->is_caldata_read = true;
						specific->rom_data[id].is_rom_read = true;
					}
				}
			} else if (rom_type == ROM_TYPE_OTPROM) {
				info("Camera: Read Cal data from OTPROM[%d]\n", id);

				if (!is_sec_readcal_otprom(dev, id)) {
					finfo->is_caldata_read = true;
					specific->rom_data[id].is_rom_read = true;
				}
			}

			/* Store original rom data before conversion for intrinsic cal */
			if (crc32_check_list[id][CRC32_CHECK] == true ) {
				is_sec_get_cal_buf(id, &buf);
				is_sec_get_cal_buf_rom_data(id, &buf_rom_data);
				if (buf != NULL && buf_rom_data != NULL) {
					memcpy(buf_rom_data, buf, is_sec_get_max_cal_size(core, id));
				}
			}
		}
	}

	is_sec_fw_find(core, id);

	if (headerOnly) {
		goto exit;
	}

	if (id != SENSOR_POSITION_REAR )
		goto exit;

	if (rom_valid == true)
		ret = is_sec_write_phone_firmware(id);

exit:
#if defined(USE_COMMON_CAM_IO_PWR)
	if (is_running_camera == false && rom_valid == true
#ifndef USES_STANDARD_CAL_RELOAD
		&& force_caldata_dump == false
#endif
	 ) {
		is_sec_rom_power_off(core, rom_position);
	}
#else
	if (rom_valid == true
#ifndef USES_STANDARD_CAL_RELOAD
		&& force_caldata_dump == false
#endif
	) {
		is_sec_rom_power_off(core, rom_position);
	}
#endif

	mutex_unlock(&specific->rom_lock);

	return ret;
}

int is_sec_get_pixel_size(char *header_ver)
{
	int pixelsize = 0;

	pixelsize += (int) (header_ver[FW_PIXEL_SIZE] - 0x30) * 10;
	pixelsize += (int) (header_ver[FW_PIXEL_SIZE + 1] - 0x30);

	return pixelsize;
}

int is_sec_core_voltage_select(struct device *dev, char *header_ver)
{
	struct regulator *regulator = NULL;
	int ret = 0;
	int minV, maxV;
	int pixelSize = 0;

	regulator = regulator_get_optional(dev, "cam_sensor_core_1.2v");
	if (IS_ERR_OR_NULL(regulator)) {
		err("%s : regulator_get fail", __func__);
		return -EINVAL;
	}
	pixelSize = is_sec_get_pixel_size(header_ver);

	if (header_ver[FW_SENSOR_MAKER] == FW_SENSOR_MAKER_SONY) {
		if (pixelSize == 13) {
			minV = 1050000;
			maxV = 1050000;
		} else if (pixelSize == 8) {
			minV = 1100000;
			maxV = 1100000;
		} else {
			minV = 1050000;
			maxV = 1050000;
		}
	} else if (header_ver[FW_SENSOR_MAKER] == FW_SENSOR_MAKER_SLSI) {
		minV = 1200000;
		maxV = 1200000;
	} else {
		minV = 1050000;
		maxV = 1050000;
	}

	ret = regulator_set_voltage(regulator, minV, maxV);

	if (ret >= 0)
		info("%s : set_core_voltage %d, %d successfully\n", __func__, minV, maxV);
	regulator_put(regulator);

	return ret;
}

void remove_dump_fw_file(void)
{
	mm_segment_t old_fs;
	/* int old_mask = 0; */
	long ret;
	char fw_path[100];
	struct is_rom_info *sysfs_finfo = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* old_mask = sys_umask(0); */ /*Fix Me: 4.19 version */

	is_sec_get_sysfs_finfo(&sysfs_finfo);

	/* RTA binary */
	snprintf(fw_path, sizeof(fw_path), "%s%s", IS_FW_DUMP_PATH, sysfs_finfo->load_rta_fw_name);

	ret = ksys_unlink(fw_path);
	info("sys_unlink (%s) %ld", fw_path, ret);

	/* DDK binary */
	snprintf(fw_path, sizeof(fw_path), "%s%s", IS_FW_DUMP_PATH, sysfs_finfo->load_fw_name);

	ret = ksys_unlink(fw_path);
	info("sys_unlink (%s) %ld", fw_path, ret);

	/* sys_umask(old_mask); */ /*Fix Me: 4.19 version */
	set_fs(old_fs);

	is_dumped_fw_loading_needed = false;
}

static void *is_sec_search_rom_extend_data(const struct rom_extend_cal_addr *extend_data, char *name)
{
	void *ret = NULL;

	const struct rom_extend_cal_addr *cur;
	cur = extend_data;

	while (cur != NULL) {
		if (!strcmp(cur->name, name)) {
			if (cur->data != NULL) {
				ret = (void *)cur->data;
			} else {
				warn("[%s] : Found -> %s, but no data \n", __func__, cur->name);
				ret = NULL;
			}
			break;
		}
		cur = cur->next;
	}

	return ret;
}
