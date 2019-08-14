//#include <ffm_videoTst.h>
#include <iostream>
#include <ffm_util.h>

using namespace std;
//using namespace cv;

#define CHECH_FMT(args) \
	((args->in_pixFmt != AV_PIX_FMT_BGR24) || (args->out_pixFmt != AV_PIX_FMT_YUV420P)) 

int get_sysCpuNum()
{
#if 0
#if defined WIN32 || defined WIN64
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return (int)sysinfo.dwNumberOfProcessors;
#elif defined __linux__
		return (int)sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
		int numCPU = 0;
		int mib[4];
		size_t len = sizeof(numCPU);
		// set the mib for hw.ncpu
		mib[0] = CTL_HW;
		mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
		sysctl(mib, 2, &numCPU, &len, NULL, 0);	 // get the number of CPUs from the system
		if (numCPU < 1) {
			mib[1] = HW_NCPU;
			sysctl(mib, 2, &numCPU, &len, NULL, 0);
			if (numCPU < 1)
				numCPU = 1;
		}
		return (int)numCPU;
#else
		return 1;
#endif
#endif
}


void ffm_init()
{
    //avdevice_register_all();
	avcodec_register_all();
	av_register_all();
	avformat_network_init();
}
int save_pic(const char *path, AVPacket *pkt)
{
	if (!pkt || (pkt->size <= 0)) {
		cout << "invalid pkt"<<endl;
		return -1;
	}
	FILE *fp;
	fp = fopen(path, "wb");
	fwrite(pkt->data, 1, pkt->size, fp);
	fclose(fp);
	return 0;
}
