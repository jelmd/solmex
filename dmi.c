/*
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License") 1.1!
 * You may not use this file except in compliance with the License.
 *
 * See  https://spdx.org/licenses/CDDL-1.1.html  for the specific
 * language governing permissions and limitations under the License.
 *
 * Copyright 2025 Jens Elkner (jel+solmex-src@cs.ovgu.de)
 */

#include <stdio.h>
#include <smbios.h>

#include <libprom/prom.h>

#include "dmi.h"

// for Linux comparision see
// https://web.git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/firmware/dmi_scan.c

// for Solaris/Illumos see
// /usr/include/sys/smbios.h
// /usr/include/sys/smbios_impl.h
// illumos-gate/usr/src/cmd/smbios/smbios.c
// illumos-gate/usr/src/common/smbios/smb_info.c
// illumos-gate/usr/src/cmd/prtdiag/i386/smbios.c

typedef enum {
	// type 0
	BIOS_VENDOR = 0,	// bios_vendor="American Megatrends Inc."
	BIOS_VERSION,		// bios_version="4001"
	BIOS_DATE,			// bios_date="10/04/2024"
	BIOS_RELEASE,		// bios_release="40.1"
	// type 1
	SYSTEM_VENDOR,		// system_vendor="ASUSTeK COMPUTER INC."
	PRODUCT_NAME,		// product_name="System Product Name"
	PRODUCT_VERSION,	// product_version="System Version"
	PRODUCT_SKU,		// product_sku="SKU"
	PRODUCT_FAMILY,		// product_family="Server"
	// type 2
	BOARD_VENDOR,		// board_vendor="ASUSTeK COMPUTER INC."
	BOARD_NAME,			// board_name="Pro WS W680-ACE IPMI"
	BOARD_REVISON,		// board_version="Rev 1.xx"
	BOARD_ASSET,		// board_asset_tag="Default string"
	// type 3
	CHASSIS_VENDOR,		// chassis_vendor="Default string"
	CHASSIS_VERSION,	// chassis_version="Default string"
	CHASSIS_ASSET,		// chassis_asset_tag="Default string"
	// the CPU infos provided by libsmbios are not sufficient => use kstats

	// last entry: do not use
	DMI_INFO_SZ			// reserved
} DMI_INFO;

static const char *DMI_ATTR[] = {
	// type 0
	"bios_vendor",
	"bios_version",
	"bios_date",
	"bios_release",
	// type 1
	"system_vendor",
	"product_name",
	"product_version",
	"product_sku",
	"product_family",
	// type 2
	"board_vendor",
	"board_name",
	"board_version",
	"board_asset_tag",
	// type 3
	"chassis_vendor",
	"chassis_version",
	"chassis_asset_tag",
};

static char *dmi[DMI_INFO_SZ];		// automagically initialized with NULL
static bool hasType[4];				// automagically initialized with false
static bool initialized = false;

static void
copyBiosInfo(smbios_hdl_t *shp) {
	smbios_bios_t bios;
	id_t id;
	char buf[8];
	int n = 0;

	if ((id = smbios_info_bios(shp, &bios)) != SMB_ERR) {
		if (bios.smbb_vendor[0] != '\0') {
			dmi[BIOS_VENDOR] = strdup(bios.smbb_vendor);
			n++;
		}
		if (bios.smbb_version[0] != '\0') {
			dmi[BIOS_VERSION] = strdup(bios.smbb_version);
			n++;
		}
		if (bios.smbb_reldate[0] != '\0') {
			dmi[BIOS_DATE] = strdup(bios.smbb_reldate);
			n++;
		}
		// Per spec only supported if both != 0xFF
		if (bios.smbb_biosv.smbv_major != 0xff
			&& bios.smbb_biosv.smbv_minor != 0xff)
		{
			id = sprintf(buf, "%d.%d",
				bios.smbb_biosv.smbv_major, bios.smbb_biosv.smbv_minor);
			dmi[BIOS_RELEASE] = strdup(buf);
			n++;
		}
	}
	hasType[0] = n != 0;
}

static void
copyProductInfo(smbios_hdl_t *shp) {
	smbios_system_t sys;
	smbios_info_t info;
	id_t id;
	int n = 0;

	if ((id = smbios_info_system(shp, &sys)) != SMB_ERR &&
		smbios_info_common(shp, id, &info) != SMB_ERR)
	{
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[SYSTEM_VENDOR] = strdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_product[0] != '\0') {
			dmi[PRODUCT_NAME] = strdup(info.smbi_product);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[PRODUCT_VERSION] = strdup(info.smbi_version);
			n++;
		}
		if (sys.smbs_sku[0] != '\0') {
			dmi[PRODUCT_SKU] = strdup(sys.smbs_sku);
			n++;
		}
		if (sys.smbs_family[0] != '\0') {
			dmi[PRODUCT_FAMILY] = strdup(sys.smbs_family);
			n++;
		}
	}
	hasType[1] = n != 0;
}

static int
copyBaseboardInfo(smbios_hdl_t *shp) {
	smbios_info_t info;
	smbios_bboard_t board;
	int n = 0;

	if (smbios_info_common(shp, 2, &info) == 0) {
		if (info.smbi_manufacturer)
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[BOARD_VENDOR] = strdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_product[0] != '\0') {
			dmi[BOARD_NAME] = strdup(info.smbi_product);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[BOARD_REVISON] = strdup(info.smbi_version);
			n++;
		}
		if (info.smbi_asset[0] != '\0') {
			dmi[BOARD_ASSET] = strdup(info.smbi_asset);
			n++;
		}
	}
	hasType[2] = n != 0;

	// should always return 3, but to make sure
	if (smbios_info_bboard(shp, 2, &board) == 0) {
		return board.smbb_chassis;
	}
	return 3;
}

static void
copyChassisInfo(smbios_hdl_t *shp, int id) {
	smbios_info_t info;
	int n = 0;

	if (smbios_info_common(shp, id, &info) == 0) {
		if (info.smbi_manufacturer)
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[CHASSIS_VENDOR] = strdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[CHASSIS_VERSION] = strdup(info.smbi_version);
			n++;
		}
		if (info.smbi_asset[0] != '\0') {
			dmi[CHASSIS_ASSET] = strdup(info.smbi_asset);
			n++;
		}
	}
	hasType[3] = n != 0;

}

static char *metric;

static void
buildMetric(psb_t *sb, bool compact) {
	smbios_hdl_t *shp;
	uint c;
	int n;
	size_t sz;
	char *buf;

	sz = psb_len(sb);
	if (!compact)
		addPromInfo(SOLMEXM_DMI);
	if ((shp = smbios_open(NULL, SMB_VERSION, 0, &n)) == NULL)
	{
		PROM_WARN("Unable to open /dev/smbios.", "");
		if (!compact) {
			 psb_add_str(sb, "# /dev/smbios n/a");
			 psb_add_char(sb, '\n');
		}
	} else {
		c = 0;
		copyBiosInfo(shp);
		copyProductInfo(shp);
		n = copyBaseboardInfo(shp);
		copyChassisInfo(shp, n);

		psb_add_str(sb, SOLMEXM_DMI_N "{");
		n = ARRAY_SIZE(dmi);
		if (n > DMI_INFO_SZ)
			n = DMI_INFO_SZ;
		n--;
		for (; n >= 0; n--) {
			if (dmi[n] == NULL)
				continue;
			psb_add_str(sb, DMI_ATTR[n]);
			psb_add_str(sb, "=\"");
			psb_add_str(sb, dmi[n]);
			psb_add_str(sb, "\",");
			// for now we do not need the attrs anymore, so save some bytes
			free(dmi[n]);
			dmi[n] = NULL;
			c++;
		}
		if (c > 0) {
			n = psb_len(sb);
			psb_truncate(sb, n - 1);	// remove the trailing comma
		}
		psb_add_str(sb, "} 1");
		psb_add_char(sb, '\n');
		smbios_close(shp);
	}
	n = psb_len(sb);
	buf = psb_dump(sb);
	metric = strdup(buf + sz);
	free(buf);
	initialized = true;
}

void
collect_dmi(psb_t *sb, bool compact) {

	PROM_DEBUG("collect_dmi ...", "");

	bool free_sb = sb == NULL;
	if (free_sb)
		sb = psb_new();

	if (initialized) {
		psb_add_str(sb, metric);
	} else {
		buildMetric(sb, compact);
	}

	if (free_sb) {
		fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
	PROM_DEBUG("Final DMI info size: %d", strlen(metric));
}
