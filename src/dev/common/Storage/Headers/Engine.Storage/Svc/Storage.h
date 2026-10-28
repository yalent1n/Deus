#pragma once
#include "../Bundle.h"
#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/HashMap.h>

namespace rw::esis
{
// Represents a file controller/directory watcher for a VCS domain.
class Storage
{
public:
    Storage(const char* root = NIL) {
        
    }

    // 'dir' should be relative to the storage root
    // @returns An SHA1 (transaction id?) that identifies a 'group' thing.
    const SHA1& WatchDirectory(const char* dir, bool subdir = true);
    bool StopWatchDirectory(const SHA1& id);
    const Dict<SHA1, void*> GetObservedDirs() const
        { return mDirectoriesWatched; }

    const SHA1& WatchFile(const char* file);
    bool StopWatchFile(const SHA1&);
    const Dict<SHA1, void*> GetObservedFiles() const
        { return mFilesWatched; }

private:
    Dict<SHA1, void*> mDirectoriesWatched;
    Dict<SHA1, void*> mFilesWatched;
};
}