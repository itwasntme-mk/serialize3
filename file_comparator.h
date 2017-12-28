#include <boost/filesystem.hpp>

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
