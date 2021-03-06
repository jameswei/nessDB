/*
 * Copyright (c) 2012, BohuTANG <overred.shuttler at gmail dot com>
 * All rights reserved.
 * Code is licensed with GPL. See COPYING.GPL file.
 *
 */

#include "db.h"
#include "index.h"
#include "buffer.h"
#include "debug.h"
#include "xmalloc.h"

struct nessdb *db_open(const char *basedir)
{
	struct nessdb *db;

	db = xcalloc(1, sizeof(struct nessdb));
	db->stats = xcalloc(1, sizeof(struct stats));
	db->stats->STATS_START_TIME = time(NULL);
	db->idx = index_new(basedir, NESSDB_MAX_MTB_SIZE, db->stats);

	return db;
}

STATUS db_add(struct nessdb *db, struct slice *sk, struct slice *sv)
{
	return index_add(db->idx, sk, sv);
}

STATUS db_get(struct nessdb *db, struct slice *sk, struct slice *sv)
{
	int ret;

	ret = index_get(db->idx, sk, sv);

	return ret;
}

void db_stats(struct nessdb *db, struct slice *stats)
{
	int arch_bits = (sizeof(long) == 8) ? 64 : 32;
	time_t uptime = time(NULL) - db->stats->STATS_START_TIME;
	int upday = uptime / (3600 * 24);

	snprintf(stats->data, stats->len, 
			"# nessDB\r\n"
			"\tversion:%s\r\n"
			"\tarch_bits:%d\r\n"
			"\tgcc_version:%d.%d.%d\r\n"
			"\tprocess_id:%ld\r\n"
			"\tuptime_in_seconds:%d\r\n"
			"\tuptime_in_days:%d\r\n"

			"# Stats\r\n"
			"\ttotal_read_count:%llu\r\n"
			"\ttotal_write_count:%llu\r\n"
			"\ttotal_remove_count:%llu\r\n"
			"\ttotal_r_from_mtbl:%llu\r\n"
			"\ttotal_r_from_disk:%llu\r\n"
			"\ttotal_crc_errors:%llu\r\n"
			"\ttotal_compress_count:%llu\r\n"
			"\ttotal_hole_reuse_count:%llu\r\n"
			"\ttotal_mtbl_count:%llu\r\n"
			"\ttotal_merging_mtbl_count:%llu\r\n"
			,
		NESSDB_VERSION,
		arch_bits,
#ifdef __GNUC__
		__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__,
#else
		0,0,0,
#endif
		(long)getpid(),
		(int)uptime,
		upday,

		db->stats->STATS_READS, 
		db->stats->STATS_WRITES, 
		db->stats->STATS_REMOVES, 
		db->stats->STATS_R_FROM_MTBL, 
		db->stats->STATS_R_COLA,
		db->stats->STATS_CRC_ERRS, 
		db->stats->STATS_COMPRESSES, 
		db->stats->STATS_HOLE_REUSES,
		db->stats->STATS_MTBL_COUNTS,
		db->stats->STATS_MTBL_MERGING_COUNTS);
}

void db_remove(struct nessdb *db, struct slice *sk)
{
	index_remove(db->idx, sk);
}

void db_close(struct nessdb *db)
{
	index_free(db->idx);
	free(db->stats);
	free(db);
}
