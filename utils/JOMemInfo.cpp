/*
#include "JOMemInfo.h"
#include "JOUtils.h"

#ifdef WIN32 && !defind WP8
#include <Windows.h>
#include <psapi.h>
#pragma comment( lib ,"psapi.lib" )
#elif define ANDROID
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <android/log.h>
#include <unistd.h>
#endif // WIN32 && !defind WP8


JOMemInfo::JOMemInfo()
{

}

JOMemInfo::~JOMemInfo()
{

}

const MEM_INFO* JOMemInfo::memoryInfo()
{
#ifdef WIN32 && !defind WP8
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	if ( GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
	{
		mem_info->uMemUsed = pmc.WorkingSetSize;
	}
	CloseHandle(hProcess);
#else
	vm_statistics_data_t vmStats;
	bool bRet = _memoryInfo(&vmStats);

	struct task_basic_info info;
	bRet &= _taskMemoryInfo(info);

	float fCpuUsage = _cpu_usage();
	if (bRet){
		MEM_INFO memInfo;
		memInfo.uMemUsed = info.resident_size; //????
		memInfo.uMemVirtual = (info.virtual_size; //????
		memInfo.uMemAvailble = (vmStats.free_count*vm_page_size) //????
			memInfo.uMemTotal = NSRealMemoryAvailable() //???
			memInfo.nWarningLv = 1;
		memInfo.fCpuUsage = fCpuUsage; //cpu???
		
		//int memUsed = math.floor(memInfo.uMemUsed / (1024 * 1024))
		//	int memAvailble = math.floor(memInfo.uMemAvailble / (1024 * 1024))
		//	int memTotal = math.floor(memInfo.uMemTotal / (1024 * 1024))
		//	string.format("MEM: %d/%d/%d mb", memUsed, memAvailble, memTotal)
		//	string.format("cpu:%d%%", memInfo.fCpuUsage)
			
	}
	
#endif
	return mem_info;
}

float JOMemInfo::_cpu_usage()
{
#ifdef APPLE
	kern_return_t kr;
	task_info_data_t tinfo;
	mach_msg_type_number_t task_info_count;

	task_info_count = TASK_INFO_MAX;
	kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
	if (kr != KERN_SUCCESS){
		return -1;
	}

	task_basic_info_t basic_info;
	thread_array_t thread_list;
	mach_msg_type_number_t thread_count;

	thread_info_data_t thinfo;
	mach_msg_type_number_t thread_info_count;

	thread_basic_info_t basic_info_th;
	uint32_t stat_thread = 0; //Mach threads

	basic_info = (task_basic_info_t)tinfo;

	// get thread in the task
	kr = task_threads(mach_task_self(), &thread_list, &thread_count);

	if (kr != KERN_SUCCESS){
		return -1;
	}
	if (thread_count>0){
		stat_thread += thread_count;
	}

	long tot_sec = 0;
	long tot_usec = 0;
	float tot_cpu = 0;
	int j;

	for (j = 0; j < thread_count; j++)
	{
		thread_info_count = THREAD_INFO_MAX;
		kr = thread_info(thread_list[j], THREAD_BASIC_INFO, (thread_info_t)thinfo, &thread_info_count);

		if (kr != KERN_SUCCESS){
			return -1;
		}
		basic_info_th = (thread_basic_info_t)thinfo;
		if (!(basic_info_th->flags & TH_FLAGS_IDLE)){
			tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
			tot_usec = tot_usec + basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
			tot_cpu = tot_cpu + basic_info_th->cpu_usage / (float)TH_USAGE_SCALE*100.0;
		}
	} // for each thread

	kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count*sizeof(thread_t));
	assert(kr == KERN_SUCCESS);

	return tot_cpu;

}
#ifdef JOUSE_STD_THREAD
#else
bool JOMemInfo::_memoryInfo(vm_statistics_data_t *vmStats)
{
	int memPercent = 0;
	int phyMem = 0;
	size_t dummy = sizeof(int);
	sysctlbyname("kern.memorystatus_level", &memPercent, &dummy, NULL, 0);
	sysctlbyname("hw.memsize", &phyMem, &dummy, NULL, 0);

	mach_msg_type_number_t infoCount = HOST_VM_INFO_COUNT;
	kern_return_t ret = host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)vmStats, &infoCount);

	return ret == KERN_SUCCESS;
}

bool JOMemInfo::_taskMemoryInfo(task_basic_info& info)
{
	int memPercent = 0;
	int phyMem = 0;
	size_t dummy = sizeof(int);
	sysctlbyname("kern.memorystatus_level", &memPercent, &dummy, NULL, 0);
	sysctlbyname("hw.memsize", &phyMem, &dummy, NULL, 0);

	mach_msg_type_number_t size = sizeof(info);
	kern_return_t ret = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);

	return ret == KERN_SUCCESS;

}
#endif

const char* JOMemInfo::getDeviceName()
{
	size_t size;
	sysctlbyname("hw.machine", NULL, &size, NULL, 0);
	char* machine = (char*)malloc(size);
	sysctlbyname("hw.machine", machine, &size, NULL, 0);
	free(machine);
}

const char* JOMemInfo::getMacAddress()
{
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM lenum;
	int i;
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR*)&lenum;
	Ncb.ncb_length = sizeof(lenum);
	uRetCode = Netbios(&Ncb);

	typedef struct ASTAT
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff[30];
	}ASTAT;

	ASTAT Adapter;
	for (i = 0; i < lenum.length; i++)
	{
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBENUM;
		Ncb.ncb_lana_num = lenum.lana[i];

		uRetCode = Netbios(&Ncb);
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];

		strcpy((char*)Ncb.ncb_callname, "*               "); //15 space
		Ncb.ncb_buffer = (UCHAR*)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);

		uRetCode = Netbios(&Ncb);
		if (uRetCode == 0)
		{
			std::string macAddr = JOString::formatString("%02X:%02X:%02X:%02X:%02X:%02X", Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5]);
			return macAddr.c_str();
		}
		
	}
	return "";
	//////////////////////////////////////////////////////////////////////////
	int mgmtInfoBase[6];
	char *msgBuffer = NULL;
	std::string errorFlag;
	size_t length;

	// setup the management Infomation Base(mib)
	mgmtInfoBase[0] = CTL_NET; //Request network subsystem
	mgmtInfoBase[1] = AF_ROUTE; // Routing table info
	mgmtInfoBase[2] = 0;
	mgmtInfoBase[3] = AF_LINK; // Request link layer information
	mgmtInfoBase[4] = NET_RT_IFLIST; // Request all configured interfaces

	// With all configured interfaces requested, get handle indes
	if ((mgmtInfoBase[5] = if_nametoindex("en0"))==0)
	{
		errorFlag = "if_nametoindex failure";
	}
	// Get the size of the data available (store in len)
	else if (sysctl(mgmtInfoBase, 6, NULL, &length, NULL, 0)<0)
	{
		errorFlag = "sysctl mgmtInfoBase failure";
	}
	// Alloc memory based on above call
	else if ((msgBuffer = (char*)malloc(length)) == NULL)
	{
		errorFlag = "buffer allocation failure";
	}
	// Get system information, store in buffer
	else if (sysctl(mgmtInfoBase, 6, msgBuffer, &length, NULL, 0)<0)
	{
		free(msgBuffer);
		errorFlag = "sysctl msgBuffer failure";
	}
	else
	{
		// Map msgbuffer to interface message structure
		struct if_msghdr * interfaceMsgStruct = (struct if_msghdr *) msgBuffer;

		// Map to link-level socket structure
		struct sockaddr_dl *socketStruct = (struct sockaddr_dl *) (interfaceMsgStruct + 1);

		// Copy link layer address data in socket structure to an array
		unsigned char macAddress[6];
		memcpy(&macAddress, socketStruct->sdl_data + socketStruct->sdl_nlen, 6);

		// Read from char array into a string object, into traditional mac address format
		std::string macAddr = JOString::formatString("%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
		free(macAddress);
		return macAddr.c_str();
	}
	return "";
}

#endif

*/