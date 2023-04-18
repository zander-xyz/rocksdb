//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//

#include "rocksdb/sst_dump_tool.h"

#include "rocksdb/comparator.h"
#include "rocksdb/db.h"
#include <iostream>

int main(int argc, char** argv) {
  ROCKSDB_NAMESPACE::SSTDumpTool tool;
  rocksdb::Options options;
  options.max_open_files = 500;

  rocksdb::PrefixPathComparator storage_comparator;
  rocksdb::PathComparator account_comparator;

  rocksdb::ColumnFamilyOptions storage_opts;
  storage_opts.comparator = &storage_comparator;

  rocksdb::ColumnFamilyOptions account_opts;
  account_opts.comparator = &account_comparator;
  std::vector<rocksdb::ColumnFamilyDescriptor> cfd = {
      {rocksdb::kDefaultColumnFamilyName, {}},
      {"StorageTrieLeaves", storage_opts},
      {"StorageTrieAll", storage_opts},
      {"AccountTrieLeaves", account_opts},
      {"AccountTrieAll", account_opts},
  };
  return tool.Run(argc, argv, options);
}
