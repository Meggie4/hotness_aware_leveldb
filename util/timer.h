/*
 * timer.h
 *
 *  Created on: Sep 24, 2016
 *      Author: pandian
 */

#ifndef UTIL_TIMER_H_
#define UTIL_TIMER_H_

#include <assert.h>
#include "leveldb/env.h"
#include "util/logging.h"

//#define TIMER_LOG
//#define TIMER_LOG_SEEK
//#define TIMER_LOG_SIMPLE

#define NUM_SEEK_THREADS 4

namespace leveldb {

enum TimerStep {
	BEGIN,
	BUILD_LEVEL0_TABLES,
	GET_LOCK_AFTER_BUILD_LEVEL0_TABLES,
	ADD_LEVEL0_FILES_TO_EDIT,
	WRITE_LEVEL0_TABLE_GUARDS,
	CMT_ADD_GUARDS_TO_EDIT,
	CMT_ADD_COMPLETE_GUARDS_TO_EDIT,
	CMT_LOG_AND_APPLY,
	CMT_ERASE_PENDING_OUTPUTS,
	CMT_DELETE_OBSOLETE_FILES,
	TOTAL_MEMTABLE_COMPACTION,
	TOTAL_BACKGROUND_COMPACTION,
	BGC_PICK_COMPACTION_LEVEL,
	BGC_PICK_COMPACTION,
	BGC_ADD_GUARDS_TO_EDIT,
	BGC_ADD_COMPLETE_GUARDS_TO_EDIT,
	BGC_DO_COMPACTION_WORK_GUARDS,
	BGC_CLEANUP_COMPACTION,
	BGC_MAKE_INPUT_ITERATOR,
	BGC_ITERATE_KEYS_AND_SPLIT,
	BGC_OPEN_COMPACTION_OUTPUT_FILE,
	BGC_FINISH_COMPACTION_OUTPUT_FILE,
	BGC_COLLECT_STATS,
	BGC_GET_LOCK_BEFORE_INSTALL,
	BGC_INSTALL_COMPACTION_RESULTS,
	GET_OVERALL_TIME,
	GET_TIME_TO_GET_MUTEX,
	GET_TIME_TO_REF_MEMS,
	GET_TIME_TO_CHECK_MEM_IMM,
	GET_TIME_TO_CHECK_VERSION,
	GET_TIME_TO_LOCK_MUTEX,
	GET_TIME_TO_FINISH_UNREF,
	WRITE_SEQUENCE_WRITE_BEGIN_TOTAL,
	WRITE_SEQUENCE_WRITE_END_TOTAL,
	WRITE_SET_SEQUENCE_CREATE_NEW_BATCH,
	WRITE_SET_GUARDS,
	WRITE_LOG_ADDRECORD,
	WRITE_LOG_FILE_SYNC,
	WRITE_INSERT_INTO_VERSION,
	WRITE_OVERALL_TIME,
	SWB_INIT_MUTEX,
	SWB_INIT_MEMTABLES,
	SWB_SET_LOG_DETAILS,
	SWB_ENQUEUE_MEM,
	SWB_SET_TAIL,
	SWB_SYNC_AND_FETCH,
	SWB_UNLOCK_MUTEX,
	SWE_LOCK_MUTEX,
	SWE_UNLOCK_MUTEX,
	SWE_SET_PREV,
	SWE_SET_NEXT,
	SWE_LOCK_WRITERS_MUTEX,
	SWE_UNLOCK_WRITERS_MUTEX,
	SWE_SET_IMM,
	SWE_SLEEP,
	GET_FIND_GUARD,
	GET_FIND_GUARD_PARSE_INTERNAL_KEY,
	GET_CLEAR_TMP2_SET_G,
	GET_CHECK_SENTINEL_FILES,
	GET_SORT_SENTINEL_FILES,
	GET_CHECK_GUARD_FILES,
	GET_SORT_GUARD_FILES,
	GET_FIND_LIST_OF_FILES,
	GET_TABLE_CACHE_GET,
	GET_TABLE_CACHE_READ_DATA_BLOCK,
	GET_TABLE_CACHE_FIND_TABLE,
	GET_TABLE_CACHE_GET_FROM_DISK,
	GET_TABLE_CACHE_GET_NEW_RANDOM_ACCESS_FILE,
	GET_TABLE_CACHE_GET_TABLE_OPEN,
	GET_TABLE_CACHE_GET_TABLE_OPEN_FOOTER_READ,
	GET_TABLE_CACHE_GET_TABLE_OPEN_INDEX_BLOCK_READ,
	GET_TABLE_CACHE_GET_TABLE_OPEN_READ_META,
	GET_TABLE_CACHE_GET_INSERT_INTO_CACHE,
	GET_TABLE_CACHE_INDEX_ITER_SEEK,
	GET_TABLE_CACHE_FILTER_CHECK,
	GET_FILE_LEVEL_FILTER_CHECK,
	GET_TABLE_CACHE_INTERNAL_GET,
	GET_TABLE_CACHE_CREATE_THREAD,
	GET_TABLE_CACHE_NUM_THREADS_SIGNALLED,
	GET_TABLE_CACHE_NUM_DIRECT_CALLS,
	GET_TABLE_CACHE_SIGNAL_READ_THREADS,
	GET_TABLE_CACHE_WAIT_FOR_READ_THREADS,
	MTC_SAVETO_ADD_FILES,
	MTC_SAVETO_ADD_GUARDS,
	MTC_SAVETO_ADD_COMPLETE_GUARDS,
	MTC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET,
	MTC_SAVETO_POPULATE_FILES,
	MTC_LAA_APPLY_EDIT_TO_BUILDER,
	MTC_LAA_SAVETO,
	MTC_LAA_FINALIZE,
	MTC_LAA_CREATE_MANIFEST_LOG_WRITER,
	MTC_LAA_COMPLETE_WRITE_SNAPSHOT,
	MTC_LAA_ENCODE_EDIT,
	MTC_LAA_ADD_RECORD_TO_DESC_LOG,
	MTC_LAA_SYNC_MANIFEST_LOG_WRITE,
	MTC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC,
	MTC_LAA_GET_DELTA_COMPLETE_GUARDS,
	MTC_LAA_APPEND_VERSION,
	MTC_LAA_SYNC_COMPLETE_GUARDS,
	BGC_SAVETO_ADD_FILES,
	BGC_SAVETO_ADD_GUARDS,
	BGC_SAVETO_ADD_COMPLETE_GUARDS,
	BGC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET,
	BGC_SAVETO_POPULATE_FILES,
	BGC_LAA_APPLY_EDIT_TO_BUILDER,
	BGC_LAA_SAVETO,
	BGC_LAA_FINALIZE,
	BGC_LAA_CREATE_MANIFEST_LOG_WRITER,
	BGC_LAA_COMPLETE_WRITE_SNAPSHOT,
	BGC_LAA_ENCODE_EDIT,
	BGC_LAA_ADD_RECORD_TO_DESC_LOG,
	BGC_LAA_SYNC_MANIFEST_LOG_WRITE,
	BGC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC,
	BGC_LAA_GET_DELTA_COMPLETE_GUARDS,
	BGC_LAA_APPEND_VERSION,
	BGC_LAA_SYNC_COMPLETE_GUARDS,
	MTC_LAA_WAIT_FOR_LIVE_BACKUP,
	BGC_LAA_WAIT_FOR_LIVE_BACKUP,
	SEEK_NUM_TABLE_ITERATOR,
	SEEK_GUARD_NUM_ITERATOR,
	SEEK_PARALLEL_TOTAL,
	SEEK_SEQUENTIAL_TOTAL,
	SEEK_SEQUENTIAL_FILES,
	SEEK_PARALLEL_ASSIGN_THREADS,
	SEEK_PARALLEL_SIGNAL_THREADS,
	SEEK_PARALLEL_WAIT_FOR_THREADS,
	SEEK_TITERATOR_PARALLEL_TOTAL,
	SEEK_TITERATOR_SEQUENTIAL_TOTAL,
	SEEK_TITERATOR_SEQUENTIAL_FILES,
	SEEK_TITERATOR_PARALLEL_ASSIGN_THREADS,
	SEEK_TITERATOR_PARALLEL_SIGNAL_THREADS,
	SEEK_TITERATOR_PARALLEL_WAIT_FOR_THREADS,
	SEEK_REINIT,
	SEEK_REINIT_FILE_LEVEL,
	SEEK_NEXT,
	SEEK_NEXT_FILE_LEVEL,
	SEEK_NEXT_POP,
	SEEK_NEXT_PUSH,
	SEEK_NEXT_CURRENT_NEXT,
	SEEK_NEXT_POP_FILE_LEVEL,
	SEEK_NEXT_PUSH_FILE_LEVEL,
	SEEK_NEXT_CURRENT_NEXT_FILE_LEVEL,
	SEEK_THREAD_TOTAL_ACTIVE,
	SEEK_THREAD_ITER_SEEK,
	SEEK_BLOCK_READER_TOTAL,
	SEEK_BLOCK_READER_READ_BLOCK,
	END

};
class Timer {

private:
	uint64_t micros_array[200];
	uint64_t timer_micros[200];
	uint64_t timer_count[200];
	uint64_t timer_count_additional[200];
	std::string message[200];

public:
	Timer() {
		init();
	}

	~Timer() {
	}

	void init() {
		message[BUILD_LEVEL0_TABLES] = "BUILD_LEVEL0_TABLES";
		message[GET_LOCK_AFTER_BUILD_LEVEL0_TABLES] = "GET_LOCK_AFTER_BUILD_LEVEL0_TABLES";
		message[ADD_LEVEL0_FILES_TO_EDIT] = "ADD_LEVEL0_FILES_TO_EDIT";
		message[WRITE_LEVEL0_TABLE_GUARDS] = "WRITE_LEVEL0_TABLE_GUARDS";
		message[CMT_ADD_GUARDS_TO_EDIT] = "CMT_ADD_GUARDS_TO_EDIT";
		message[CMT_ADD_COMPLETE_GUARDS_TO_EDIT] = "CMT_ADD_COMPLETE_GUARDS_TO_EDIT";
		message[CMT_LOG_AND_APPLY] = "CMT_LOG_AND_APPLY";
		message[CMT_ERASE_PENDING_OUTPUTS] = "CMT_ERASE_PENDING_OUTPUTS";
		message[CMT_DELETE_OBSOLETE_FILES] = "CMT_DELETE_OBSOLETE_FILES";
		message[TOTAL_MEMTABLE_COMPACTION] = "TOTAL_MEMTABLE_COMPACTION";
		message[TOTAL_BACKGROUND_COMPACTION] = "TOTAL_BACKGROUND_COMPACTION";
		message[BGC_PICK_COMPACTION_LEVEL] = "BGC_PICK_COMPACTION_LEVEL";
		message[BGC_PICK_COMPACTION] = "BGC_PICK_COMPACTION";
		message[BGC_ADD_GUARDS_TO_EDIT] = "BGC_ADD_GUARDS_TO_EDIT";
		message[BGC_ADD_COMPLETE_GUARDS_TO_EDIT] = "BGC_ADD_COMPLETE_GUARDS_TO_EDIT";
		message[BGC_DO_COMPACTION_WORK_GUARDS] = "BGC_DO_COMPACTION_WORK_GUARDS";
		message[BGC_CLEANUP_COMPACTION] = "BGC_CLEANUP_COMPACTION";
		message[BGC_MAKE_INPUT_ITERATOR] = "BGC_MAKE_INPUT_ITERATOR";
		message[BGC_ITERATE_KEYS_AND_SPLIT] = "BGC_ITERATE_KEYS_AND_SPLIT";
		message[BGC_OPEN_COMPACTION_OUTPUT_FILE] = "BGC_OPEN_COMPACTION_OUTPUT_FILE";
		message[BGC_FINISH_COMPACTION_OUTPUT_FILE] = "BGC_FINISH_COMPACTION_OUTPUT_FILE";
		message[BGC_COLLECT_STATS] = "BGC_COLLECT_STATS";
		message[BGC_GET_LOCK_BEFORE_INSTALL] = "BGC_GET_LOCK_BEFORE_INSTALL";
		message[BGC_INSTALL_COMPACTION_RESULTS] = "BGC_INSTALL_COMPACTION_RESULTS";
		message[GET_OVERALL_TIME] = "GET_OVERALL_TIME";
		message[GET_TIME_TO_GET_MUTEX] = "GET_TIME_TO_GET_MUTEX";
		message[GET_TIME_TO_REF_MEMS] = "GET_TIME_TO_REF_MEMS";
		message[GET_TIME_TO_CHECK_MEM_IMM] = "GET_TIME_TO_CHECK_MEM_IMM";
		message[GET_TIME_TO_CHECK_VERSION] = "GET_TIME_TO_CHECK_VERSION";
		message[GET_TIME_TO_LOCK_MUTEX] = "GET_TIME_TO_LOCK_MUTEX";
		message[GET_TIME_TO_FINISH_UNREF] = "GET_TIME_TO_FINISH_UNREF";
		message[WRITE_SEQUENCE_WRITE_BEGIN_TOTAL] = "WRITE_SEQUENCE_WRITE_BEGIN_TOTAL";
		message[WRITE_SEQUENCE_WRITE_END_TOTAL] = "WRITE_SEQUENCE_WRITE_END_TOTAL";
		message[WRITE_SET_SEQUENCE_CREATE_NEW_BATCH] = "WRITE_SET_SEQUENCE_CREATE_NEW_BATCH";
		message[WRITE_SET_GUARDS] = "WRITE_SET_GUARDS";
		message[WRITE_LOG_ADDRECORD] = "WRITE_LOG_ADDRECORD";
		message[WRITE_LOG_FILE_SYNC] = "WRITE_LOG_FILE_SYNC";
		message[WRITE_INSERT_INTO_VERSION] = "WRITE_INSERT_INTO_VERSION";
		message[WRITE_OVERALL_TIME] = "WRITE_OVERALL_TIME";
		message[SWB_INIT_MUTEX] = "SWB_INIT_MUTEX";
		message[SWB_INIT_MEMTABLES] = "SWB_INIT_MEMTABLES";
		message[SWB_SET_LOG_DETAILS] = "SWB_SET_LOG_DETAILS";
		message[SWB_ENQUEUE_MEM] = "SWB_ENQUEUE_MEM";
		message[SWB_SET_TAIL] = "SWB_SET_TAIL";
		message[SWB_SYNC_AND_FETCH] = "SWB_SYNC_AND_FETCH";
		message[SWB_UNLOCK_MUTEX] = "SWB_UNLOCK_MUTEX";
		message[SWE_LOCK_MUTEX] = "SWE_LOCK_MUTEX";
		message[SWE_UNLOCK_MUTEX] = "SWE_UNLOCK_MUTEX";
		message[SWE_SET_PREV] = "SWE_SET_PREV";
		message[SWE_SET_NEXT] = "SWE_SET_NEXT";
		message[SWE_LOCK_WRITERS_MUTEX] = "SWE_LOCK_WRITERS_MUTEX";
		message[SWE_UNLOCK_WRITERS_MUTEX] = "SWE_UNLOCK_WRITERS_MUTEX";
		message[SWE_SET_IMM] = "SWE_SET_IMM";
		message[SWE_SLEEP] = "SWE_SLEEP";
		message[GET_FIND_GUARD] = "GET_FIND_GUARD";
		message[GET_FIND_GUARD_PARSE_INTERNAL_KEY] = "GET_FIND_GUARD_PARSE_INTERNAL_KEY";
		message[GET_CLEAR_TMP2_SET_G] = "GET_CLEAR_TMP2_SET_G";
		message[GET_CHECK_SENTINEL_FILES] = "GET_CHECK_SENTINEL_FILES";
		message[GET_SORT_SENTINEL_FILES] = "GET_SORT_SENTINEL_FILES";
		message[GET_CHECK_GUARD_FILES] = "GET_CHECK_GUARD_FILES";
		message[GET_SORT_GUARD_FILES] = "GET_SORT_GUARD_FILES";
		message[GET_FIND_LIST_OF_FILES] = "GET_FIND_LIST_OF_FILES";
		message[GET_TABLE_CACHE_GET] = "GET_TABLE_CACHE_GET";
		message[GET_TABLE_CACHE_READ_DATA_BLOCK] = "GET_TABLE_CACHE_READ_DATA_BLOCK";
		message[MTC_SAVETO_ADD_FILES] = "MTC_SAVETO_ADD_FILES";
		message[MTC_SAVETO_ADD_GUARDS] = "MTC_SAVETO_ADD_GUARDS";
		message[MTC_SAVETO_ADD_COMPLETE_GUARDS] = "MTC_SAVETO_ADD_COMPLETE_GUARDS";
		message[MTC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET] = "MTC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET";
		message[MTC_SAVETO_POPULATE_FILES] = "MTC_SAVETO_POPULATE_FILES";
		message[MTC_LAA_APPLY_EDIT_TO_BUILDER] = "MTC_LAA_APPLY_EDIT_TO_BUILDER";
		message[MTC_LAA_SAVETO] = "MTC_LAA_SAVETO";
		message[MTC_LAA_FINALIZE] = "MTC_LAA_FINALIZE";
		message[MTC_LAA_CREATE_MANIFEST_LOG_WRITER] = "MTC_LAA_CREATE_MANIFEST_LOG_WRITER";
		message[MTC_LAA_COMPLETE_WRITE_SNAPSHOT] = "MTC_LAA_COMPLETE_WRITE_SNAPSHOT";
		message[MTC_LAA_ENCODE_EDIT] = "MTC_LAA_ENCODE_EDIT";
		message[MTC_LAA_ADD_RECORD_TO_DESC_LOG] = "MTC_LAA_ADD_RECORD_TO_DESC_LOG";
		message[MTC_LAA_SYNC_MANIFEST_LOG_WRITE] = "MTC_LAA_SYNC_MANIFEST_LOG_WRITE";
		message[MTC_LAA_GET_DELTA_COMPLETE_GUARDS] = "MTC_LAA_GET_DELTA_COMPLETE_GUARDS";
		message[MTC_LAA_APPEND_VERSION] = "MTC_LAA_APPEND_VERSION";
		message[MTC_LAA_SYNC_COMPLETE_GUARDS] = "MTC_LAA_SYNC_COMPLETE_GUARDS";
		message[BGC_SAVETO_ADD_FILES] = "BGC_SAVETO_ADD_FILES";
		message[BGC_SAVETO_ADD_GUARDS] = "BGC_SAVETO_ADD_GUARDS";
		message[BGC_SAVETO_ADD_COMPLETE_GUARDS] = "BGC_SAVETO_ADD_COMPLETE_GUARDS";
		message[BGC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET] = "BGC_SAVETO_ADD_COMPLETE_GUARDS_TO_GUARDSET";
		message[BGC_SAVETO_POPULATE_FILES] = "BGC_SAVETO_POPULATE_FILES";
		message[BGC_LAA_APPLY_EDIT_TO_BUILDER] = "BGC_LAA_APPLY_EDIT_TO_BUILDER";
		message[BGC_LAA_SAVETO] = "BGC_LAA_SAVETO";
		message[BGC_LAA_FINALIZE] = "BGC_LAA_FINALIZE";
		message[BGC_LAA_CREATE_MANIFEST_LOG_WRITER] = "BGC_LAA_CREATE_MANIFEST_LOG_WRITER";
		message[BGC_LAA_COMPLETE_WRITE_SNAPSHOT] = "BGC_LAA_COMPLETE_WRITE_SNAPSHOT";
		message[BGC_LAA_ENCODE_EDIT] = "BGC_LAA_ENCODE_EDIT";
		message[BGC_LAA_ADD_RECORD_TO_DESC_LOG] = "BGC_LAA_ADD_RECORD_TO_DESC_LOG";
		message[BGC_LAA_SYNC_MANIFEST_LOG_WRITE] = "BGC_LAA_SYNC_MANIFEST_LOG_WRITE";
		message[BGC_LAA_GET_DELTA_COMPLETE_GUARDS] = "BGC_LAA_GET_DELTA_COMPLETE_GUARDS";
		message[BGC_LAA_APPEND_VERSION] = "BGC_LAA_APPEND_VERSION";
		message[BGC_LAA_SYNC_COMPLETE_GUARDS] = "BGC_LAA_SYNC_COMPLETE_GUARDS";
		message[MTC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC] = "MTC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC";
		message[BGC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC] = "BGC_LAA_GET_LOCK_AFTER_MANIFEST_SYNC";
		message[MTC_LAA_WAIT_FOR_LIVE_BACKUP] = "MTC_LAA_WAIT_FOR_LIVE_BACKUP";
		message[BGC_LAA_WAIT_FOR_LIVE_BACKUP] = "BGC_LAA_WAIT_FOR_LIVE_BACKUP";
		message[GET_TABLE_CACHE_FIND_TABLE] = "GET_TABLE_CACHE_FIND_TABLE";
		message[GET_TABLE_CACHE_GET_FROM_DISK] = "GET_TABLE_CACHE_GET_FROM_DISK";
		message[GET_TABLE_CACHE_GET_NEW_RANDOM_ACCESS_FILE] = "GET_TABLE_CACHE_GET_NEW_RANDOM_ACCESS_FILE",
		message[GET_TABLE_CACHE_GET_TABLE_OPEN] = "GET_TABLE_CACHE_GET_TABLE_OPEN",
		message[GET_TABLE_CACHE_GET_TABLE_OPEN_FOOTER_READ] = "GET_TABLE_CACHE_GET_TABLE_OPEN_FOOTER_READ",
		message[GET_TABLE_CACHE_GET_TABLE_OPEN_INDEX_BLOCK_READ] = "GET_TABLE_CACHE_GET_TABLE_OPEN_INDEX_BLOCK_READ",
		message[GET_TABLE_CACHE_GET_TABLE_OPEN_READ_META] = "GET_TABLE_CACHE_GET_TABLE_OPEN_READ_META",
		message[GET_TABLE_CACHE_GET_INSERT_INTO_CACHE] = "GET_TABLE_CACHE_GET_INSERT_INTO_CACHE",
		message[GET_TABLE_CACHE_INDEX_ITER_SEEK] = "GET_TABLE_CACHE_INDEX_ITER_SEEK";
		message[GET_TABLE_CACHE_FILTER_CHECK] = "GET_TABLE_CACHE_FILTER_CHECK";
		message[GET_TABLE_CACHE_INTERNAL_GET] = "GET_TABLE_CACHE_INTERNAL_GET";
		message[GET_TABLE_CACHE_CREATE_THREAD] = "GET_TABLE_CACHE_CREATE_THREAD";
		message[GET_TABLE_CACHE_NUM_THREADS_SIGNALLED] = "GET_TABLE_CACHE_NUM_THREADS_SIGNALLED";
		message[GET_TABLE_CACHE_NUM_DIRECT_CALLS] = "GET_TABLE_CACHE_NUM_DIRECT_CALLS";
		message[GET_TABLE_CACHE_SIGNAL_READ_THREADS] = "GET_TABLE_CACHE_SIGNAL_READ_THREADS";
		message[GET_TABLE_CACHE_WAIT_FOR_READ_THREADS] = "GET_TABLE_CACHE_WAIT_FOR_READ_THREADS";
		message[GET_FILE_LEVEL_FILTER_CHECK] = "GET_FILE_LEVEL_FILTER_CHECK";
		message[SEEK_NUM_TABLE_ITERATOR] = "SEEK_NUM_TABLE_ITERATOR";
		message[SEEK_GUARD_NUM_ITERATOR] = "SEEK_GUARD_NUM_ITERATOR";
		message[SEEK_PARALLEL_TOTAL] = "SEEK_PARALLEL_TOTAL";
		message[SEEK_SEQUENTIAL_TOTAL] = "SEEK_SEQUENTIAL_TOTAL";
		message[SEEK_SEQUENTIAL_FILES] = "SEEK_SEQUENTIAL_FILES";
		message[SEEK_PARALLEL_ASSIGN_THREADS] = "SEEK_PARALLEL_ASSIGN_THREADS";
		message[SEEK_PARALLEL_SIGNAL_THREADS] = "SEEK_PARALLEL_SIGNAL_THREADS";
		message[SEEK_PARALLEL_WAIT_FOR_THREADS] = "SEEK_PARALLEL_WAIT_FOR_THREADS";
		message[SEEK_TITERATOR_PARALLEL_TOTAL] = "SEEK_TITERATOR_PARALLEL_TOTAL";
		message[SEEK_TITERATOR_SEQUENTIAL_TOTAL] = "SEEK_TITERATOR_SEQUENTIAL_TOTAL";
		message[SEEK_TITERATOR_SEQUENTIAL_FILES] = "SEEK_TITERATOR_SEQUENTIAL_FILES";
		message[SEEK_TITERATOR_PARALLEL_ASSIGN_THREADS] = "SEEK_TITERATOR_PARALLEL_ASSIGN_THREADS";
		message[SEEK_TITERATOR_PARALLEL_SIGNAL_THREADS] = "SEEK_TITERATOR_PARALLEL_SIGNAL_THREADS";
		message[SEEK_TITERATOR_PARALLEL_WAIT_FOR_THREADS] = "SEEK_TITERATOR_PARALLEL_WAIT_FOR_THREADS";
		message[SEEK_REINIT] = "SEEK_REINIT";
		message[SEEK_REINIT_FILE_LEVEL] = "SEEK_REINIT_FILE_LEVEL";
		message[SEEK_NEXT] = "SEEK_NEXT";
		message[SEEK_NEXT_FILE_LEVEL] = "SEEK_NEXT_FILE_LEVEL";
		message[SEEK_NEXT_POP] = "SEEK_NEXT_POP";
		message[SEEK_NEXT_PUSH] = "SEEK_NEXT_PUSH";
		message[SEEK_NEXT_CURRENT_NEXT] = "SEEK_NEXT_CURRENT_NEXT";
		message[SEEK_NEXT_POP_FILE_LEVEL] = "SEEK_NEXT_POP_FILE_LEVEL";
		message[SEEK_NEXT_PUSH_FILE_LEVEL] = "SEEK_NEXT_PUSH_FILE_LEVEL";
		message[SEEK_NEXT_CURRENT_NEXT_FILE_LEVEL] = "SEEK_NEXT_CURRENT_NEXT_FILE_LEVEL";
		message[SEEK_THREAD_TOTAL_ACTIVE] = "SEEK_THREAD_TOTAL_ACTIVE";
		message[SEEK_THREAD_ITER_SEEK] = "SEEK_THREAD_ITER_SEEK";
		message[SEEK_BLOCK_READER_TOTAL] = "SEEK_BLOCK_READER_TOTAL",
		message[SEEK_BLOCK_READER_READ_BLOCK] = "SEEK_BLOCK_READER_READ_BLOCK",

		clear();
	}

	void StartTimer(TimerStep step) {
		micros_array[step] = Env::Default()->NowMicros();
	}

	void Record(TimerStep step) {
		assert(micros_array[step] != 0);
		timer_micros[step] += Env::Default()->NowMicros() - micros_array[step];
		timer_count[step]++;
	}

	void Record(TimerStep step, uint64_t additional_count) {
		assert(micros_array[step] != 0);
		timer_micros[step] += Env::Default()->NowMicros() - micros_array[step];
		timer_count[step]++;
		timer_count_additional[step] += additional_count;
	}

	void clear() {
		for (int i = BEGIN; i < END; i++) {
			micros_array[i] = timer_micros[i] = timer_count[i] = timer_count_additional[i] = 0;
		}
	}

	std::string DebugString() {
		std::string result;
		for (int i = BEGIN; i < END; i++) {
			if (timer_count[i] > 0) {
				result.append(message[i])
						.append(": timer_micros: ");
				AppendNumberTo(&result, timer_micros[i]);
				result.append(" timer_count: ");
				AppendNumberTo(&result, timer_count[i]);
				result.append(" timer_count_additional: ");
				AppendNumberTo(&result, timer_count_additional[i]);
				result.append("\n");
			}
		}
		return result;
	}

	void AppendTimerInfo(Timer* timer) {
		if (timer == NULL) {
			return;
		}
		for (int i = BEGIN; i < END; i++) {
			if (timer->timer_count[i] > 0) {
				timer_count[i] += timer->timer_count[i];
				timer_count_additional[i] += timer->timer_count_additional[i];
				timer_micros[i] += timer->timer_micros[i];
			}
		}
	}
};

}


#endif /* UTIL_TIMER_H_ */