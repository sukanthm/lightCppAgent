#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>
#include <list>
#include <sstream>
#include <time.h>
#include <iterator> 
#include <future>
using namespace std;

#define MB 1048576

static int CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	static unsigned long long _previousTotalTicks = 0;
	static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	int ret = round(100 * (1 - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0)));

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}

static unsigned long long FileTimeToInt64(const FILETIME & ft)
{
	return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

int GetCPULoad()
{
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0;
}

void showlist(list <int> g)
{
	list <int> ::iterator it;
	cout << g.size() << ":";
	for (it = g.begin(); it != g.end(); ++it)
		cout << *it << " ";
	cout << '\n';
}

void getCPU(int pollInterval, int windowSize, int popCount, int varianceCap)
{
	list <int> CPUlist;

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);

	string fileName = string("CPU_") /*+ string(buf) */ + ".csv";
	ofstream myfile;
	string msg = "CPU%,mean,variance\n";
	myfile.open(fileName.c_str(), ios::out);
	myfile.write(msg.c_str(), msg.length());
	myfile.close();

	while (true)
	{
		int sum = 0, mean = 0, sum2 = 0, variance = 0;

		CPUlist.push_back(GetCPULoad());
		//showlist(CPUlist);

		if (CPUlist.size() >= windowSize) {

			list <int> ::iterator it;
			for (it = CPUlist.begin(); it != CPUlist.end(); ++it) {
				sum += *it;
			}
			mean = sum / CPUlist.size();
			for (it = CPUlist.begin(); it != CPUlist.end(); ++it) {
				sum2 += (*it - mean)*(*it - mean);
			}
			variance = sum2 / CPUlist.size();
			//cout << "mean:" << mean << "\t" << "variance:" << variance << "\n";

			if (variance > varianceCap) {
				msg = "";
				for (int i = 0; i < popCount; ++i) {
					stringstream ss;
					ss << CPUlist.front();
					CPUlist.pop_front();
					msg += ss.str() + ",-1,-1" + "\n";
				}
				myfile.open(fileName.c_str(), ios::out | ios::app);
				myfile.write(msg.c_str(), msg.length());
				myfile.close();
				cout << "written to CPU file:" << endl << msg << endl << endl;
			}
			else {
				msg = "";
				CPUlist.clear();
				stringstream ss1, ss2;
				ss1 << mean;
				ss2 << variance;
				msg += "-1," + ss1.str() + "," + ss2.str() + "\n";

				myfile.open(fileName.c_str(), ios::out | ios::app);
				myfile.write(msg.c_str(), msg.length());
				myfile.close();
				cout << "written to CPU file:" << endl << msg << endl << endl;
			}
		}
		Sleep(pollInterval * 1000);
	}
}

void getRAM(int pollInterval, int windowSize, int popCount, int varianceCap)
{
	list <int> RAMlist;

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);

	string fileName = string("RAM_") /*+ string(buf) */ + ".csv";
	ofstream myfile;
	string msg = "RAM%,mean,variance\n";
	myfile.open(fileName.c_str(), ios::out);
	myfile.write(msg.c_str(), msg.length());
	myfile.close();

	while (true)
	{
		int sum = 0, mean = 0, sum2 = 0, variance = 0;

		MEMORYSTATUSEX memStat;
		memStat.dwLength = sizeof(memStat);
		GlobalMemoryStatusEx(&memStat);

		RAMlist.push_back(memStat.dwMemoryLoad);
		//showlist(RAMlist);

		if (RAMlist.size() >= windowSize) {

			list <int> ::iterator it;
			for (it = RAMlist.begin(); it != RAMlist.end(); ++it) {
				sum += *it;
			}
			mean = sum / RAMlist.size();
			for (it = RAMlist.begin(); it != RAMlist.end(); ++it) {
				sum2 += (*it - mean)*(*it - mean);
			}
			variance = sum2 / RAMlist.size();
			//cout << "mean:" << mean << "\t" << "variance:" << variance << "\n";

			if (variance > varianceCap) {
				msg = "";
				for (int i = 0; i < popCount; ++i) {
					stringstream ss;
					ss << RAMlist.front();
					RAMlist.pop_front();
					msg += ss.str() + ",-1,-1" + "\n";
				}
				myfile.open(fileName.c_str(), ios::out | ios::app);
				myfile.write(msg.c_str(), msg.length());
				myfile.close();
				cout << "written to RAM file:" << endl << msg << endl << endl;
			}
			else {
				msg = "";
				RAMlist.clear();
				stringstream ss1, ss2;
				ss1 << mean;
				ss2 << variance;
				msg += "-1," + ss1.str() + "," + ss2.str() + "\n";

				myfile.open(fileName.c_str(), ios::out | ios::app);
				myfile.write(msg.c_str(), msg.length());
				myfile.close();
				cout << "written to RAM file:" << endl << msg << endl << endl;
			}
		}
		Sleep(pollInterval * 1000);
	}
}

int main()
{
	auto a1 = async(launch::async, getCPU, 3, 5, 1, 10);
	auto a2 = async(launch::async, getRAM, 3, 5, 1, 10);

	return 0;
}
