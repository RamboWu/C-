#pragma once

#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#include <stdint.h>
#include <string>
#include <time.h>


class Time {

	static const int month_sum[13];

	int GetNumber(const std::string& str, int start, int length, int& end) {
		int result = 0;
		int count = 0;
		if (start < str.length()) {

			for (int i = start; i < str.length(); i++) {
				if (str[i] >= 48 && str[i] <= 48 + 9) {
					result = result * 10 + str[i] - 48;
					count++;
				}
				if (count == length) {
					end = i;
					break;
				}
			}
		}
		if (count != length) end = str.length();
		return result;
	}

	time_t GetTimeNumber(const std::string &str) {
		tm p;
		int year, month, day, hour, minutes, second;
		int count;
		year = GetNumber(str, 0, 4, count);
		month = GetNumber(str, count + 1, 2, count);
		day = GetNumber(str, count + 1, 2, count);
		hour = GetNumber(str, count + 1, 2, count);
		minutes = GetNumber(str, count + 1, 2, count);
		second = GetNumber(str, count + 1, 2, count);

		p.tm_year = year - 1900;
		p.tm_mon = month - 1;
		p.tm_mday = day;
		p.tm_hour = hour;
		p.tm_min = minutes;
		p.tm_sec = second;
		p.tm_isdst = 0;
		p.tm_wday = 0;
		p.tm_yday = 0;
		return mktime(&p);
	}

public:

	bool IsLeapYear(int year) {
		if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
			return true;
		else
			return false;
	}

	time_t ToLong(const std::string &str) {
		return GetTimeNumber(str);
	}

	std::string GetTimeString(time_t time_number) {
		tm* p=localtime(&time_number);
		char chTmp[20];
		sprintf(chTmp, "%d-%d%d-%d%d %d%d:%d%d:%d%d",
			1900 + p->tm_year,
			(1 + p->tm_mon) / 10,
			(1 + p->tm_mon) % 10,
			p->tm_mday/10,
			p->tm_mday%10,
			p->tm_hour/10,
			p->tm_hour%10,
			p->tm_min/10,
			p->tm_min%10,
			p->tm_sec/10,
			p->tm_sec%10
		);
		return std::string(chTmp);
	}
};

const int Time::month_sum[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

#endif