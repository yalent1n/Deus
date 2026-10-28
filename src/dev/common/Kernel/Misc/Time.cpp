#include <Engine.Base/Misc/Time.h>
#include <cstring>
#include <time.h>
#include <new>

#ifdef SvcCompilerMSVC
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#include <sys/timeb.h>
#endif

namespace rw
{
	inline constexpr Timestamp s_epochTime = 11644473600000ll;	
	inline constexpr Timestamp s_minTime = 0; // 1970-01-01T00:00:00Z

	namespace Time
	{
		Timestamp getSystemMsTime()
		{
			SYSTEMTIME sysTime;
			LARGE_INTEGER fileTime;
			GetSystemTime(&sysTime);
			SystemTimeToFileTime(&sysTime, (LPFILETIME)&fileTime);
			s64 epoch = (fileTime.QuadPart / 10000) - s_epochTime;
			return epoch;
		}
		
		Timestamp getSystemUsTime()
		{
			static LARGE_INTEGER liFreq;
			static bool bInit;
			if(!bInit)
			{
				QueryPerformanceFrequency(&liFreq);
				bInit = YES;
			}

			LARGE_INTEGER liCounter;
			QueryPerformanceCounter(&liCounter);

			return (uint64_t)(liCounter.QuadPart * 1000000ULL / liFreq.QuadPart);
		}
	}

	DateTime::DateTime()
		: m_dateTime(s_minTime)
	{}

	DateTime::DateTime(Timestamp dateTime)
		: m_dateTime(dateTime)
	{}

	DateTime::DateTime(int year, int month, int day, int hour, int min, int sec, int msec)
	{
		SYSTEMTIME sysTime;
		sysTime.wYear = year;
		sysTime.wMonth = month;
		sysTime.wDay = day;
		sysTime.wHour = hour;
		sysTime.wMinute = min;
		sysTime.wSecond = sec;
		sysTime.wMilliseconds = msec;
		SystemTimeToFileTime(&sysTime, (LPFILETIME)&m_dateTime);
		m_dateTime = (m_dateTime / 10000) - s_epochTime;
	}

	DateTime::DateTime(const DateTime& other)
		: m_dateTime(other.m_dateTime)
	{}

	DateTime DateTime::now()
	{
		return DateTime(Time::getSystemMsTime());	
	}

	Timestamp DateTime::toTimestamp() const
	{
		return m_dateTime;
	}

	DateTime DateTime::toLocal() const
	{
		timeb tmb;
		ftime(&tmb);

		return DateTime(m_dateTime - tmb.timezone * 60000 + tmb.dstflag * 0x36EE80);
	}

	void DateTime::toDate(int& year, int& month, int& day/*, DayFormat dayFormat*/) const
	{
        s64 rest = m_dateTime / 1000;

		int z = rest / 86400 + 719468;
        int era = (z >= 0 ? z : z - 146096) / 146097;
        unsigned doe = static_cast<unsigned>(z - era * 146097);
        unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
        int y = static_cast<int>(yoe) + era * 400;
        unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);
        unsigned mp = (5*doy + 2)/153;
        int d = doy - (153*mp+2)/5 + 1;
        int m = mp + (mp < 10 ? 3 : -9);
        y += (m <= 2);

		year = y;
		month = m;
		day = d;
	}
	
	void DateTime::toTime(int& hours, int& minutes, int& seconds, int* milliseconds) const
	{
		s64 ms = m_dateTime % 1000;
        s64 rest = m_dateTime / 1000;
        s64 sec = rest % (60 * 60 * 24);
        s64 hour = sec / (60 * 60);
        sec -= (hour * 60 * 60);
        s64 min = sec / 60;
        sec -= min * 60;

		hours = static_cast<int>(hour);
		minutes = static_cast<int>(min);
		seconds = static_cast<int>(sec);
		if(milliseconds) *milliseconds = ms;
	}

	bool DateTime::fromString(const char* str)
	{
		size_t len = rw::strlen(str);
		if(len == 0)
			return false;

		int Tindex = indexOf('T', str);
		int Sindex = indexOf('-', str);
		//bool isTimeOnly = Tindex < 0;
		bool isUtc = str[len - 1] == 'Z';
		//bool hasOffset = str[len - 6] =='+' || str[len - 6] == '-';

		int year = 1970, month = 1, day = 1;
		int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

		char dateBuffer[6];
		bool hasDateComponent = Sindex == 4;
		bool hasTimeComponent = Tindex > 0 ? str[Tindex + 3] == ':' : str[2] == ':';

		if(!hasDateComponent && !hasTimeComponent)
			return false;

		if(hasDateComponent)
		{
			const char* dateCursor = str;
			int sep;

			if((sep = indexOf('-', dateCursor)) < 0)
				return false;
            std::memcpy(dateBuffer, dateCursor, sep);
			dateCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, year))
				return false;

			if((sep = indexOf('-', dateCursor)) < 0)
				return false;
			std::memcpy(dateBuffer, dateCursor, sep);
			dateCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, month))
				return false;
			
			sep = hasTimeComponent ? indexOf('T', dateCursor) : indexOf('\0', dateCursor);
			std::memcpy(dateBuffer, dateCursor, sep);
			dateCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, day))
				return false;
		}
		
		if(hasTimeComponent)
		{
			const char* timeCursor;
			int sep;

			timeCursor = str + Tindex + 1;
			
			if((sep = indexOf(':', timeCursor)) < 0)
				return false;
			std::memcpy(dateBuffer, timeCursor, sep);
			timeCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, hours))
				return false;

			if((sep = indexOf(':', timeCursor)) < 0)
				return false;
			std::memcpy(dateBuffer, timeCursor, sep);
			timeCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, minutes))
				return false;
			
			int MSindex = indexOf('.', timeCursor);
			sep = MSindex >= 0 ? MSindex : (isUtc ? indexOf('Z', timeCursor) : indexOf('\0', timeCursor));
			std::memcpy(dateBuffer, timeCursor, sep);
			timeCursor += sep+1;
			dateBuffer[sep] = '\0';
			if(!tointeger(dateBuffer, seconds))
				return false;

			if(MSindex > 0)
			{
				int l = isUtc ? indexOf('Z', timeCursor) : indexOf('\0',  timeCursor);
                std::memcpy(dateBuffer, timeCursor, l);
				dateBuffer[l] = '\0';
				if(!tointeger(dateBuffer, milliseconds))
					return false;

				// sanity check
				if(milliseconds >= 1000)
					return false;
			}
		}

		new (this) DateTime(year, month, day, hours, minutes, seconds, milliseconds);
		return true;
	}

	void DateTime::toString(StringEditor& str) const
	{
		int year, month, day, hour, min, sec, msec;
		toDate(year, month, day);
		toTime(hour, min, sec, &msec);

		str<<year<<'-'; 
		if(month<10) str<<'0'; str<<month<<'-'; 
		if(day<10) str<<'0'; str<<day;
		str<<'T';
		if(hour<10) str<<'0'; str<<hour<<':'; 
		if(min<10) str<<'0'; str<<min<<':'; 
		if(sec<10) str<<'0'; str<<sec;
		if(msec) str<<'.'<<msec;
		str<<'Z';
	}

	int DateTime::compare(const DateTime& rhs) const
	{
		return m_dateTime - rhs.m_dateTime;
	}
}
