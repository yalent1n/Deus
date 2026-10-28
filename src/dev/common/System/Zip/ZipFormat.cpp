#include <Engine.Core/Zip/ZipFormat.h>
#include <Engine.Base/Memory/VirtualAlloc.h>

namespace rw
{
namespace zipFile
{
	const char ErasedPrefix[] = "{erased}";

	char* readString(Buffer* source, u16 length)
	{
        char* str = (char*) HeapAllocGlobal(length);
		source->read(str, size_t(length));
        return str;
	}
}

// local file header signature, 4 bytes (0x04034b50)
const u8 LocalFileHeader::Signature[4]					= { 0x50, 0x4b, 0x03, 0x04 };

// central file header signature, 4 bytes (0x02014b50)
const u8 CentralFileHeader::Signature[4]				= { 0x50, 0x4b, 0x01, 0x02 };

// end of central dir signature, 4 bytes (0x06054b50)
const u8 EndOfCentralDirectory32::Signature[4]			= { 0x50, 0x4b, 0x05, 0x06 };

// zip64 end of central dir locator signature, 4 bytes (0x07064b50)
const u8 EndOfCentralDirectoryLocator64::Signature[4]	= { 0x50, 0x4b, 0x06, 0x07 };

// zip64 end of central dir signature, 4 bytes (0x07064b50)
const u8 EndOfCentralDirectory64::Signature[4]			= { 0x50, 0x4b, 0x06, 0x06 };

#ifndef __GNUC__
# pragma pack(push, 1)
#endif
struct DiskLayout {
	char signature[4];
	u16 versionToExtract;		// Version needed to extract		(2 bytes)
	u16 bitFlags;				// General purpose bit flag			(2 bytes)
	u16 compressionMethod;		// Compression method				(2 bytes)
	u16 lastModTime;			// Last mod file time				(2 bytes)
	u16 lastModDate;			// Last mod file date				(2 bytes)
	u32 crc32;					// Crc-32							(4 bytes)
	u32 compressedSize;			// Compressed size					(4 bytes)
	u32 uncompressedSize;		// Uncompressed size				(4 bytes)
	u16 filenameLength;			// File name length					(2 bytes)
	u16 extraFieldLength;		// Extra field length				(2 bytes)
}
#ifdef __GNUC__
__attribute__ ((__packed__))
#else
#pragma pack(pop)
#endif
; // NB: struct ends here.

static_assert(sizeof(DiskLayout) == 30 && "Invalid ZipLocalFileHeader struct packing");

bool LocalFileHeader::read(Buffer* f) 
{
	// This function is called a lot during level loads, so I've opted for the
	// hackish read-an-entire-struct way of reading the data.

	DiskLayout diskLayout;

	f->read(&diskLayout, size_t(sizeof(DiskLayout)));

	if (memcmp(LocalFileHeader::Signature, diskLayout.signature, 4) != 0)
		return false;

	versionToExtract = fromLittleEndian(diskLayout.versionToExtract);
	bitFlags = fromLittleEndian(diskLayout.bitFlags);
	compressionMethod = fromLittleEndian(diskLayout.compressionMethod);
	lastModTime = fromLittleEndian(diskLayout.lastModTime);
	lastModDate = fromLittleEndian(diskLayout.lastModDate);
	crc32 = fromLittleEndian(diskLayout.crc32);
	compressedSize = fromLittleEndian(diskLayout.compressedSize);
	uncompressedSize = fromLittleEndian(diskLayout.uncompressedSize);
	const u16 filenameLength = fromLittleEndian(diskLayout.filenameLength);
	extraFieldLength = fromLittleEndian(diskLayout.extraFieldLength);

    filename = zipFile::readString(f, filenameLength);

	// Parse extension attributes
	u32 remainingSpace = extraFieldLength;
	while (remainingSpace > 0) {
		u16 header;
		u16 dataSize;
        header = f->readUint16();
        dataSize = f->readUint16();

		s64 pos = f->getPosition();

		switch (header)
		{
		case zipFile::ExtraFileInfo_Zip64ExtensionField:
			s64 tmp;
			if (uncompressedSize == 0xFFFFFFFFu) {
                tmp = f->readInt64();
				uncompressedSize = s64(tmp);
			}
			if (compressedSize == 0xFFFFFFFFu) {
				tmp = f->readInt64();
				compressedSize = s64(tmp);
			}
			__builtin_assert(pos+dataSize == f->getPosition());
			break;

		default:
			break;
		}

		f->setPosition(pos+dataSize);

		remainingSpace -= dataSize + 2 + 2;

		// Check for size/index corruption to avoid overruns.
		if (remainingSpace > extraFieldLength)
			return false;
	}

	return true;
}

u32 getTruncated32(s64 val) {
	if (u64(val) > 0xFFFFFFFFu) {
		return 0xFFFFFFFFu;
	} else {
		return u32(val);
	}
}

void LocalFileHeader::write(Buffer* f) const
{
	int zip64AttrsNeeded = 0;
	if (compressedSize > zipFile::MaxZip32FileSize) ++zip64AttrsNeeded;
	if (uncompressedSize > zipFile::MaxZip32FileSize) ++zip64AttrsNeeded;

	f->write(Signature, 4);
    f->write(versionToExtract);
	f->write(bitFlags);
	f->write(compressionMethod);
	f->write(lastModTime);
	f->write(lastModDate);
	f->write(crc32);
	f->write(getTruncated32(compressedSize));
	f->write(getTruncated32(uncompressedSize));

    u16 filenameLen = u16(strlen(filename));
    f->write(filenameLen);

	u16 myExtraFieldLength = u16(zip64AttrsNeeded > 0 ? (2 + 2 + 8 * zip64AttrsNeeded) : 0);
	f->write(myExtraFieldLength);
	f->write(filename, size_t(filenameLen));

	if (myExtraFieldLength > 0) {
		f->write(zipFile::ExtraFileInfo_Zip64ExtensionField);
		f->write(u16(8 * zip64AttrsNeeded));
		if (uncompressedSize > zipFile::MaxZip32FileSize)
            f->write(uncompressedSize);
		if (compressedSize > zipFile::MaxZip32FileSize)
			f->write(uncompressedSize);
	}
}

bool EndOfCentralDirectory32::read(Buffer* f) {
	if (!verifyZipSignature32(Signature, f))
		return false;
        
	thisDiskNumber = f->readUint16();
	diskNumberWithCd = f->readUint16();
	numCdEntriesThisDisk = f->readUint16();
	totalCdEntries = f->readUint16();
	cdSize = f->readUint();
	cdOffsetFromDiskStart = f->readUint();
	u16 commentLen = f->readUint16();
	// Ignore the comment.
    f->seek(size_t(commentLen));
	return true;
}

void EndOfCentralDirectory32::write(Buffer* f) const {
	f->write(Signature, 4);
	f->write(thisDiskNumber);
	f->write(diskNumberWithCd);
	f->write(numCdEntriesThisDisk);
	f->write(totalCdEntries);
	f->write(cdSize);
	f->write(cdOffsetFromDiskStart);
	f->write(u16(0)); // comment length
}

bool EndOfCentralDirectoryLocator64::read(Buffer* f) {
	if (!verifyZipSignature32(Signature, f))
		return false;
	
	diskNumberWithEocd = f->readUint();
	eocdOffset = f->readInt64();
	numDisks = f->readUint();
	return true;
}

void EndOfCentralDirectoryLocator64::write(Buffer* f) const {
	f->write(Signature, 4);
	
	f->write(diskNumberWithEocd);
	f->write(eocdOffset);
	f->write(numDisks);
}

bool EndOfCentralDirectory64::read(Buffer* f) {
	if (!verifyZipSignature32(Signature, f))
		return false;

	s64 mySize;
	mySize = f->readInt64();

	StreamSize origin = f->getPosition();

	versionMadeBy = f->readUint16();
	versionToExtract = f->readUint16();
	thisDiskNumber = f->readUint();
	diskWithCd = f->readUint();
	numCdEntriesThisDisk = f->readInt64();
	numCdEntriesTotal = f->readInt64();
	sizeOfCd = f->readInt64();
	centralDirectoryFilePosition = f->readInt64();
	// Skip the extra data section
	f->setPosition(origin + StreamSize(mySize));
	return true;
}

void EndOfCentralDirectory64::write(Buffer* f) const {
	f->write(Signature, 4);

	// FIXME: this isn't pretty
	s64 mySize = 2 + 2 + 4 + 4 + 8 + 8 + 8 + 8;
	f->write(mySize);

	f->write(versionMadeBy);
	f->write(versionToExtract);
	f->write(thisDiskNumber);
	f->write(diskWithCd);
	f->write(numCdEntriesThisDisk);
	f->write(numCdEntriesTotal);
	f->write(sizeOfCd);
	f->write(centralDirectoryFilePosition);
}

bool CentralFileHeader::read(Buffer* f)
{
	if (!verifyZipSignature32(Signature, f))
		return false;

	detail.versionMadeBy = f->readUint16();
	detail.versionNeededToExtract = f->readUint16();
	detail.bitFlags = f->readUint16();
	detail.compressionMethod = f->readUint16();
	detail.lastModFileTime = f->readUint16();
	detail.lastModFileDate = f->readUint16();
	detail.crc32 = f->readUint();

	// Read the file sizes here as u32s from the vanilla (pre-zip64) header. The fields are StreamSize, so we can't
	// just stream to them directly.
	u32 tmp;
	tmp = f->readUint(); detail.compressedSize = StreamSize(tmp);
	tmp = f->readUint(); detail.uncompressedSize = StreamSize(tmp);

	u16 filenameLength;
	filenameLength = f->readUint16();

	u16 extraFieldLength, commentLength;
	extraFieldLength = f->readUint16();
	commentLength = f->readUint16();

	{
		u16 tmp16;
		tmp16 = f->readUint16();
		detail.diskNumberStart = u32(tmp16);
	}

	detail.internalFileAttributes = f->readUint16();
	detail.externalFileAttributes = f->readUint();

	tmp = f->readUint(); detail.positionOfHeader = StreamSize(tmp);

	filename = zipFile::readString(f, filenameLength);

	// Parse extension attributes
	u32 remainingSpace = extraFieldLength;
	while (remainingSpace > 0) {
		u16 header, dataSize;
		header = f->readUint16();
		dataSize = f->readUint16();
		const StreamSize pos = f->getPosition();

		switch (header)
		{
		case zipFile::ExtraFileInfo_Zip64ExtensionField:
			s64 tmp64;
			if (detail.uncompressedSize == StreamSize(0xFFFFFFFFu)) {
				tmp64 = f->readInt64();
				detail.uncompressedSize = StreamSize(tmp64);
			}
			if (detail.compressedSize == StreamSize(0xFFFFFFFFu)) {
				tmp64 = f->readInt64();
				detail.compressedSize = StreamSize(tmp64);
			}
			if (detail.positionOfHeader == StreamSize(0xFFFFFFFFu)) {
				tmp64 = f->readInt64();
				detail.positionOfHeader = StreamSize(tmp64);
			}
			if (detail.diskNumberStart == 0xFFFFu) {
				detail.diskNumberStart = f->readUint();
			}
			// This can trigger sometimes; esp. for 64-bit zip files
			// created by Total Commander 7b4.. No idea what the remaining fields
			// are supposed to be.
			// FB_ASSERT(pos+dataSize == xftell(f));
			break;

		default:
			break;
		}

		const StreamSize posNow = f->getPosition();
		const StreamSize dest = pos + dataSize;

		if (posNow != dest)
			f->seek(dest-posNow);

		remainingSpace -= dataSize + 2 + 2;

		// Check for size/index corruption to avoid overruns.
		if (remainingSpace > extraFieldLength)
			return false;
	}

	/* ignore the file comment */
	f->seek(commentLength);

	return true;
}

bool CentralFileHeader::write(Buffer* f) const
{
	int zip64AttrsNeeded = 0;

	// See if we need to write any 64-bit quantities
	if (detail.compressedSize > zipFile::MaxZip32FileSize) ++zip64AttrsNeeded;
	if (detail.uncompressedSize > zipFile::MaxZip32FileSize) ++zip64AttrsNeeded;
	// These 64-bit sizes are always written in pairs when needed.
	if (zip64AttrsNeeded > 0)
		zip64AttrsNeeded = 2;
	if (detail.positionOfHeader > zipFile::MaxZip32FileSize) ++zip64AttrsNeeded;

	f->write(Signature, 4);

	const u16 extraDataSize = u16(zip64AttrsNeeded > 0 ? (2 + 2 + zip64AttrsNeeded * 8) : 0);

	f->write(detail.versionMadeBy);
	f->write(detail.versionNeededToExtract);
	f->write(detail.bitFlags);
	f->write(detail.compressionMethod);
	f->write(detail.lastModFileTime);
	f->write(detail.lastModFileDate);
	f->write(detail.crc32);
	f->write(getTruncated32(detail.compressedSize));
	f->write(getTruncated32(detail.uncompressedSize));
	f->write(u16(strlen(filename)));
	f->write(extraDataSize); // extraFieldLength: header + size + num 64 fields
	f->write(u16(0)); // comment length
	f->write(u16(0)); // disk number start
	f->write(detail.internalFileAttributes);
	f->write(detail.externalFileAttributes);
	f->write(getTruncated32(detail.positionOfHeader));

	f->write(filename, strlen(filename));

	// Write extension attributes
	if (extraDataSize > 0) {
		f->write(u16(zipFile::ExtraFileInfo_Zip64ExtensionField));
		f->write(u16(8 * zip64AttrsNeeded));
		if (detail.uncompressedSize > zipFile::MaxZip32FileSize || detail.compressedSize > zipFile::MaxZip32FileSize) {
			f->write(s64(detail.uncompressedSize));
			f->write(s64(detail.compressedSize));
		}
		if (detail.positionOfHeader > zipFile::MaxZip32FileSize)
			f->write(s64(detail.positionOfHeader));
	}

	return zip64AttrsNeeded > 0;
}

bool CentralFileHeader::isDirectory() const {
	return
		(!filename || !filename[0]) &&
		filename[strlen(filename) - 1] == '/' &&
		detail.crc32 == 0 &&
		detail.compressedSize == 0 &&
		detail.uncompressedSize == 0;
}

// Return true if the header represents an erased file.
bool CentralFileHeader::isErased() const {
	return compareIgnoreCase(zipFile::ErasedPrefix, filename, sizeof_array(zipFile::ErasedPrefix) - 1) == 0;
}

//=============================================================================

FileHeaderCollection::Storage::iterator
FileHeaderCollection::getHeaderIterator(const char* filename)
{
	return m_headers.begin();
}

const FileHeaderCollection::Storage::iterator
FileHeaderCollection::getConstHeaderIterator(const char* filename) const
{
	return m_headers.begin();
}

FileHeaderCollection::FileHeaderCollection() {}
FileHeaderCollection::~FileHeaderCollection() {}

bool FileHeaderCollection::findHeaderByRef(const char* filename, CentralFileHeader::Detail** target)
{
	Storage::iterator pos = m_headers.find(filename);
	if (pos == m_headers.end())
		return false;
	else {
		*target = &(pos->second);
		return true;
	}
}

bool FileHeaderCollection::findHeader(const char* filename, CentralFileHeader* target)
{
	CentralFileHeader::Detail* candidate = nullptr;
	if (findHeaderByRef(filename, &candidate)) {
		target->filename = filename;
		target->detail = *candidate;
		return true;
	} else {
		return false;
	}
}

void FileHeaderCollection::removeHeader(const char* filename)
{
	Storage::iterator pos = m_headers.find(filename);
	if (pos == m_headers.end()) {
		EERRORF("Can't remove %s; file not in archive", filename);
		return;
	}

	// Create a new header with the same details but with an "erased" filename. 
	FormattedStringBuilder<256> newName("%s%d", zipFile::ErasedPrefix, getErasedFileCount()+1);
	CentralFileHeader newHeader(newName.c_str(), &(pos->second));

	// Drop the old handle from the header map.
	m_headers.erase(pos);

	addHeader(newHeader);
}

int FileHeaderCollection::getErasedFileCount() const
{
	int count = 0;
	Storage::iterator pos = getConstHeaderIterator(zipFile::ErasedPrefix);
	while (pos != m_headers.end() && equalsIgnoreCase(pos->first /* fn */, zipFile::ErasedPrefix, sizeof_array(zipFile::ErasedPrefix) - 1)) {
		++count;
		++pos;
	}
	return count;
}

void FileHeaderCollection::clear()
{
	m_headers.clear();
}

void FileHeaderCollection::addHeader(const CentralFileHeader& newHeader)
{
	CentralFileHeader::Detail* existing = nullptr;
	// See if we're updating an existing header.
	if (findHeaderByRef(newHeader.filename, &existing)) {
		*existing = newHeader.detail;
	} else {
		m_headers.insert(newHeader.filename, newHeader.detail);
	}
}

const FileHeaderCollection::Storage& FileHeaderCollection::getHeaders() const
{
	return m_headers;
}

void FileHeaderCollection::updateHeader(const CentralFileHeader& header)
{
	CentralFileHeader::Detail* existing = nullptr;
	// Make sure we're updating an existing header.
	if (!findHeaderByRef(header.filename, &existing))
	{
		EERRORF("File %s isn't in the archive", header.filename);
		return;
	}
	*existing = header.detail;
}
}