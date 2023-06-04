// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/log_writer.h"

#include <cstdint>

#include "leveldb/env.h"
#include "util/coding.h"
#include "util/crc32c.h"

#include <libpmem.h>
#include <iostream>

namespace leveldb {
namespace log {

static void InitTypeCrc(uint32_t* type_crc) {
  for (int i = 0; i <= kMaxRecordType; i++) {
    char t = static_cast<char>(i);
    type_crc[i] = crc32c::Value(&t, 1);
  }
}

Writer::Writer(WritableFile* dest) : dest_(dest), block_offset_(0) {
  InitTypeCrc(type_crc_);
}

Writer::Writer(WritableFile* dest, uint64_t dest_length)
    : dest_(dest), block_offset_(dest_length % kBlockSize) {
  InitTypeCrc(type_crc_);
}

Writer::~Writer() = default;

Status Writer::AddRecord(const Slice& slice) {
  if(laddr == nullptr) {
    // mmap to create a file
    if((laddr = (char*)pmem_map_file(filename_.c_str(), MAX_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == nullptr) {
      std::cout << "log_writer pmem_map_file error\n";
    }
  }
  const char* ptr = slice.data();
  size_t left = slice.size();

  // Fragment the record if necessary and emit it.  Note that if slice
  // is empty, we still want to iterate once to emit a single
  // zero-length record
  Status s;
  bool begin = true;
  do {
    const int leftover = kBlockSize - block_offset_;
    assert(leftover >= 0);
    if (leftover < kHeaderSize) {
      // Switch to a new block
      if (leftover > 0) {
        // Fill the trailer (literal below relies on kHeaderSize being 7)
        static_assert(kHeaderSize == 7, "");
        // dest_->Append(Slice("\x00\x00\x00\x00\x00\x00", leftover));

        // PMDB append padding use memcpy
        pmem_memcpy(laddr + loffset_, "\x00\x00\x00\x00\x00\x00", leftover, 0);
        loffset_ += leftover;
      }
      block_offset_ = 0;
    }

    // Invariant: we never leave < kHeaderSize bytes in a block.
    assert(kBlockSize - block_offset_ - kHeaderSize >= 0);

    const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
    const size_t fragment_length = (left < avail) ? left : avail;

    RecordType type;
    const bool end = (left == fragment_length);
    if (begin && end) {
      type = kFullType;
    } else if (begin) {
      type = kFirstType;
    } else if (end) {
      type = kLastType;
    } else {
      type = kMiddleType;
    }

    s = EmitPhysicalRecord(type, ptr, fragment_length);
    ptr += fragment_length;
    left -= fragment_length;
    begin = false;
  } while (s.ok() && left > 0);
  return s;
}

Status Writer::EmitPhysicalRecord(RecordType t, const char* ptr,
                                  size_t length) {
  assert(length <= 0xffff);  // Must fit in two bytes
  assert(block_offset_ + kHeaderSize + length <= kBlockSize);

  // Format the header
  char buf[kHeaderSize];
  buf[4] = static_cast<char>(length & 0xff);
  buf[5] = static_cast<char>(length >> 8);
  buf[6] = static_cast<char>(t);

  // Compute the crc of the record type and the payload.
  uint32_t crc = crc32c::Extend(type_crc_[t], ptr, length);
  crc = crc32c::Mask(crc);  // Adjust for storage
  EncodeFixed32(buf, crc);

  // Write the header and the payload
  // Status s = dest_->Append(Slice(buf, kHeaderSize));

  // PMDB use memcpy write header and payload
  pmem_memcpy(laddr + loffset_, &buf, kHeaderSize, 0);
  loffset_ += kHeaderSize;
  Status s = Status::OK();

  if (s.ok()) {
    // s = dest_->Append(Slice(ptr, length));

    // PMDB
    pmem_memcpy(laddr + loffset_, ptr, length, 0);
    loffset_ += length;

    if (s.ok()) {
      // s = dest_->Flush();

      // PMDB use persist
      if(is_pmem) {
        pmem_persist(laddr + lpersist_offset_, loffset_ - lpersist_offset_);
        lpersist_offset_ = loffset_;
      }
      // pmem_unmap(laddr, lpersist_offset_);
    }
  }
  block_offset_ += kHeaderSize + length;
  return s;
}

// PMDB set file name
void Writer::setFileName(std::string filename) {
  filename_ = filename;
}

void Writer::Close() {
  pmem_unmap(laddr, loffset_);
}

}  // namespace log
}  // namespace leveldb
