// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/builder.h"
#include "db/filename.h"
#include "db/dbformat.h"
#include "db/table_cache.h"
#include "db/version_edit.h"
////////////////////meggie
#include "db/version_set.h"
#include "db/nvmtable.h"
#include "util/multi_bloomfilter.h"
////////////////////meggie
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"

namespace leveldb {


Status BuildTable(const std::string& dbname,
                  Env* env,
                  const Options& options,
                  TableCache* table_cache,
                  Iterator* iter,
                  FileMetaData* meta, 
                  ////////////meggie
                  NVMTable* nvmtbl,
                  MultiHotBloomFilter* hot_bf
                  ////////////meggie
                  ) {
  Status s;
  meta->file_size = 0;
  iter->SeekToFirst();
  
  ///////////////meggie
  int nvm_num = 0, sst_num = 0;
  bool first_entry = true;
  int hot_num = 0;
  int not_in_num = 0;
  int i = 0;
  bool has_current_user_key = false;
  std::string current_user_key;
  SequenceNumber last_sequence_for_key = kMaxSequenceNumber;
  int count = 0;
  int drop_count = 0;
  ///////////////meggie
  std::string fname = TableFileName(dbname, meta->number);
  if (iter->Valid()) {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    TableBuilder* builder = new TableBuilder(options, file);
    for (; iter->Valid(); iter->Next()) {
      Slice key = iter->key();
      //////////meggie
      count++;
      bool drop = false;
      if(hot_bf && nvmtbl){
          i++;
          Slice user_key(key.data(), key.size() - 8);
          if(!has_current_user_key ||
                  nvmtbl->GetComparator()->user_comparator()->Compare(user_key, 
                      Slice(current_user_key)) != 0){
              current_user_key.assign(user_key.data(), user_key.size());
              has_current_user_key = true;
              last_sequence_for_key = kMaxSequenceNumber;
          }
          if(last_sequence_for_key < kMaxSequenceNumber){
              drop = true;
              drop_count++;
          }
          
          last_sequence_for_key =  DecodeFixed64(key.data() + key.size() - 8) >> 8;
          if(!drop){ 
              if(hot_bf->CheckHot(user_key)){
                 //DEBUG_T("user_key:%s is hot\n", user_key.ToString().c_str());
                 hot_num++;
              }
              if(!nvmtbl->MaybeContains(user_key)){
                  not_in_num++;
              }
              else{
                 ;//DEBUG_T("user_key:%s is in nvmtable\n", user_key.ToString().c_str());
              }
              //DEBUG_T("user_key:%s\n", user_key.ToString().c_str());
              if(hot_bf->CheckHot(user_key) || 
                   nvmtbl->MaybeContains(user_key)){
                DEBUG_T("add to nvmtable, user_key:%s\n", user_key.ToString().c_str());
                nvmtbl->Add(iter->GetNodeKey(), user_key);
                nvm_num++;
              }
              else{
                if(first_entry){
                    meta->smallest.DecodeFrom(iter->key());
                    first_entry = false;
                }
                meta->largest.DecodeFrom(key);
                builder->Add(key, iter->value());
                //DEBUG_T("after add to sstable\n");
                sst_num++;
              }
          }
          else 
              ;//DEBUG_T("it will be dropped\n");
      }
      else{
          if(first_entry){
            meta->smallest.DecodeFrom(iter->key());
            first_entry = false;
          }
          meta->largest.DecodeFrom(key);
          builder->Add(key, iter->value());
          sst_num++;
      }
      //////////meggie
    }

    /////////////////meggie
    nvmtbl->PrintInfo();
    //DEBUG_T("i:%d, not_in_num:%d, hot_num:%d\n", i, not_in_num, hot_num);
    DEBUG_T("after buildtable, req_num_of_bf:%lu, count:%d, drop_count:%d, nvm_num:%d, sst_num:%d\n", hot_bf->GetReqNum(), count, drop_count, nvm_num, sst_num);
    /////////////////meggie 
    
    // Finish and check for builder errors
    
    ///////////////meggie
    if(sst_num <= 0){
        delete builder; 
        delete file;
        env->DeleteFile(fname);
        meta->file_size = 0;
        return s;
    }
    ///////////////meggie
    
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      assert(meta->file_size > 0);
    }
    delete builder;

    // Finish and check for file errors
    if (s.ok()) {
      s = file->Sync();
    }
    if (s.ok()) {
      s = file->Close();
    }
    delete file;
    file = nullptr;

    if (s.ok()) {
      // Verify that the table is usable
      Iterator* it = table_cache->NewIterator(ReadOptions(),
                                              meta->number,
                                              meta->file_size);
      s = it->status();
      delete it;
    }
  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(fname);
  }
  return s;
}
}  // namespace leveldb
