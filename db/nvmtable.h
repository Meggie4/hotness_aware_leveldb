/*************************************************************************
	> File Name: nvmtable.h
	> Author: Meggie
	> Mail: 1224642332@qq.com 
	> Created Time: Sat 19 Jan 2019 04:48:08 PM CST
 ************************************************************************/
#ifndef STORAGE_LEVELDB_DB_NVMTABLE_H_
#define STORAGE_LEVELDB_DB_NVMTABLE_H_

#include <string>
#include <assert.h>
#include <vector>
#include "leveldb/db.h"
#include "db/dbformat.h"
#include "db/nvm_skiplist.h"
#include "db/chunklog.h"
#include "util/arena.h"
#include "port/port.h"
#include "util/hash.h"
#include "table/merger.h"

namespace leveldb{
class InternalKeyComparator;
class chunkTableIterator;
class chunkTable{
    public:
        chunkTable(const InternalKeyComparator& comparator, 
                ArenaNVM* arena, chunkLog* cklog, bool recovery = false);
        ~chunkTable();
        void Add(const char* kvitem);
        bool Get(const Slice& key, std::string* value, Status* s, SequenceNumber sequence);
        bool Contains(const Slice& user_key, SequenceNumber sequence);
        size_t getKeyNum()const {return table_.GetNodeNum();} 
        Iterator* NewIterator();
        size_t ApproximateLogNVMUsage() {return cklog_->NVMUsage(); };
        size_t ApproximateIndexNVMUsage() {return arena_->MemoryUsage(); };
        void Ref(){++refs_;}
        void Unref(){
            --refs_;
            assert(refs_ >= 0);
            if(refs_ <= 0){
                delete this;
            }
        }
        void* operator new(std::size_t sz){
            return malloc(sz);
        }
        void operator delete(void* ptr){
            free(ptr);
        }
        void* operator new[](std::size_t sz){
            return malloc(sz);
        }
        ////(TODO)move to private, to debug
        //chunkLog* cklog_;
        //typedef NVMSkipList<const char*, KeyComparator> Table;
        //Table table_;
    private:
        struct KeyComparator{
            const InternalKeyComparator comparator;
            explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) { }
            int operator()(const char* a, const char* b) const;
            int user_compare(const char* a, const char* b) const;
        };

        friend class chunkTableIterator;
        KeyComparator comparator_;
        ArenaNVM* arena_;
        chunkLog* cklog_;
        typedef NVMSkipList<const char*, KeyComparator> Table;
        Table table_;
        int refs_;

        chunkTable(const chunkTable&);
        void operator=(const chunkTable&);
};

static const int kNumChunkTableBits = 3;
static const int kNumChunkTable = 1 << kNumChunkTableBits;
class NVMTable {
    public:
        NVMTable(const InternalKeyComparator& comparator, 
            std::vector<ArenaNVM*>& arenas,
            std::vector<chunkLog*>& cklogs,  
            bool recovery);
        void Add(const char* kvitem, const Slice& key);
        bool Get(const Slice& key, std::string* value, Status* s, SequenceNumber sequence);
        bool Contains(const Slice& user_key, SequenceNumber sequence);
        void CheckAndAddToCompactionList(std::vector<chunkTable*>& toCompactionList, 
                size_t index_thresh,
                size_t log_thresh);
        Iterator* NewIterator(); 
        Iterator* getchunkTableIterator(int index);
        void PrintInfo(); 
        void Ref(){++refs_;}
        void Unref(){
            --refs_;
            assert(refs_ >= 0);
            if(refs_ <= 0){
                delete this;
            }
        }
    private:
        ~NVMTable();
        chunkTable* cktables_[kNumChunkTable];
        const InternalKeyComparator* comparator_;
        int refs_;
        static inline uint32_t chunkTableHash(const Slice& key){
            return Hash(key.data(), key.size(), 0);
        }
        static uint32_t chunkTableIndex(uint32_t hash){
           return hash >> (32 - kNumChunkTableBits);
        }
        
        NVMTable(const NVMTable&);
        void operator=(const NVMTable&);
};
}

#endif
