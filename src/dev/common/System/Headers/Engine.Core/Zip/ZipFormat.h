#pragma once
#include "ZipIo.h"
#include <Engine.Base/HashMap.h>

namespace rw
{
struct LocalFileHeader
{
    LocalFileHeader()
	:	versionToExtract(0)
	,	bitFlags(0)
	,	compressionMethod(0)
	,	lastModTime(0)
	,	lastModDate(0)
	,	crc32(0)
	,	compressedSize(0)
	,	uncompressedSize(0)
	,	extraFieldLength(0)
	{}

	bool read(Buffer* f);
	void write(Buffer* f) const;

	static const u8 Signature[4];

	u16 versionToExtract;		///< Version needed to extract		(2 bytes)
	u16 bitFlags;				///< General purpose bit flag		(2 bytes)
	u16 compressionMethod;		///< Compression method				(2 bytes)
	u16 lastModTime;			///< Last mod file time				(2 bytes)
	u16 lastModDate;			///< Last mod file date				(2 bytes)
	u32 crc32;					///< Crc-32							(4 bytes)
	s64 compressedSize;			///< Compressed size				(4 bytes)
	s64 uncompressedSize;		///< Uncompressed size				(4 bytes)
	u16 extraFieldLength;		///< Extra field length				(2 bytes)

    /// file name (variable size)
	const char* filename;
};

/// The end-of-central-directory record (32-bit, old-style).
/// All zip files have one of these records tacked onto the end.
struct EndOfCentralDirectory32
{
	bool read(Buffer* f);
	void write(Buffer* f) const;

	static const u8 Signature[4];

	u16 thisDiskNumber;
	u16 diskNumberWithCd;
	u16 numCdEntriesThisDisk;
	u16 totalCdEntries;
	u32 cdSize;
	u32 cdOffsetFromDiskStart;
};

/// The end-of-central-directory locator record (64-bit).
/// Zip files can optionally contain one of these just before the EOCD record;
/// its presence indicates that there is a 64-bit EOCD section as well.
struct EndOfCentralDirectoryLocator64
{
	bool read(Buffer* f);
	void write(Buffer* f) const;

	static const u8 Signature[4];

	u32 diskNumberWithEocd;
	s64 eocdOffset;
	u32 numDisks;
};

/// The end-of-central-directory record (64-bit).
/// This is present in 64-bit zip files and pointed out by a 64-bit eocd locator.
struct EndOfCentralDirectory64
{
	bool read(Buffer* f);
	void write(Buffer* f) const;

	const static u8 Signature[4];
    
	u16 versionMadeBy;					///< version made by (2 bytes)
	u16 versionToExtract;				///< version needed to extract (2 bytes)
	u32 thisDiskNumber;					///< number of this disk (4 bytes)
	u32 diskWithCd;						///< number of the disk with the start of the central directory (4 bytes)
	s64 numCdEntriesThisDisk;			///< total number of entries in the central directory on this disk (8 bytes)
	s64 numCdEntriesTotal;				///< total number of entries in the central directory (8 bytes)
	s64 sizeOfCd;						///< size of the central directory   8 bytes
	s64 centralDirectoryFilePosition;	///< offset of start of cd with respect to the starting disk number        8 bytes

};

// A file header in the central directory.
/// The central directory redundantly stores a lot of data about the files
/// in the archive.
class CentralFileHeader
{
private:
	static const u8 Signature[4];

public:
	const char* filename;

	struct Detail
	{
		u16 versionMadeBy;
		u16 versionNeededToExtract;
		u16 bitFlags;
		u16 compressionMethod;
		u16 lastModFileTime;
		u16 lastModFileDate;
		u32 crc32;
		s64 compressedSize;
		s64 uncompressedSize;
		u32 diskNumberStart;
		u16 internalFileAttributes;
		u32 externalFileAttributes;
		s64 positionOfHeader;
	};

	Detail detail;

public:
	CentralFileHeader()
	{
		memset(&detail, 0, sizeof(detail));
	}

	CentralFileHeader(const char* fn_, const Detail* detail_)
		: filename(fn_)
		, detail(*detail_)
	{
	}

public:
	bool read(Buffer* f);

	/// Return true if 64-bit zip attributes were required.
	bool write(Buffer* f) const;

	/// Return true if this file seems to be a directory. I say "seems to be" because
	/// the detection relies on a few heuristics..
	bool isDirectory() const;

	/// Return true if the header represents an erase file.
	bool isErased() const;
};

/// Maintains a collection of file headers in the zip file sorted by filename.
/// This is used to represent the central directory in memory to locate files
/// quickly. It also tries to support file removal by renaming old enties.
class FileHeaderCollection
{
public:
	// Must be sorted.
	typedef Dict<const char*, CentralFileHeader::Detail> Storage;

private:
	Storage m_headers;

private:
	FileHeaderCollection(const FileHeaderCollection&);
	FileHeaderCollection& operator=(const FileHeaderCollection&);

private:
	/// Find an insert position for a header based on a filename. This has O(log(N)) complexity
	Storage::iterator getHeaderIterator(const char* filename);

public:
	const Storage::iterator getConstHeaderIterator(const char* filename) const;

public:
	FileHeaderCollection();
	~FileHeaderCollection();

	bool findHeaderByRef(const char* filename, CentralFileHeader::Detail** target);

	bool findHeader(const char* filename, CentralFileHeader* target);

	void removeHeader(const char* filename);

	int getErasedFileCount() const;

	void clear();

	void addHeader(const CentralFileHeader& newHeader);

	const Storage& getHeaders() const;

	/// Update fields in a header. The filename must stay the same.
	void updateHeader(const CentralFileHeader& header);
};
}