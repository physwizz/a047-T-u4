/*
* Samsung Exynos5 SoC series IS driver
 *
 * exynos5 is vender functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef IS_VENDER_SPECIFIC_H
#define IS_VENDER_SPECIFIC_H

#define JDM_MAX_GROUP		3
#define JDM_MAX_SEG		10

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>

/* #define USE_ION_ALLOC */
#define I2C_RETRY_COUNT			5

enum is_crc32_check_list {
	CRC32_CHECK_HEADER		= 0,
	CRC32_CHECK			= 1,
	CRC32_CHECK_FACTORY	= 2,
	CRC32_CHECK_SETFILE	= 3,
	CRC32_CHECK_FW		= 4,
	CRC32_CHECK_FW_VER		= 5,
	CRC32_CHECK_STANDARD_CAL	= 6,
	CRC32_SCENARIO_MAX,
};

struct is_rom_share {
	bool 		check_rom_share;
	int			share_position;
};

struct rom_extend_cal_addr {
	char *name;
	void *data;
	struct rom_extend_cal_addr *next;
};

/***** Extend data define of Calibration map(ROM address)  *****/
#define EXTEND_OEM_CHECKSUM       "oem_checksum_base_addr"
#define EXTEND_AE_CAL             "ae_cal_data"
#define EXTEND_STANDARD_CAL       "standard_cal_data"
#define EXTEND_AP2AP_STANDARD_CAL "ap2ap_standard_cal_data"

struct rom_ae_cal_data {
	int32_t		rom_header_main_ae_start_addr;
	int32_t		rom_header_main_ae_end_addr;
	int32_t		rom_ae_module_info_start_addr;
	int32_t		rom_ae_checksum_addr;
	int32_t		rom_ae_checksum_len;
};

struct rom_standard_cal_data{
	int32_t		rom_standard_cal_start_addr;
	int32_t		rom_standard_cal_end_addr;
	int32_t		rom_standard_cal_module_crc_addr;
	int32_t		rom_standard_cal_module_checksum_len;
	int32_t		rom_header_standard_cal_end_addr;
	int32_t		rom_standard_cal_sec2lsi_end_addr;

	int32_t		rom_awb_start_addr;
	int32_t		rom_awb_end_addr;
	int32_t		rom_shading_start_addr;
	int32_t		rom_shading_end_addr;
	
	int32_t		rom_awb_sec2lsi_start_addr;
	int32_t		rom_awb_sec2lsi_end_addr;
	int32_t		rom_awb_sec2lsi_checksum_addr;
	int32_t		rom_awb_sec2lsi_checksum_len;

	int32_t		rom_shading_sec2lsi_start_addr;
	int32_t		rom_shading_sec2lsi_end_addr;
	int32_t		rom_shading_sec2lsi_checksum_addr;
	int32_t		rom_shading_sec2lsi_checksum_len;
};

struct rom_ap2ap_standard_cal_data{
	int32_t		rom_orig_start_addr;
	int32_t		rom_orig_end_addr;
	int32_t		rom_lsi_start_addr;
	int32_t		rom_lsi_end_addr;

	int32_t		rom_num_of_banks;
	int32_t		rom_num_of_segments;

	int32_t		rom_bank_start_addr[JDM_MAX_SEG][JDM_MAX_GROUP];
	int32_t		rom_seg_checksum_addr[JDM_MAX_SEG];
	int32_t		rom_seg_checksum_len[JDM_MAX_SEG];
	int32_t		rom_total_checksum_addr[JDM_MAX_GROUP];
	int32_t		rom_seg_size[JDM_MAX_SEG];
};

struct is_vender_rom_addr {
	/* Set '-1' if not used */

/***** ROM Version Check Section *****/
	char			camera_module_es_version;
	char			cal_map_es_version;
	int32_t		rom_max_cal_size;

/***** HEADER Referenced Section ******/
	int32_t		rom_header_cal_data_start_addr;
	int32_t		rom_header_main_module_info_start_addr;
	int32_t		rom_header_cal_map_ver_start_addr;
	int32_t		rom_header_project_name_start_addr;
	int32_t		rom_header_module_id_addr;
	int32_t		rom_header_main_sensor_id_addr;

	int32_t		rom_header_sub_module_info_start_addr;
	int32_t		rom_header_sub_sensor_id_addr;

	int32_t		rom_header_main_header_start_addr;
	int32_t		rom_header_main_header_end_addr;
	int32_t		rom_header_main_oem_start_addr;
	int32_t		rom_header_main_oem_end_addr;
	int32_t		rom_header_main_awb_start_addr;
	int32_t		rom_header_main_awb_end_addr;
	int32_t		rom_header_main_shading_start_addr;
	int32_t		rom_header_main_shading_end_addr;
	int32_t		rom_header_main_sensor_cal_start_addr;
	int32_t		rom_header_main_sensor_cal_end_addr;
	int32_t		rom_header_dual_cal_start_addr;
	int32_t		rom_header_dual_cal_end_addr;
	int32_t		rom_header_pdaf_cal_start_addr;
	int32_t		rom_header_pdaf_cal_end_addr;

	int32_t		rom_header_sub_oem_start_addr;
	int32_t		rom_header_sub_oem_end_addr;
	int32_t		rom_header_sub_awb_start_addr;
	int32_t		rom_header_sub_awb_end_addr;
	int32_t		rom_header_sub_shading_start_addr;
	int32_t		rom_header_sub_shading_end_addr;

	int32_t		rom_header_main_mtf_data_addr;
	int32_t		rom_header_sub_mtf_data_addr;

	int32_t		rom_header_checksum_addr;
	int32_t		rom_header_checksum_len;

/***** OEM Referenced Section *****/
	int32_t		rom_oem_af_inf_position_addr;
	int32_t		rom_oem_af_macro_position_addr;
	int32_t		rom_oem_module_info_start_addr;
	int32_t		rom_oem_checksum_addr;
	int32_t		rom_oem_checksum_len;

/***** AWB Referenced section *****/
	int32_t		rom_awb_module_info_start_addr;
	int32_t		rom_awb_checksum_addr;
	int32_t		rom_awb_checksum_len;

/***** Shading Referenced section *****/
	int32_t		rom_shading_module_info_start_addr;
	int32_t		rom_shading_checksum_addr;
	int32_t		rom_shading_checksum_len;

/***** SENSOR CAL(CrossTalk Cal for Remosaic) Referenced section *****/
	int32_t		rom_sensor_cal_module_info_start_addr;
	int32_t		rom_sensor_cal_checksum_addr;
	int32_t		rom_sensor_cal_checksum_len;

/***** DUAL DATA Referenced section *****/
	int32_t		rom_dual_module_info_start_addr;
	int32_t		rom_dual_checksum_addr;
	int32_t		rom_dual_checksum_len;

/***** PDAF CAL Referenced section *****/
	int32_t		rom_pdaf_module_info_start_addr;
	int32_t		rom_pdaf_checksum_addr;
	int32_t		rom_pdaf_checksum_len;

/***** SUB OEM Referenced Section *****/
	int32_t		rom_sub_oem_af_inf_position_addr;
	int32_t		rom_sub_oem_af_macro_position_addr;
	int32_t		rom_sub_oem_module_info_start_addr;
	int32_t		rom_sub_oem_checksum_addr;
	int32_t		rom_sub_oem_checksum_len;

/***** SUB AWB Referenced section *****/
	int32_t		rom_sub_awb_module_info_start_addr;
	int32_t		rom_sub_awb_checksum_addr;
	int32_t		rom_sub_awb_checksum_len;

/***** SUB Shading Referenced section *****/
	int32_t		rom_sub_shading_module_info_start_addr;
	int32_t		rom_sub_shading_checksum_addr;
	int32_t		rom_sub_shading_checksum_len;

/***** Dual Calibration Data2 *****/
	int32_t		rom_dual_cal_data2_start_addr;
	int32_t		rom_dual_cal_data2_size;
	int32_t		rom_dual_tilt_x_addr;
	int32_t		rom_dual_tilt_y_addr;
	int32_t		rom_dual_tilt_z_addr;
	int32_t		rom_dual_tilt_sx_addr;
	int32_t		rom_dual_tilt_sy_addr;
	int32_t		rom_dual_tilt_range_addr;
	int32_t		rom_dual_tilt_max_err_addr;
	int32_t		rom_dual_tilt_avg_err_addr;
	int32_t		rom_dual_tilt_dll_version_addr;
	int32_t		rom_dual_tilt_dll_model_id_addr;
	int32_t		rom_dual_tilt_dll_model_id_size;
	int32_t		rom_dual_shift_x_addr;
	int32_t		rom_dual_shift_y_addr;

/***** Extend Cal Data *****/
	const struct rom_extend_cal_addr		*extend_cal_addr;
};

struct is_vender_specific {
	struct mutex							rom_lock;
	struct i2c_client						*rom_client[SENSOR_POSITION_MAX];
	struct is_rom_data				rom_data[SENSOR_POSITION_MAX];
	struct is_rom_share				rom_share[SENSOR_POSITION_MAX];
	const struct is_vender_rom_addr		*rom_cal_map_addr[SENSOR_POSITION_MAX];
#if defined(USE_DUALIZED_OTPROM_SENSOR) || defined(SUPPORT_SENSOR_DUALIZATION)
	struct i2c_client					*dualized_rom_client[SENSOR_POSITION_MAX];
	const struct is_vender_rom_addr		*dualized_rom_cal_map_addr[SENSOR_POSITION_MAX];
#endif

	bool			running_camera[SENSOR_POSITION_MAX];

	u8			standby_state;

	/* dt */
	u32			sensor_id[SENSOR_POSITION_MAX];
#if defined(USE_DUALIZED_OTPROM_SENSOR) || defined(SUPPORT_SENSOR_DUALIZATION)
	u32			dualized_sensor_id[SENSOR_POSITION_MAX];
#endif
#ifdef CONFIG_SECURE_CAMERA_USE
	u32			secure_sensor_id;
#endif

	bool			check_sensor_vendor;
	bool			skip_cal_loading;
	bool			use_ois_hsi2c;
	bool			use_ois;
	bool			use_module_check;
	bool			need_i2c_config;

	bool			suspend_resume_disable;
	bool			need_cold_reset;
	bool			zoom_running;


	/* Need to set define */
#ifdef USE_ION_ALLOC
	struct ion_client	*ion_client;
#endif

#ifdef CONFIG_OIS_USE
	bool			ois_ver_read;
#endif

#ifdef CONFIG_SENSOR_RETENTION_USE
	bool			need_retention_init;
#endif
	u32			rom_bank[SENSOR_POSITION_MAX];
};

#endif
