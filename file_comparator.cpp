///\file file_comparator.cpp

#include "file_comparator.h"

#include <iostream>
#include <algorithm>
#include <boost/iostreams/device/mapped_file.hpp>


namespace bfs = boost::filesystem;
namespace bio = boost::iostreams;


TFileComparator::TFileComparator(bfs::path& file, bool compare)
  : OldFile(file), NewFile(file), DoCompare(compare && bfs::exists(file))
  {
  if (DoCompare == false)
    return;

  NewFile = OldFile.parent_path();
  NewFile /= OldFile.stem().generic_string() + "__temporary_internal_file";
  NewFile.replace_extension(OldFile.extension());
  }

bool TFileComparator::Compare() const
  {
  if (DoCompare == false || bfs::exists(NewFile) == false || bfs::exists(OldFile) == false)
    return false;

  bool result = false;

  {
  bio::mapped_file_source f1(NewFile);
  bio::mapped_file_source f2(OldFile);

  result = (f1.size() == f2.size() && std::equal(f1.data(), f1.data() + f1.size(), f2.data()));
  }

  if (result == false)
    bfs::rename(NewFile, OldFile);

  NewFile = OldFile;

  return result;
  }

TFileComparator::~TFileComparator()
  {
  if (DoCompare && bfs::exists(NewFile))
    bfs::remove(NewFile);
  }
