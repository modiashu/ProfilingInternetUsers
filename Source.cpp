/* Source.cpp
 * Author: Ashutosh Modi
 * Description: This file is used to extract data from excel files and
 *              transform data in a form that can be used in MATLAB
 * Date: 29 NOV 2017
 */

#pragma once
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <filesystem>
#include <time.h>
using namespace std;


// Strucure used in creating instances for each entry of data in excel file
struct DayTime
{
	int sec;
	int min;
	int hour;
	int day;

	bool operator <(const DayTime& dt) const
	{
		int totalSec = sec + 60 * min + 3600 * hour;
		int dtTotalSec = dt.sec + 60 * dt.min + 3600 * dt.hour;
		return (totalSec < dtTotalSec);
	}

	bool operator ==(const DayTime& dt) const
	{
		int totalSec = sec + 60 * min + 3600 * hour;
		int dtTotalSec = dt.sec + 60 * dt.min + 3600 * dt.hour;
		return (totalSec == dtTotalSec);
	}

	bool operator <=(const DayTime& dt) const
	{
		int totalSec = sec + 60 * min + 3600 * hour;
		int dtTotalSec = dt.sec + 60 * dt.min + 3600 * dt.hour;
		return (totalSec <= dtTotalSec);
	}

	bool operator >(const DayTime& dt) const
	{
		int totalSec = sec + 60 * min + 3600 * hour;
		int dtTotalSec = dt.sec + 60 * dt.min + 3600 * dt.hour;
		return (totalSec > dtTotalSec);
	}
};

// Used to create time windows
struct Interval
{
	DayTime start;
	DayTime end;
};


//Global variables
list<Interval> TimeSlots;
map<string, vector<map<int, pair<long long, long long>>>> AllData;

int ReadData(string csvFile);
int convertEpoch(long long epochT, struct DayTime &dateTimeStru);
int CreateTimeIntervals(int intervals);
int GetSlotIndex(const struct DayTime &dateTimeStru);
int WriteMissingData();
int ExportData();
int WriteSlots();

int main()
{
	ifstream listFile;
	listFile.open("List.txt");
	if (!listFile.is_open())
	{
		cout << "Error in opening file" << endl;
		return -1;
	}

	string str;

	CreateTimeIntervals(25);
	getline(listFile, str);
	while (!listFile.eof())
	{
		cout << str << endl;
		//Read and transform data
		ReadData(str);
		getline(listFile, str);
	}

	//Update map structure for missing values
	WriteMissingData();

	//Create Slots.csv file required for analysis in MATLAB
	WriteSlots();

	//Create Data.csv file required for analysis in MATLAB
	ExportData();

	return 0;
}

int ReadData(string csvFile)
{
	fstream file;
	file.open(csvFile.c_str());

	if (!file.is_open())
	{
		cout << "Error in opening file" << endl;
		return -1;
	}

	// Create vector of mas to store data usage entries
	// vector index represents day of the week and 
	// map indices indicates slot indeces
	vector<map<int, pair<long long, long long>>> userData(5);
	string str;

	getline(file, str);
	getline(file, str);
	while (!file.eof())
	{
		istringstream myline(str);

		//read 4th column
		for (int i = 0; i <= 3; ++i)
			getline(myline, str, ',');

		int doctets = stoi(str);
		//cout << str << endl;

		//read 6th column
		for (int i = 0; i <= 1; ++i)
			getline(myline, str, ',');

		//cout << str << endl;
		long long epochT = stoll(str);

		//read 10th column
		for (int i = 0; i <= 3; ++i)
			getline(myline, str, ',');
		//cout << str << endl;

		long long duration = stoll(str);
		//double duration;
		//stringstream(str) >> duration;

		if(0 < duration)
		{
			struct DayTime dateTimeStru;
			convertEpoch(epochT, dateTimeStru);
			int slot = GetSlotIndex(dateTimeStru);

			//Check if slot is within weekdays
			if (0<= dateTimeStru.day && 5> dateTimeStru.day)
			{
				//get map from specifc day
				map<int, pair<long long, long long>> &tempMap = userData.at(dateTimeStru.day);

				map<int, pair<long long, long long>>::iterator itr;
				itr = tempMap.find(slot);
				//increment duration and doctets value
				if (itr == tempMap.end())
				{
					tempMap.insert(pair<int, pair<long long, long long>>(slot, make_pair(doctets, duration)));
				}
				else
				{
					pair<long long, long long> pr = itr->second;
					pr.first += doctets;
					pr.second += duration;
					itr->second = pr;
				}
			}
		}
		getline(file, str);
	}

	file.close();

	//Update All data for this user
	
	map<string, vector<map<int, pair<long long, long long>>>>::iterator itr;
	itr = AllData.find(csvFile);
	//increment duration and doctets value
	if (itr == AllData.end())
	{
		//tempMap.insert(pair<int, pair<int, int>>(slot, make_pair(doctets, duration)));
		AllData.insert(pair<string, vector<map<int, pair<long long, long long>>>>(csvFile,userData));
	}
	else
	{
		cout << "Duplicate user found!!" << endl;
	}

	return 0;
}

int convertEpoch(long long epochT, struct DayTime &dateTimeStru)
{
	//double now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	time_t timeEpoch = epochT / 1000;
	char strTime[30];
	ctime_s(strTime, RSIZE_MAX, &timeEpoch);
	//printf("%s", str);

	//To get individual components from time
	struct tm timeStrPtr;
	localtime_s(&timeStrPtr, &timeEpoch);

	//cout << timeStrPtr.tm_year + 1900 << '-';
	//cout << timeStrPtr.tm_mon + 1 << '-';
	//cout << timeStrPtr.tm_mday << ' ';
	//cout << timeStrPtr.tm_hour << ':';
	//cout << timeStrPtr.tm_min << ':';
	//cout << timeStrPtr.tm_sec << endl;

	dateTimeStru.day = timeStrPtr.tm_wday - 1;//-1 because vector iterated from 0, which is considered as Monday
	dateTimeStru.hour = timeStrPtr.tm_hour;
	dateTimeStru.min = timeStrPtr.tm_min;
	dateTimeStru.sec = timeStrPtr.tm_sec;

	return 0;
}

int CreateTimeIntervals(int intervals)
{

	time_t timeNULL, timeMidNight;
	struct tm temp;
	struct tm* timeStru = &temp;

	timeNULL = time(NULL);
	localtime_s(timeStru, &timeNULL);
	timeStru->tm_hour = 0;
	timeStru->tm_min = 0;
	timeStru->tm_sec = 0;
	timeMidNight = mktime(timeStru);
	
	localtime_s(timeStru, &timeMidNight);

	int slot = 60;// in seconds
	//count of total slots
	int totalSlots = (24*60*60)/slot;
	for (int i = 0; i < totalSlots; ++i)
	{
		struct Interval tempStruct;

		tempStruct.start.day = 0;// For slots day does not matter
		tempStruct.start.hour = timeStru->tm_hour;
		tempStruct.start.min = timeStru->tm_min;
		tempStruct.start.sec = timeStru->tm_sec;
		
		//increment time
		timeMidNight += slot;

		//Update structure for updated time
		localtime_s(timeStru, &timeMidNight);

		tempStruct.end.day = 0;
		tempStruct.end.hour = timeStru->tm_hour;
		tempStruct.end.min = timeStru->tm_min;
		tempStruct.end.sec = timeStru->tm_sec;

		TimeSlots.push_back(tempStruct);
	}
	return 0;
}

int GetSlotIndex(const struct DayTime &dateTimeStru)
{
	int size = TimeSlots.size();
	int ind = 1;//used in map; hence started from index 1
	list<Interval>::iterator itr = TimeSlots.begin();
	for (; itr != TimeSlots.end(); ++itr)
	{
		Interval slot = *itr;
		if (slot.start<= dateTimeStru && slot.end>dateTimeStru)
		{
			break;
		}

		ind++;
	}
	if (itr == TimeSlots.end())
		ind--;

	return ind;
}

int ExportData()
{
	ofstream dataCSV;
	dataCSV.open("Data.csv");
	dataCSV << "doctets,Duration,slot,Day,User\n";

	map<string, vector<map<int, pair<long long, long long>>>>::iterator itr;
	itr = AllData.begin();
	for (; itr != AllData.end(); itr++)
	{
		string user = itr->first;
		vector<map<int, pair<long long, long long>>> dayDataVec = itr->second;
		for (int i =0;i<dayDataVec.size();i++)
		{
			map<int, pair<long long, long long>>::iterator inItr;
			map<int, pair<long long, long long>> inMap;
			inMap = dayDataVec.at(i);

			for (inItr = inMap.begin(); inItr != inMap.end(); inItr++)
			{
				long long doctets = inItr->second.first;
				long long duration = inItr->second.second;
				int slot = inItr->first;

				dataCSV << doctets << "," << duration << "," << slot << "," << i + 1 << "," << user << "\n";
			}
		}
	}
	dataCSV.close();
	return 0;
}

int WriteSlots()
{
	ofstream slotCSV;
	slotCSV.open("Slots.csv");
	slotCSV << "StartTime,EndTime\n";
	
	for (Interval s : TimeSlots)
	{
		slotCSV << s.start.hour << ":" << s.start.min << ":" << s.start.sec << "," << s.end.hour << ":" << s.end.min << ":" << s.end.sec << "\n";
	}
	slotCSV.close();
	return 0;
}

int WriteMissingData()
{
	int slotCount = TimeSlots.size();
	map<string, vector<map<int, pair<long long, long long>>>>::iterator itr;
	itr = AllData.begin();
	for (; itr != AllData.end(); itr++)
	{
		//string user = itr->first;
		vector<map<int, pair<long long, long long>>> &dayDataVec = itr->second;

		for (int i = 0; i<dayDataVec.size(); i++)
		{
			map<int, pair<long long, long long>>::iterator inItr;
			map<int, pair<long long, long long>> &inMap = dayDataVec.at(i);
			
			for (int s = 1; s <= slotCount; ++s)
			{
				inItr = inMap.find(s);
				if (inItr == inMap.end())
				{
					inMap.insert(pair<int, pair<long long, long long>>(s,make_pair(0,0)));
				}
			}
		}
	}

	return 0;
}