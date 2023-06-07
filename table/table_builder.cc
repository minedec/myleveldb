// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/table_builder.h"

#include <cassert>

#include "leveldb/comparator.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/options.h"
#include "table/block_builder.h"
#include "table/filter_block.h"
#include "table/format.h"
#include "util/coding.h"
#include "util/crc32c.h"

#include "db/db_impl.h"
#include "util/env_posix.cc"
#include <libpmem.h>

namespace leveldb {

struct TableBuilder::Rep {
  Rep(const Options& opt, WritableFile* f)
      : options(opt),
        index_block_options(opt),
        file(f),
        offset(0),
        data_block(&options),
        index_block(&index_block_options),
        num_entries(0),
        closed(false),
        filter_block(opt.filter_policy == nullptr
                         ? nullptr
                         : new FilterBlockBuilder(opt.filter_policy)),
        pending_index_entry(false) {
    index_block_options.block_restart_interval = 1;
    builder_buf.resize(MAX_LEN);
  }

  Options options;
  Options index_block_options;
  WritableFile* file;
  uint64_t offset;
  Status status;
  BlockBuilder data_block;
  BlockBuilder index_block;
  std::string last_key;
  int64_t num_entries;
  bool closed;  // Either Finish() or Abandon() has been called.
  FilterBlockBuilder* filter_block;

  // We do not emit the index entry for a block until we have seen the
  // first key for the next data block.  This allows us to use shorter
  // keys in the index block.  For example, consider a block boundary
  // between the keys "the quick brown fox" and "the who".  We can use
  // "the r" as the key for the index block entry since it is >= all
  // entries in the first block and < all entries in subsequent
  // blocks.
  //
  // Invariant: r->pending_index_entry is true only if data_block is empty.
  bool pending_index_entry;
  BlockHandle pending_handle;  // Handle to add to index block

  std::string compressed_output;

  // PMDB temporaily store insert key, when finish block, add block handle encoding
  std::vector<std::string> tmp_keys;
  // std::vector<std::string> tmpskeys;
  std::vector<uint64_t> tmp_seq;

  // PMDB add char array to append table content
  std::vector<char> builder_buf;
  uint64_t MAX_LEN = 1024 * 1024 * 4;
  uint64_t buf_pos = 0;
  std::string filename_;
  uint64_t filenumber_;
};

TableBuilder::TableBuilder(const Options& options, WritableFile* file)
    : rep_(new Rep(options, file)) {
  if (rep_->filter_block != nullptr) {
    rep_->filter_block->StartBlock(0);
  }
}

TableBuilder::~TableBuilder() {
  assert(rep_->closed);  // Catch errors where caller forgot to call Finish()
  delete rep_->filter_block;
  delete rep_;
}

Status TableBuilder::ChangeOptions(const Options& options) {
  // Note: if more fields are added to Options, update
  // this function to catch changes that should not be allowed to
  // change in the middle of building a Table.
  if (options.comparator != rep_->options.comparator) {
    return Status::InvalidArgument("changing comparator while building table");
  }

  // Note that any live BlockBuilders point to rep_->options and therefore
  // will automatically pick up the updated options.
  rep_->options = options;
  rep_->index_block_options = options;
  rep_->index_block_options.block_restart_interval = 1;
  return Status::OK();
}

void TableBuilder::Add(const Slice& key, const Slice& value) {
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;
  if (r->num_entries > 0) {
    assert(r->options.comparator->Compare(key, Slice(r->last_key)) > 0);
  }

  if (r->pending_index_entry) {
    assert(r->data_block.empty());
    r->options.comparator->FindShortestSeparator(&r->last_key, key);
    std::string handle_encoding;
    r->pending_handle.EncodeTo(&handle_encoding);
    r->index_block.Add(r->last_key, Slice(handle_encoding));  
    r->pending_index_entry = false;
  }

  if (r->filter_block != nullptr) {
    r->filter_block->AddKey(key);
  }

  // PMDB add key to tmp_keys
  // ParsedInternalKey ikey;
  ParsedInternalKey ikey;
  ParseInternalKey(key, &ikey);
  if(ikey.type == kTypeDeletion) {
#if DEBUG_PRINT
     printf("table_builder delete key %s\n", ikey.user_key.ToString().c_str());
#endif
    DBImpl::dbimpl_instance->mem_hashtable_->deleteKey(ikey.user_key);
  } else {
#if DEBUG_PRINT
    printf("table_builder #%ld, key %s push tmp_keys %p\n", r->filenumber_, ikey.user_key.ToString().c_str(),
      &r->tmp_keys);
    printf("data addr %p\n", ikey.user_key.data());
#endif
    MemHashTableValue* v = DBImpl::dbimpl_instance->mem_hashtable_->getTableValue(ikey.user_key);
    if(v == nullptr || (v != nullptr && v->sequence_number_ <= ikey.sequence)) {
      r->tmp_keys.push_back(ikey.user_key.ToString());
      // r->tmpskeys.push_back(ikey.user_key.ToString());
      r->tmp_seq.push_back(ikey.sequence);
    }
  }

  r->last_key.assign(key.data(), key.size());
  r->num_entries++;
  r->data_block.Add(key, value);
#if DEBUG_PRINT
  printf("table_builder #%ld, key %s add to data block, key type %s, key addr %p\n", r->filenumber_, ikey.user_key.ToString().c_str(),
    (ikey.type == kTypeValue)?"kTypeValue":"kTypeDeletion", &ikey);
#endif

  const size_t estimated_block_size = r->data_block.CurrentSizeEstimate();
  if (estimated_block_size >= r->options.block_size) {
#if DEBUG_PRINT
     printf("table_builder::Add user key %s trigger flush\n", ikey.user_key.ToString().c_str());
    printf("before flush()\n");
    for(int i = 0; i < r->tmp_keys.size(); i++) {
      printf("#%ld before flush tmp key %s, tmp_keys %p, key addr %p\n", r->filenumber_, r->tmp_keys[i].c_str(),
        &r->tmp_keys, &r->tmp_keys[i]);
      // printf("#%ld before flush tmp key s %s, tmp_keys %p, key addr %p\n", r->filenumber_, r->tmpskeys[i].c_str(),
      //   &r->tmpskeys, &r->tmpskeys[i]);
      printf("slice data addr %p\n", r->tmp_keys[i].data());
      // printf("string data addr %p\n", r->tmpskeys[i].data());
    }
#endif
    Flush();
  }
}

void TableBuilder::Flush() {
  Rep* r = rep_;
  assert(!r->closed);
  if (!ok()) return;
  if (r->data_block.empty()) return;
  assert(!r->pending_index_entry);
  WriteBlock(&r->data_block, &r->pending_handle);
  if (ok()) {
    // PMDB add handle index encoding
    std::string handle_encoding;
    r->pending_handle.EncodeTo(&handle_encoding);
    assert(r->tmp_keys.size() == r->tmp_seq.size());
    for(int i = 0; i < r->tmp_keys.size(); i++) {
      std::string ukey = r->tmp_keys[i];
      uint64_t seq = r->tmp_seq[i];
      Slice uskey(ukey);
      MemHashTableValue* v = DBImpl::dbimpl_instance->mem_hashtable_->getTableValue(uskey);
      if (v == nullptr) {
#if DEBUG_PRINT
         printf("table builder #%ld add new key %s\n", r->filenumber_, ukey.c_str());
#endif
        v = new MemHashTableValue();
      } else {
#if DEBUG_PRINT
        printf("table builder #%ld key %s exist\n", r->filenumber_, ukey.c_str());
#endif
      }
      v->on_change = true;
#if DEBUG_PRINT
      printf("table builder #%ld key %s on change true\n", r->filenumber_, ukey.c_str());
#endif
      v->block_handle_encoding_ = std::string(handle_encoding);
      v->sequence_number_ = seq;
#if DEBUG_PRINT
      BlockHandle bh;
      bh.DecodeFrom(new Slice(v->block_handle_encoding_));
       printf("table builder #%ld add hash block handle offset %ld, size %ld, key %s\n", r->filenumber_, bh.offset(), bh.size(), ukey.c_str());
#endif
      DBImpl::dbimpl_instance->mem_hashtable_->setValue(uskey, v);
    }
    r->tmp_keys.clear();
    // r->tmpskeys.clear();
    r->tmp_seq.clear();
    r->pending_index_entry = true;

    // PMDB if is manifest, do regular way
    if(PosixWritableFile::IsManifest(r->filename_)) {
      r->status = r->file->Flush();
    }
  }
  if (r->filter_block != nullptr) {
    r->filter_block->StartBlock(r->offset);
  }
}

void TableBuilder::WriteBlock(BlockBuilder* block, BlockHandle* handle) {
  // File format contains a sequence of blocks where each block has:
  //    block_data: uint8[n]
  //    type: uint8
  //    crc: uint32
  assert(ok());
  Rep* r = rep_;
  Slice raw = block->Finish();

  Slice block_contents;
  CompressionType type = r->options.compression;
  // TODO(postrelease): Support more compression options: zlib?
  switch (type) {
    case kNoCompression:
      block_contents = raw;
      break;

    case kSnappyCompression: {
      std::string* compressed = &r->compressed_output;
      if (port::Snappy_Compress(raw.data(), raw.size(), compressed) &&
          compressed->size() < raw.size() - (raw.size() / 8u)) {
        block_contents = *compressed;
      } else {
        // Snappy not supported, or compressed less than 12.5%, so just
        // store uncompressed form
        block_contents = raw;
        type = kNoCompression;
      }
      break;
    }

    case kZstdCompression: {
      std::string* compressed = &r->compressed_output;
      if (port::Zstd_Compress(r->options.zstd_compression_level, raw.data(),
                              raw.size(), compressed) &&
          compressed->size() < raw.size() - (raw.size() / 8u)) {
        block_contents = *compressed;
      } else {
        // Zstd not supported, or compressed less than 12.5%, so just
        // store uncompressed form
        block_contents = raw;
        type = kNoCompression;
      }
      break;
    }
  }
  WriteRawBlock(block_contents, type, handle);
  r->compressed_output.clear();
  block->Reset();
}

void TableBuilder::WriteRawBlock(const Slice& block_contents,
                                 CompressionType type, BlockHandle* handle) {
  Rep* r = rep_;
  handle->set_offset(r->offset);
  handle->set_size(block_contents.size());
  
  // PMDB if is manifest, do regular way
  if(PosixWritableFile::IsManifest(r->filename_)) {
    r->status = r->file->Append(block_contents);
  } else {
    // PMDB append block_contents to buf
    const char* data = block_contents.data();
    size_t size = block_contents.size();
    memcpy(&r->builder_buf[0] + r->buf_pos, data, size);
    r->status = Status::OK();
    r->buf_pos += size;
  }

  if (r->status.ok()) {
    char trailer[kBlockTrailerSize];
    trailer[0] = type;
    uint32_t crc = crc32c::Value(block_contents.data(), block_contents.size());
    crc = crc32c::Extend(crc, trailer, 1);  // Extend crc to cover block type
    EncodeFixed32(trailer + 1, crc32c::Mask(crc));
    
    // PMDB if is manifest, do regular way
    if(PosixWritableFile::IsManifest(r->filename_)) {
      r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
    } else {
      // PMDB append trailer to buf
      memcpy(&r->builder_buf[0] + r->buf_pos, trailer, kBlockTrailerSize);
      r->status = Status::OK();
      r->buf_pos += kBlockTrailerSize;
    }

    if (r->status.ok()) {
      r->offset += block_contents.size() + kBlockTrailerSize;
    }
  }
}

Status TableBuilder::status() const { return rep_->status; }

Status TableBuilder::Finish() {
  Rep* r = rep_;
  Flush(); // Flush the last data block
  assert(!r->closed);
  r->closed = true;

  BlockHandle filter_block_handle, metaindex_block_handle, index_block_handle;

  // Write filter block
  if (ok() && r->filter_block != nullptr) {
    WriteRawBlock(r->filter_block->Finish(), kNoCompression,
                  &filter_block_handle);
  }

  // Write metaindex block
  if (ok()) {
    BlockBuilder meta_index_block(&r->options);
    if (r->filter_block != nullptr) {
      // Add mapping from "filter.Name" to location of filter data
      std::string key = "filter.";
      key.append(r->options.filter_policy->Name());
      std::string handle_encoding;
      filter_block_handle.EncodeTo(&handle_encoding);
      meta_index_block.Add(key, handle_encoding);
    }

    // TODO(postrelease): Add stats and other meta blocks
    WriteBlock(&meta_index_block, &metaindex_block_handle);
  }

  // Write index block
  if (ok()) {
    if (r->pending_index_entry) {
      r->options.comparator->FindShortSuccessor(&r->last_key);
      std::string handle_encoding;
      r->pending_handle.EncodeTo(&handle_encoding);
      r->index_block.Add(r->last_key, Slice(handle_encoding));
      r->pending_index_entry = false;
    }
    WriteBlock(&r->index_block, &index_block_handle);
  }

  // Write footer
  if (ok()) {
    Footer footer;
    footer.set_metaindex_handle(metaindex_block_handle);
    footer.set_index_handle(index_block_handle);
    std::string footer_encoding;
    footer.EncodeTo(&footer_encoding);
    
    // PMDB if is manifest, do regular way
    if(PosixWritableFile::IsManifest(r->filename_)) {
      r->status = r->file->Append(footer_encoding);
    } else {
      //PMDB append footer
      const char* data = footer_encoding.data();
      size_t size = footer_encoding.size();
      memcpy(&r->builder_buf[0] + r->buf_pos, data, size);
      r->status = Status::OK();
      r->buf_pos += size;
    }

    if (r->status.ok()) {
      r->offset += footer_encoding.size();
    }
  }

  // PMDB use mmap to flush data to PM and truncate file size
  if(!PosixWritableFile::IsManifest(r->filename_)) {
    assert(r->buf_pos == r->offset);
    size_t mapped_len;
    int is_pmem;
    char* faddr;
    if((faddr = (char*)pmem_map_file(r->filename_.c_str(), r->buf_pos, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == nullptr) {
      std::cout << "pmem_map_file error\n";
    }
    pmem_memcpy(faddr, &r->builder_buf[0], r->buf_pos, 0);
    if(is_pmem) {
      pmem_persist(faddr, mapped_len);
    }
    pmem_unmap(faddr, r->buf_pos);
    
  }

  return r->status;
}

void TableBuilder::Abandon() {
  Rep* r = rep_;
  assert(!r->closed);
  r->closed = true;
}

uint64_t TableBuilder::NumEntries() const { return rep_->num_entries; }

uint64_t TableBuilder::FileSize() const { return rep_->offset; }

// PMDB set file name
void TableBuilder::setFileName(std::string filename) {
  rep_->filename_ = filename;
}

void TableBuilder::setFileNumber(uint64_t filenumber) {
  rep_->filenumber_ = filenumber;
}

}  // namespace leveldb
