#include <Engine.Core/FsBackend.h>
#include <Engine.Base/String/Util.h>
#include <vcruntime_string.h>

namespace rw
{
FileIterator::FileIterator()
{}

FileIterator::~FileIterator()
{}

FsBackend::FsBackend()
{}

FsBackend::FsBackend(StringFn_t&& fn)
    : m_isDynamic(true)
{
    m_dirFnPath = fn;
}

FsBackend::FsBackend(const char* path)
    : m_isDynamic(false)
{
    m_dirAbsPath = path;
}

inline bool isabsoluteorunc(const char* path)
{
    bool isdriveabs = isalchar(path[0]) && path[1] == ':';
    bool isuncabs = issep(path[0]) && issep(path[1]);
    return !isnil(path) && (isdriveabs | isuncabs);
}

struct pathstruct
{
    const char* dir;
    const char* file;
    const char* filter;
};

bool internalTraverseFiles(FsBackend* self, const char* filter, TraverseFn_t fn, bool excludeDirs, PathStringBuilder& rollingDir, u8 depth)
{
    size_t iBS = lastIndexOf('\\', filter);
    size_t iFS = lastIndexOf('/', filter);
    size_t iDX = iFS < iBS ? iFS : iBS;

    char name[_MAX_PATH];

    FileInfo file;
    ScopedRef<FileIterator> iter = self->getFileIterator(filter);
    while(iter->getNextFile(name, sizeof(name), &file))
    {               
        PathStringBuilder rollingSubDir;
        PathStringBuilder subDirFilter;

        if(file.isDirectory)
        {

            if(!rollingDir.isEmpty())
                rollingSubDir << rollingDir;
            
            if(iDX != npos && !depth)
            {
                rollingSubDir.appendChars(filter, iDX + 1);
            }
            else
            {
                rollingSubDir.appendChars(name).appendChar('/');
            }

            subDirFilter
                .appendChars(c_str(rollingSubDir))
                .appendChars(filter + (__maybe(iDX != npos, iDX + 1)));
        }

        PathStringBuilder nativePath;
        if(const char* dir = self->directory(); dir)
            nativePath.appendChars(dir);

        if(!nativePath.isEmpty() && !issep(nativePath.back()))
        {
            nativePath
                .appendChar('/');
        }

        // miniature fix to have correct local absolute directory paths
        if(!depth && iDX != npos)
        {
            nativePath.appendChars(filter, iDX);
        }

        if(!rollingDir.isEmpty() && depth)
        {
            nativePath
                .appendChars(c_str(rollingDir));
        }

        // "content/*.*" -> basically this
        if(!(file.isDirectory && !depth && iDX != npos))
            nativePath.appendChars(name);

        fixPath(nativePath.data(), nativePath.size());    
        fn(c_str(nativePath), file);

        if(file.isDirectory) {
            internalTraverseFiles(self, subDirFilter.c_str(), fn, excludeDirs, rollingSubDir, depth + 1);
        }
    }

    return true;
}

bool FsBackend::traverseFiles(const char* filter, TraverseFn_t fn, bool excludeDirs)
{
    PathStringBuilder subDir;
    return internalTraverseFiles(this, filter, fn, excludeDirs, subDir, 0);
}

void FsBackend::nativePathTranslation(const char* file, StringEditor& result) const
{
    bool do_append = true;

    // either "\\path\to\dir" or "g:\path\to\dir"
    if(isabsoluteorunc(file))
        do_append = false; // actually an absolute or UNC path

    const char* dir = directory();
    if(isnil(dir))
        do_append = false; // assume any path is the raw path
    
    if(do_append)
    {
        result.appendChars(dir);
        if(!issep(dir[strlen(dir) - 1]))
            result.appendChar('/');
    }

    result.appendChars(file);
    fixPath(result.data(), result.size());
}

const char* FsBackend::directory() const
{
    return m_isDynamic ? m_dirFnPath() : m_dirAbsPath;
}
}