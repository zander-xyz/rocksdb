//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//

#include "rocksdb/ldb_tool.h"
#include "rocksdb/comparator.h"
#include <iostream>

namespace detail
{
    // compare number of nibbles first. if equal, compare lexicographically
    inline int path_compare(
            rocksdb::Slice const& s1,
            rocksdb::Slice const& s2,
            size_t begin)
    {
        auto const s1_size = static_cast<uint8_t>(s1[begin]);
        auto const s2_size = static_cast<uint8_t>(s2[begin]);

        auto rc = std::memcmp(&s1_size, &s2_size, 1);
        if (rc != 0) {
            return rc;
        }

        bool const odd = s1_size % 2;
        rc = std::memcmp(s1.data(), s2.data(), s1.size() - odd);
        if (rc != 0 || !odd) {
            return rc;
        }

        uint8_t const b1 = s1[s1.size() - 1] & 0xF0;
        uint8_t const b2 = s2[s2.size() - 1] & 0xF0;

        return std::memcmp(&b1, &b2, 1);
    }
}

// IMPORTANT: changes to the comparator are NOT backwards compatible with
// previous databases.
class PathComparator : public rocksdb::Comparator
{
public:
    virtual int Compare(rocksdb::Slice const& s1, rocksdb::Slice const& s2) const override
    {
        return detail::path_compare(s1, s2, 0);
    }

    // Update this whenever the logic of this compartor is changed
    virtual const char* Name() const override
    {
        return "PathComparator 0.0.1";
    }

    // TODO: implement these for potential optimizations? Figure out what they do
    void FindShortestSeparator(std::string*, rocksdb::Slice const&) const override final {}
    void FindShortSuccessor(std::string*) const override final {}
};

class PrefixPathComparator : public rocksdb::Comparator
{
    int Compare(rocksdb::Slice const& s1, rocksdb::Slice const& s2) const override final
    {
        auto const rc = std::memcmp(s1.data(), s2.data(), 20);
        if (rc != 0) {
            return rc;
        }

        return detail::path_compare(s1, s2, 20);
    }

    // Update this whenever the logic of this compartor is changed
    const char* Name() const override final
    {
        return "PrefixPathComparator 0.0.1";
    }

    // TODO: implement these for potential optimizations? Figure out what they do
    void FindShortestSeparator(std::string*, rocksdb::Slice const&) const override final {}
    void FindShortSuccessor(std::string*) const override final {}
};

int main(int argc, char** argv) {
  ROCKSDB_NAMESPACE::LDBTool tool;

  rocksdb::Options options;
  options.max_open_files = 500;

  PrefixPathComparator storage_comparator;
  PathComparator account_comparator;

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
  tool.Run(argc, argv, options, {}, &cfd);
  return 0;
}
