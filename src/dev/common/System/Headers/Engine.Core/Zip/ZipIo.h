#pragma once
#include "ZipConstants.h"
#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Base/String/String.h>
#include <cstring>

namespace rw
{
/// Convert the zip MS-DOS date stamp to our internal representation.
u64 convertZipTimeToDiceTime(u16 date, u16 time);

/// Convert an internal time stamp to a MS-DOS time/date used in zip files.
void convertDiceTimeToZipTime(u64 diceTime, u16* date, u16* time);

/// Convert the current time into a MS-DOS time/date used in zip files.
void getCurrentZipTime(u16* date, u16* time);

inline bool verifyZipSignature32(const u8* expected, Buffer* f)
{
	u8 my_signature[4];
	f->read(my_signature, sizeof(my_signature));
	return 0 == memcmp(expected, my_signature, sizeof(my_signature));
}

// Helper RAII class to roll back the file cursor
class RevertSeek 
{
public:

	explicit RevertSeek(Buffer* f)
	:	m_file(f)
	,	m_origPos(f->getPosition())
	,	m_disabled(false)
	{ }

	~RevertSeek() 
	{
		if (!m_disabled)
			m_file->setPosition(m_origPos);
	}

	void disable() 
	{
		m_disabled = true;
	}

private:

	RevertSeek(const RevertSeek&);
	RevertSeek& operator=(const RevertSeek&);

private:

	Buffer* const m_file;
	const StreamSize m_origPos;
	bool m_disabled;
};

/// Scan backwards for the specified signature pattern an either
/// 1) return true with the file cursor positioned at the signature, or
/// 2) return false with the file cursor unmodified
bool findBackwards(Buffer* f, const u8* signature, size_t signatureSize, StreamSize maxOffset);
}