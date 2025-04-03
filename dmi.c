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
#include <ctype.h>
#include <errno.h>
#include <strings.h>

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

static char *dmi[DMI_INFO_SZ];	// the final per package alias socket metrics
static bool hasType[4];			// BIOS sections seen/analyzed
static uint16_t cpu_count;	// number of populated packages alias sockets found
static bool initialized;

typedef struct smbios_cpu {
	id_t id;				/* BIOS ID 0 is always the BIOS information */
	// clkspeed is usually 'Unknown' and therefore no help
	// core*  same thing with - always 0
	uint32_t maxspeed;		/* maximum speed [MHz] */
	uint32_t l1_size;		/* level 1 cache size [bytes] */
	uint32_t l2_size;		/* level 2 cache size [bytes] */
	uint32_t l3_size;		/* level 3 cache size [bytes] */
	bool enabled;			/* whether this CPU is enabled */
} smbios_cpu_t;

static smbios_cpu_t *cpu_info = NULL;
static size_t cpu_info_sz = 0;

static char *
strtrimdup(const char *str) {
	size_t len;
	char *t, *s;

	if (str == NULL)
		return NULL;

	len = strlen(str);
	if (len == 0)
		return NULL;

	s = strdup(str);
	t = s + len;
	while (t > s) {
		t--;
		if (!isspace(t[0]))
			break;

		t[0] = '\0';
	}
	if (s[0] == '\0') {
		free(s);
		return NULL;
	}
	// they are usually pretty small so do not re-allocate, even if shortened
	while (t > s) {
		t--;
		if (isspace(t[0])) {
			t[0] = ' ';		// replace all *tabs, CRs LFs with a whitespace
			continue;
		}
		// replace " with single quote char to make it attr val safe
		if (t[0] == '"')
			t[0] = '\'';
	}
	return s;
}

static int
cacheSize(smbios_hdl_t *shp, id_t biosID) {
	smbios_struct_t lcp;
	smbios_cache_t cache;

	if (biosID < 1)
		return 0;
	if (smbios_lookup_id(shp, biosID, &lcp) == SMB_ERR)
		return 0;
	if (smbios_info_cache(shp, biosID, &cache) == SMB_ERR)
		return 0;

	return (cache.smba_maxsize > 0) ? cache.smba_size : 0;
}

static int
recordCpu(smbios_hdl_t *shp, const smbios_struct_t *sp, void *arg) {
	(void) arg;					// make gcc happy
	smbios_processor_t info;
	uint8_t status;
	smbios_cpu_t *ci;

	// returning anything != 0 would stop iteration over smbios chain.
	if (sp->smbstr_type != SMB_TYPE_PROCESSOR)
		return 0;
	if (cpu_count > system_cpu_max) {
		PROM_WARN("Max. number of supported CPUs reached (%d)). Bios entry %d "
			"(and all subsequent) ignored.", system_cpu_max, sp->smbstr_id);
		return 1;	// stop iteration
	}
	if (smbios_info_processor(shp, sp->smbstr_id, &info) != 0) {
		PROM_WARN("Ooops! Data error for BIOS entry %s.", sp->smbstr_id);
		return 0;
	}
	if (!SMB_PRSTATUS_PRESENT(info.smbp_status))
		return 0;

	if (cpu_info == NULL) {
		cpu_info = calloc(8, sizeof(smbios_cpu_t)); // most systems have < 8 packages
		if (cpu_info == NULL) {
			PROM_WARN("Memory problem in dmi: %s - cpu %d ignored",
				strerror(errno), cpu_count);
			return 2;
		}
		cpu_info_sz = 8;
	} else if (cpu_count == cpu_info_sz) {
		size_t nl = cpu_info_sz << 1;
		if ((nl - 1) > system_cpu_max)
			nl = system_cpu_max + 1;
		if (cpu_count == nl) {
			PROM_WARN("CPU limit reached in dmi - cpu %d ignored", cpu_count);
			return 3;
		}
		smbios_cpu_t *p = realloc(cpu_info, nl * sizeof(smbios_cpu_t));
		if (p == NULL) {
			PROM_WARN("Memory problem in dmi: %s - cpu %d ignored",
					  strerror(errno), cpu_count);
			return 3;
		} else {
			cpu_info = p;
			memset(&(p[cpu_info_sz]), 0, (nl - cpu_info_sz) * sizeof(smbios_cpu_t));
			cpu_info_sz = nl;
		}
	}
	ci = &cpu_info[cpu_count];
	cpu_count++;

	ci->id = sp->smbstr_id;
	ci->maxspeed = info.smbp_maxspeed;
	ci->l1_size = cacheSize(shp, info.smbp_l1cache);
	ci->l2_size = cacheSize(shp, info.smbp_l2cache);
	ci->l3_size = cacheSize(shp, info.smbp_l3cache);
	status = SMB_PRSTATUS_STATUS(info.smbp_status);
	ci->enabled = (status == SMB_PRS_ENABLED) || (status == SMB_PRS_IDLE);
	return 0;
}

static void
copyBiosInfo(smbios_hdl_t *shp) {
	smbios_bios_t bios;
	id_t id;
	char buf[32];
	int n = 0;

	if ((id = smbios_info_bios(shp, &bios)) != SMB_ERR) {
		if (bios.smbb_vendor[0] != '\0') {
			dmi[BIOS_VENDOR] = strtrimdup(bios.smbb_vendor);
			n++;
		}
		if (bios.smbb_version[0] != '\0') {
			dmi[BIOS_VERSION] = strtrimdup(bios.smbb_version);
			n++;
		}
		if (bios.smbb_reldate[0] != '\0') {
			dmi[BIOS_DATE] = strtrimdup(bios.smbb_reldate);
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

	if ((id = smbios_info_system(shp, &sys)) != SMB_ERR
		&& smbios_info_common(shp, id, &info) != SMB_ERR)
	{
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[SYSTEM_VENDOR] = strtrimdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_product[0] != '\0') {
			dmi[PRODUCT_NAME] = strtrimdup(info.smbi_product);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[PRODUCT_VERSION] = strtrimdup(info.smbi_version);
			n++;
		}
		if (sys.smbs_sku[0] != '\0') {
			dmi[PRODUCT_SKU] = strtrimdup(sys.smbs_sku);
			n++;
		}
		if (sys.smbs_family[0] != '\0') {
			dmi[PRODUCT_FAMILY] = strtrimdup(sys.smbs_family);
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
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[BOARD_VENDOR] = strtrimdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_product[0] != '\0') {
			dmi[BOARD_NAME] = strtrimdup(info.smbi_product);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[BOARD_REVISON] = strtrimdup(info.smbi_version);
			n++;
		}
		if (info.smbi_asset[0] != '\0') {
			dmi[BOARD_ASSET] = strtrimdup(info.smbi_asset);
			n++;
		}
	}
	hasType[2] = n != 0;

	// should always return 3, but to make sure
	return (smbios_info_bboard(shp, 2, &board) == 0) ? board.smbb_chassis : 3;
}

static void
copyChassisInfo(smbios_hdl_t *shp, int id) {
	smbios_info_t info;
	int n = 0;

	if (smbios_info_common(shp, id, &info) == 0) {
		if (info.smbi_manufacturer[0] != '\0') {
			dmi[CHASSIS_VENDOR] = strtrimdup(info.smbi_manufacturer);
			n++;
		}
		if (info.smbi_version[0] != '\0') {
			dmi[CHASSIS_VERSION] = strtrimdup(info.smbi_version);
			n++;
		}
		if (info.smbi_asset[0] != '\0') {
			dmi[CHASSIS_ASSET] = strtrimdup(info.smbi_asset);
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

	sz = psb_len(sb);
	if (!compact)
		addPromInfo(SOLMEXM_DMI);
	if ((shp = smbios_open(NULL, SMB_VERSION, 0, &n)) == NULL) {
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
		smbios_iter(shp, recordCpu, NULL);

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
	metric = strdup(psb_str(sb) + sz);
	initialized = true;
}

int64_t
get_cache_size(uint16_t cpuNum) {
	if (!initialized) {
		psb_t *sb = psb_new();
		// if dmi collector is disabled, it got not called yet
		collect_dmi(sb, true);
		psb_destroy(sb);
	}
	if (cpuNum >= cpu_count)
		return -1;

	smbios_cpu_t *c = &cpu_info[cpuNum];
	return c->l3_size != 0 ? c->l3_size : (c->l2_size > 0 ? c->l2_size : c->l1_size);
}

int64_t
get_turbo_speed(uint16_t cpuNum) {
	if (initialized) {
		psb_t *sb = psb_new();
		// if dmi collector is disabled, it got not called yet
		collect_dmi(sb, true);
		psb_destroy(sb);
	}
	if (cpuNum >= cpu_count)
		return -1;

	return cpu_info[cpuNum].maxspeed * 1e6;
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
