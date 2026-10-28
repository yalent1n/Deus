#include <Engine.Core/Zip/ZipIo.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
u64 convertZipTimeToDiceTime(u16 date, u16 time) 
{
#ifdef SvcCompilerMSVC
	FILETIME ft;
	if (DosDateTimeToFileTime(date, time, &ft)) 
	{
		ULARGE_INTEGER val;
		val.HighPart = ft.dwHighDateTime;
		val.LowPart = ft.dwLowDateTime;
		return val.QuadPart;
	}
#endif
	return 0;
}

void convertDiceTimeToZipTime(u64 diceTime, u16* date, u16* time)
{
	*date = 0;
	*time = 0;
#ifdef SvcCompilerMSVC
	ULARGE_INTEGER val;
	val.QuadPart = diceTime;
	FILETIME ft;
	ft.dwLowDateTime = val.LowPart;
	ft.dwHighDateTime = val.HighPart;
	FileTimeToDosDateTime(&ft, date, time);
#endif
}

void getCurrentZipTime(u16* date, u16* time)
{
	*date = 0;
	*time = 0;

#ifdef SvcCompilerMSVC
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	FILETIME ft;
	if (SystemTimeToFileTime(&sysTime, &ft)) {
		ULARGE_INTEGER val;
		val.HighPart = ft.dwHighDateTime;
		val.LowPart = ft.dwLowDateTime;
		convertDiceTimeToZipTime(val.QuadPart, date, time);
		return;
	} else {
		EERROR("Couldn't convert system time to file time");
	}
#endif
}

bool findBackwards(
	Buffer* f,
	const u8* signature,
	size_t signatureSize,
	StreamSize maxOffset)
{
	// Save the file cursor
	RevertSeek rewinder(f);

	// See how much we can read of the maximum allowed offset
	const StreamSize origPos = f->getPosition();
	const StreamSize scanSize = min(maxOffset, origPos);

	// If the signature is smalled than the preceeding data it can never match.
	if (size_t(scanSize) < signatureSize)
		return false;

	// Use a vector for temporary storage
    u8* buffer = new u8[scanSize];

	// Move the file cursor back..
	f->setPosition(origPos - scanSize);

	// ..and read the entire candidate space into the buffer
	f->read(&buffer[0], scanSize);

	// Now for the detection; start from the back of the buffer and work towards the beginning
	for(const u8* candidate = &buffer[scanSize-static_cast<StreamSize>(signatureSize)]; candidate != &buffer[0]; --candidate) 
	{
		// Check for a match
		if (candidate[0] != signature[0] || 0 != memcmp(signature, candidate, signatureSize))
			continue;

		// Position the file cursor right after the signature
		f->setPosition(origPos - scanSize + (candidate - &buffer[0]));

		// Disable automatic position resetting because we want the file cursor to move.
		rewinder.disable();
		return true;
	}

	// No match found.
	return false;
}
}