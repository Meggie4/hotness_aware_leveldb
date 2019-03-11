#include "db/nvmtable.h"
#include "db/dbformat.h"
#include "leveldb/comparator.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"
#include "util/coding.h"
#include "util/debug.h"
#include "util/multi_bloomfilter.h"

#define BIT_BLOOM_SIZE (1 << 10) << 10
#define BIT_BLOOM_HASH 4
namespace leveldb{

    static Slice GetNVMLengthPrefixedSlice(const char* data) {
        uint32_t len;
        const char* p = data;
        p = GetVarint32Ptr(p, p + 5, &len);  // +5: we assume "p" is not corrupted
        //DEBUG_T("GetNVMLengthPrefixedSlice, user_len:%d\n", len);
        return Slice(p, len);
    }
    static const char* EncodeNVMKey(std::string* scratch, const Slice& target) {
        scratch->clear();
        PutVarint32(scratch, target.size());
        scratch->append(target.data(), target.size());
        return scratch->data();
    }

    int chunkTable::KeyComparator::operator()(const char* aptr, const char* bptr)
        const {
        // Internal keys are encoded as length-prefixed strings.
        Slice a = GetNVMLengthPrefixedSlice(aptr);
        Slice b = GetNVMLengthPrefixedSlice(bptr);
        return comparator.Compare(a, b);
    }

    int chunkTable::KeyComparator::user_compare(const char* aptr, const char* bptr) const
    {
        Slice a = GetNVMLengthPrefixedSlice(aptr);
        Slice b = GetNVMLengthPrefixedSlice(bptr);
        Slice user_a = Slice(a.data(), a.size() - 8);
        Slice user_b = Slice(b.data(), b.size() - 8);
        //DEBUG_T("user_a:%s,len:%d, user_b:%s,len:%d\n", user_a.ToString().c_str(),
          //      user_a.size(),user_b.ToString().c_str(),user_b.size());
        return comparator.user_comparator()->Compare(user_a, user_b);
    }

    class chunkTableIterator: public Iterator{
        public:
            explicit chunkTableIterator(chunkTable::Table* table, chunkLog* cklog) : 
                iter_(table, cklog), cklog_(cklog){  }
    
            virtual bool Valid() const { return iter_.Valid(); }
            virtual void Seek(const Slice& k) { iter_.Seek(EncodeNVMKey(&tmp_, k)); }
            virtual void SeekToFirst() { iter_.SeekToFirst(); }
            virtual void SeekToLast() { iter_.SeekToLast(); }
            virtual void Next() { iter_.Next(); }
            virtual void Prev() { iter_.Prev(); }
            virtual Slice key() const { return GetNVMLengthPrefixedSlice(cklog_->getKV(iter_.key_offset())); }
            virtual Slice value() const {
                Slice key_slice = GetNVMLengthPrefixedSlice(cklog_->getKV(iter_.key_offset()));
                return GetNVMLengthPrefixedSlice(key_slice.data() + key_slice.size());
            }

            virtual Status status() const { return Status::OK(); }

            void* operator new(std::size_t sz){
                return malloc(sz);
            }

            void* operator new[](std::size_t sz){
                return malloc(sz);
            }

            void operator delete(void* ptr){
                free(ptr);
            }
        private:
            chunkTable::Table::Iterator iter_;
            chunkLog* cklog_;
            std::string tmp_;
            
            chunkTableIterator(const chunkTableIterator&);
            void operator=(const chunkTableIterator&);
    };

    Iterator* chunkTable::NewIterator(){
        return new chunkTableIterator(&table_, cklog_);
    }

    chunkTable::chunkTable(const InternalKeyComparator& comparator, 
            ArenaNVM* arena, chunkLog* cklog, bool recovery):
        comparator_(comparator),
        arena_(arena),
        cklog_(cklog),
        table_(comparator_, arena, cklog_, recovery),
        refs_(0),
        bbf_(new BitBloomFilter(BIT_BLOOM_HASH, BIT_BLOOM_SIZE)){
    }
    chunkTable::~chunkTable(){
        delete bbf_;
        delete arena_;
        delete cklog_;
        assert(refs_ == 0);
    }
    void chunkTable::Add(const char* kvitem){
        const char* key_ptr;
        uint32_t key_length;
        size_t kv_length;
        uint32_t key_length1;
        const char* key_ptr1 = GetVarint32Ptr(kvitem, kvitem + 5, &key_length1);
        //DEBUG_T("before Add, user_key:%s, len:%d\n", Slice(key_ptr1, key_length1 - 8).ToString().c_str(), 
         //       key_length1 - 8);
        key_ptr = GetKVLength(kvitem, &key_length, &kv_length);
        //Slice nvmkey = Slice(key_ptr, key_length); 
        
        uint64_t tag = DecodeFixed64(key_ptr + key_length - 8);
        //ValueType type = static_cast<ValueType>(tag & 0xff);
        SequenceNumber s = tag >> 8;
        //DEBUG_T("before add to chunklog\n");
        const char* kv_offset = reinterpret_cast<char*>(cklog_->insert(kvitem, kv_length));
        //DEBUG_T("chunktable,add kvoffset：%p\n", kv_offset);
        table_.Add(kvitem, kv_offset, s); 
        //DEBUG_T("after add to chunktable\n");
    }

    bool chunkTable::Get(const Slice& user_key, std::string* value, Status* s, SequenceNumber sequence){
        Table::Iterator iter(&table_, cklog_);
        ///////create lookupkey 
        LookupKey lkey(user_key, sequence);
        Slice memkey = lkey.memtable_key();
        iter.Seek(memkey.data());
        //uint32_t userkey_length;
        //const char* userkey_ptr = GetVarint32Ptr(key.data(), key.data() + 5, &userkey_length);
        //Slice user_key = Slice(userkey_ptr, userkey_length - 8);
        if(iter.Valid()){
            const char* key_offset = iter.key_offset();
            //DEBUG_T("get, key_offset%p\n", key_offset);
            const char* entry = cklog_->getKV(key_offset);
            uint32_t key_length;
            const char* key_ptr = GetVarint32Ptr(entry, entry+5, &key_length);
            //DEBUG_T("user_key:%s\n", Slice(key_ptr, key_length - 8).ToString().c_str());
            if(comparator_.comparator.user_comparator()->Compare(
                        Slice(key_ptr, key_length - 8), user_key) == 0){
                //DEBUG_T("user_key,is equal\n");
                const uint64_t tag = DecodeFixed64(key_ptr + key_length - 8);
                switch(static_cast<ValueType>(tag & 0xff)){
                    case kTypeValue: {
                        //DEBUG_T("get, kTypeValue\n");
                        Slice v = GetNVMLengthPrefixedSlice(key_ptr + key_length);
                        value->assign(v.data(), v.size());
                        return true;
                    }
                    case kTypeDeletion: {
                        //DEBUG_T("get, kTypeDeletion\n");
                        *s = Status::NotFound(Slice());
                        return true;
                    }
                }
            }
        }
        return false;
    }
   
    void chunkTable::AddPredictIndex(const Slice& user_key){
        bbf_->Add(user_key);
    }

    bool chunkTable::CheckPredictIndex(const Slice& user_key){
        return bbf_->Query(user_key);
    }

    bool chunkTable::Contains(const Slice& user_key, SequenceNumber sequence){
        LookupKey lkey(user_key, sequence);
        Slice memkey = lkey.memtable_key();
        return table_.Contains(memkey.data());
    }


    NVMTable::NVMTable(const InternalKeyComparator& comparator, 
            std::vector<ArenaNVM*>& arenas,
            std::vector<chunkLog*>& cklogs,
            bool recovery):
            refs_(0){
                comparator_ = &comparator;
                assert(arenas.size() == kNumChunkTable);
                for(int i = 0; i < kNumChunkTable; i++){
                    chunkTable* ckTbl = new chunkTable(comparator, arenas[i], cklogs[i], recovery);
                    cktables_[i] = ckTbl;
                }
    }

    NVMTable::NVMTable(const InternalKeyComparator& comparator){
        comparator_ = &comparator;
        for(int i = 0; i < kNumChunkTable; i++){
            cktables_[i] = NULL;
        }
    } 

    NVMTable::~NVMTable(){
        for(int i = 0; i < kNumChunkTable; i++){
            if(cktables_[i])
                delete cktables_[i];
        }
    }

    void NVMTable::Add(const char* kvitem, const Slice& key){
       const uint32_t hash = chunkTableHash(key);
       //DEBUG_T("add, index:%d\n", chunkTableIndex(hash));
       cktables_[chunkTableIndex(hash)]->Add(kvitem);
       cktables_[chunkTableIndex(hash)]->AddPredictIndex(key);
    }

    bool NVMTable::Get(const Slice& key, std::string* value, Status* s, SequenceNumber sequence){
       const uint32_t hash = chunkTableHash(key);
       //DEBUG_T("get, index:%d\n", chunkTableIndex(hash));
       chunkTable* cktbl = cktables_[chunkTableIndex(hash)];
       if(!cktbl->CheckPredictIndex(key))
           return false;
       else 
           return cktbl->Get(key, value, s, sequence);
    }

    bool NVMTable::Contains(const Slice& user_key, SequenceNumber sequence){
       const uint32_t hash = chunkTableHash(user_key);
       chunkTable* cktbl = cktables_[chunkTableIndex(hash)];
       if(!cktbl->CheckPredictIndex(user_key))
           return false;
       else 
           return cktbl->Contains(user_key, sequence);
    }
    
    bool NVMTable::MaybeContains(const Slice& user_key){
       const uint32_t hash = chunkTableHash(user_key);
       chunkTable* cktbl = cktables_[chunkTableIndex(hash)];
       if(!cktbl->CheckPredictIndex(user_key))
           return false;
       else 
           return true;
    }


    bool NVMTable::CheckAndAddToCompactionList(std::vector<chunkTable*>& toCompactionList,
                                               std::vector<int>& need_updates,
                                               size_t index_thresh,
                                               size_t log_thresh){
        for(int i = 0; i < kNumChunkTable; i++){
            //if(cktables_[i])
                //DEBUG_T("this chunktable is not null\n");
            if(cktables_[i] && (cktables_[i]->ApproximateIndexNVMUsage() >= index_thresh ||
                    cktables_[i]->ApproximateLogNVMUsage() >= log_thresh)){
                toCompactionList.push_back(cktables_[i]);
                need_updates.push_back(i);
            }
        }
        if(toCompactionList.empty())
            return false;
        else 
            return true;
    }

    void NVMTable::AllocateForCompactionChunkTable(std::vector<int>& need_updates, 
            std::map<int, std::pair<ArenaNVM*, chunkLog*>>& allocation){
        int size = need_updates.size();
        std::map<int, std::pair<ArenaNVM*, chunkLog*>>::iterator iter;
        for(int i = 0; i < size; i++){
            int index = need_updates[i];
            iter = allocation.find(index);
            chunkTable* cktbl =  new chunkTable(*comparator_, iter->second.first, 
                            iter->second.second, false);
            if(cktables_[index]){
                delete cktables_[index];
                //DEBUG_T("delete chunk%d, and allocate another\n", index);
            }
            /*else 
                DEBUG_T("allocate new chunk%d\n", index);*/
            cktables_[index] = cktbl;
        } 
    }

    void NVMTable::AddAllToCompactionList(std::vector<chunkTable*>& toCompactionList){
        for(int i = 0; i < kNumChunkTable; i++){
                toCompactionList.push_back(cktables_[i]);
        } 
    }
    
    Iterator* NVMTable::NewIterator(){
        std::vector<Iterator*> list;
        for(int i = 0; i < kNumChunkTable; i++){
            list.push_back(cktables_[i]->NewIterator());
        }
        return NewMergingIterator(comparator_, &list[0], list.size());
    }

    Iterator* NVMTable::GetMergeIterator(std::vector<chunkTable*>& toCompactionList){
        std::vector<Iterator*> list;
        for(int i = 0; i < toCompactionList.size(); i++){
            list.push_back(toCompactionList[i]->NewIterator());
        }
        return NewMergingIterator(comparator_, &list[0], list.size());
    }

    Iterator* NVMTable::getchunkTableIterator(int index){
        return cktables_[index]->NewIterator();
    } 

    void NVMTable::PrintInfo(){
        DEBUG_T("---------------PRINT_NVMTABLE-------------------\n");
        for(int i = 0; i < kNumChunkTable; i++){
            DEBUG_T("index:%d, indexUsage:%zu, logUsage:%zu, numofEntries:%d\n", 
                    i, cktables_[i]->ApproximateIndexNVMUsage(),
                    cktables_[i]->ApproximateLogNVMUsage(), cktables_[i]->getKeyNum());
        }
        DEBUG_T("-------------END PRINT_NVMTABLE-----------------\n");
    }
    
}
