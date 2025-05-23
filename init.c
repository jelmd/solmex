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
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "common.h"
#include "init.h"

static uint8_t started = 0;

static char *versionProm = NULL;	// version string emitted via /metrics
static char *versionHR = NULL;		// version string emitted to stdout/stderr

#define MMATCH(_x)	(cfg->_x && (regexec(cfg->_x, buf, 0,NULL,0) == 0))
#define SMATCH(_x)	(cfg->_x && (regexec(cfg->_x, e->prom.name, 0,NULL,0) == 0))

uint8_t
start(bool dmi, bool kstats, bool compact, uint32_t *tasks) {
	struct stat sbuf;
	int status;
	uint8_t res = 0;

	if (started)
		return 0;

	*tasks = 0;
	if (dmi) {
		if (((status = lstat("/dev/smbios", &sbuf)) != 0)
			|| (sbuf.st_mode & S_IFLNK) != S_IFLNK)
		{
			PROM_WARN("'/dev/smbios' symlink not found. DMI infos n/a.", "");
			res |= 1;
		} else {
			(*tasks)++;
		}
	}
	if (kstats) {
		if (((status = lstat("/dev/kstat", &sbuf)) != 0)
			|| (((sbuf.st_mode & S_IFLNK) != S_IFLNK)
				&& ((sbuf.st_mode & S_IFCHR) != S_IFCHR)))
		{
			PROM_WARN("'/dev/kstat' not found. Kernel stats n/a.", "");
			res |= 2;
		} else {
			(*tasks)++;
		}
	}

	if (!compact)
	PROM_INFO("node task initialized (%d)", *tasks);

	started = 1;
	return res;
}

void
stop() {
	free(versionHR);
	versionHR = NULL;
	free(versionProm);
	versionProm = NULL;
	PROM_DEBUG("Node stack has been properly shutdown", "");
	started = 0;
}

char *
getVersions(psb_t *sbp, bool compact) {
	psb_t *sbi = NULL, *sb = NULL;

	if (versionProm != NULL) {
		goto end;
	}

	sbi = psb_new();
	sb = psb_new();
	if (sbi == NULL || sb == NULL) {
		psb_destroy(sbi);
		psb_destroy(sb);
		return NULL;
	}

	psb_add_str(sbi, "solmex " SOLMEX_VERSION "\n(C) 2025 " SOLMEX_AUTHOR "\n");
	versionHR = psb_dump(sbi);
	psb_destroy(sbi);

	if (!compact)
		addPromInfo(SOLMEXM_VERS);

	psb_add_str(sb, SOLMEXM_VERS_N "{name=\"server\",value=\"" SOLMEX_VERSION
		"\"} 1\n");

	versionProm = psb_dump(sb);
	psb_destroy(sb);

end:
	if (sbp == NULL) {
		fprintf(stdout, "%s", versionHR);
	} else {
		psb_add_str(sbp, versionProm);
	}
	return versionHR;
}
