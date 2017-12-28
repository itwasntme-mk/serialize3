#include <boost/filesystem.hpp>

/** Helper class to compare files: first name is given from caller, second is created internally
    and replace that from caller one. After comparison, if files are different, internal file
    replace the old one - if files are the same, internal file is simply removed and old stay
    untouched.
*/
class TFileComparator
  {
  private:
    typedef boost::filesystem::path path;

  public:
    TFileComparator(path& file, bool compare);

    /// Returns true if files are identical
    bool Compare() const;

    ~TFileComparator();

  private:
    path   OldFile;
    path&  NewFile;
    bool   DoCompare = false;
  };
