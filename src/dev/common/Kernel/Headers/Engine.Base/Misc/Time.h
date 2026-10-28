#pragma once
#include "../String/String.h"

namespace rw
{
    typedef s64 Timestamp;
        
    namespace Time
    {
        // Returns the unix epoch in milliseconds.
        FB_BASE_API Timestamp getSystemMsTime();
        FB_BASE_API Timestamp getSystemUsTime();
    }

    class DateTime
    {
    public:
        FB_BASE_API DateTime();
        FB_BASE_API DateTime(Timestamp dateTime);
        FB_BASE_API DateTime(int year, int month, int day, int hour, int min, int sec, int msec = 0);
        FB_BASE_API DateTime(const DateTime& other);
                
        // Return the UTC date time.
        static FB_BASE_API DateTime now();

        // Returns the actual unix epoch timestamp.
        FB_BASE_API Timestamp toTimestamp() const;

        // Returns the local date time.
        FB_BASE_API DateTime toLocal() const;

        // enum DayFormat : u8
        // {
        //     DayOfWeek = 1 << 0,
        //     DayOfMonth = 1 << 1,
        //     DayOfYear = 1 << 2
        // };

        // Returns the date components to the parameters.
        FB_BASE_API void toDate(int& year, int& month, int& day/*, DayFormat dayFormat = DayOfMonth*/) const;

        // Returns the time components to the parameters.
        FB_BASE_API void toTime(int& hours, int& minutes, int& seconds, int* milliseconds = nullptr) const;
        
        // Returns true if the string is a validly formatted date/datetime
        // and creates a timestamp for it, otherwise false.
        FB_BASE_API bool fromString(const char* str);

        // Returns the string in ISO-8601 format.
        FB_BASE_API void toString(StringEditor& str) const;

        // Compares the two DateTime field.
        FB_BASE_API int compare(const DateTime& rhs) const;

        bool operator<(const DateTime& rhs) const { return compare(rhs) < 0; }
        bool operator>(const DateTime& rhs) const { return compare(rhs) > 0; }
        bool operator==(const DateTime& rhs) const { return compare(rhs) == 0; }

    private:
        Timestamp m_dateTime;
    };
}
