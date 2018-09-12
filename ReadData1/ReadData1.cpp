#include "pch.h"
#define LINE_LEN 16
#include <stdio.h>
#include <tchar.h>
#include "pcap.h"
#include <fstream>
#include "String"
#include <vector>
typedef unsigned char hrl_uint8_t;
typedef unsigned short int hrl_uint16_t;
typedef unsigned long hrl_uint32_t;
using namespace std;

const u_char *pkt_data;
const unsigned char *pData;		//读取数据的指针
int num_packet;//记录每一圈中有多少packet

static const int SIZE_BLOCK = 100;
static const int RAW_SCAN_SIZE = 3;
static const int SCANS_PER_BLOCK = 32;
static const int BLOCKS_PER_PACKET = 12;
static const int BLOCK_DATA_SIZE =
(SCANS_PER_BLOCK * RAW_SCAN_SIZE);


struct RawBlock {
	hrl_uint16_t header;        ///< UPPER_BANK or LOWER_BANK
	hrl_uint16_t rotation;      ///< 0-35999, divide by 100 to get degrees
	hrl_uint8_t  data[BLOCK_DATA_SIZE];
};

struct RawPacket {
	RawBlock blocks[BLOCKS_PER_PACKET];
	hrl_uint32_t time_stamp;
	hrl_uint8_t factory[2];
	//uint16_t revolution;
	//uint8_t status[PACKET_STATUS_SIZE];
};

int _tmain(int argc, _TCHAR* argv[]) {
	pcap_if_t *alldevs, *d;
	pcap_t *fp;  //在线&离线数据指针
	u_int inum, i = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	int res;
	struct pcap_pkthdr *header;

	if (argc < 3) {
		printf("选择设备：\n");
		if (pcap_findalldevs(&alldevs, errbuf) == -1) {
			printf("error: \n", errbuf);
			exit(1);
		}

		//打印设备列表
		for (d = alldevs; d; d = d->next) {
			printf("%d. %s\n", ++i, d->name);
			if (d->description)
				printf("(%s)\n", d->description);
			else
			{
				printf("无设备详情");
			}
		}

		if (i == 0) {
			printf("winpcap未安装");
		}
		printf("请输入设备序号(1-%d):", i);
		scanf_s("%d", &inum);
		
		if (inum<1 || inum>i) {
			printf("输入的序号错误");
			pcap_freealldevs(alldevs);
			return -1;
		}


		//选择适配器
		for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);
		//打开适配器
		if ((fp = pcap_open_live(d->name, 65536, 1, 1000, errbuf)) == NULL) {
			printf("打开适配器错误");
			return -1;
		}
		
		//读包
		while ((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0) {		//pkt_data中存储着数据，pcap_next_ex()每次捕获一个数据包，成功捕获则返回1
			if (res == 0)
				continue;

			/////////////////////////////////////////////////////////////////////////////////////将数据显示在控制台
			//printf("%1d:%1d(%1d)\n", header->ts.tv_sec, header->ts.tv_usec, header->len);

			////打印包
			//for (i = 1; (i < header->caplen + 1); i++) {
			//	printf("%.2x", pkt_data[i - 1]);
			//	if ((i%LINE_LEN) == 0)
			//		printf("\n");
			//}
			//printf("\n\n");
			/////////////////////////////////////////////////////////////////////////////////////





		}
		if (res == -1) {
			printf("读文件错误:&s\n", pcap_geterr(fp));
			return -1;
		}

		pcap_close(fp);
	}
}

int decodeCircle(/*  传入u_char* data  (即pkt_data)  */) {
	
	const RawPacket* raw_packet;
	pData = pkt_data + 42;
	raw_packet = (const RawPacket*)(pData);
	decodePacket(raw_packet);
	num_packet++;
}

void decodePacket( const RawPacket* packet ) {

}

