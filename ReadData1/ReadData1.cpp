
#pragma once
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

#define DEG_TO_RAD 0.017453292

const u_char *pkt_data;
const unsigned char *pData;		//读取数据的指针
int num_packet;//记录每一圈中有多少packet

#define M_PI  3.14159265358979323846

static const int SCANS_PER_FIRING = 16;
static const int FIRINGS_PER_BLOCK = 2;

static const int SIZE_BLOCK = 100;
static const int RAW_SCAN_SIZE = 3;
static const int SCANS_PER_BLOCK = 32;
static const int BLOCKS_PER_PACKET = 12;
static const int BLOCK_DATA_SIZE =
(SCANS_PER_BLOCK * RAW_SCAN_SIZE);
static const int FIRINGS_PER_PACKET =
FIRINGS_PER_BLOCK * BLOCKS_PER_PACKET;




double rawAzimuthToDouble(const uint16_t& raw_azimuth) {
	// According to the user manual,
	// azimuth = raw_azimuth / 100.0;
	/*int raw_azimuth2=raw_azimuth+0;
	if (raw_azimuth2>36000)
	{
		raw_azimuth2=raw_azimuth2-36000;
	}*/
	return static_cast<double>(raw_azimuth) / 100.0 * DEG_TO_RAD;
}

struct Firing {
	// Azimuth associated with the first shot within this firing.
	double firing_azimuth;
	double azimuth[SCANS_PER_FIRING];
	double distance[SCANS_PER_FIRING];
	double intensity[SCANS_PER_FIRING];

};
Firing firings[FIRINGS_PER_PACKET];//定义存储firing的数据空间
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



void decodePacket( const RawPacket* packet ) {



	// Compute the azimuth angle for each firing.
	for (size_t fir_idx = 0; fir_idx < FIRINGS_PER_PACKET; fir_idx += 2) {
		size_t blk_idx = fir_idx / 2;
		firings[fir_idx].firing_azimuth = rawAzimuthToDouble(
			packet->blocks[blk_idx].rotation);
		//rotation_record<<packet->blocks[blk_idx].rotation<<endl;
	}

	// Interpolate the azimuth values
	for (size_t fir_idx = 1; fir_idx < FIRINGS_PER_PACKET; fir_idx += 2) {
		size_t lfir_idx = fir_idx - 1;
		size_t rfir_idx = fir_idx + 1;

		if (fir_idx == FIRINGS_PER_PACKET - 1) {
			lfir_idx = fir_idx - 3;
			rfir_idx = fir_idx - 1;
		}

		double azimuth_diff = firings[rfir_idx].firing_azimuth -
			firings[lfir_idx].firing_azimuth;
		azimuth_diff = azimuth_diff < 0 ? azimuth_diff + 2 * M_PI : azimuth_diff;

		firings[fir_idx].firing_azimuth =
			firings[fir_idx - 1].firing_azimuth + azimuth_diff / 2.0;
		firings[fir_idx].firing_azimuth =
			firings[fir_idx].firing_azimuth > 2 * M_PI ?
			firings[fir_idx].firing_azimuth - 2 * M_PI : firings[fir_idx].firing_azimuth;
	}

	// Fill in the distance and intensity for each firing.
	for (size_t blk_idx = 0; blk_idx < BLOCKS_PER_PACKET; ++blk_idx) {
		const RawBlock& raw_block = packet->blocks[blk_idx];

		for (size_t blk_fir_idx = 0; blk_fir_idx < FIRINGS_PER_BLOCK; ++blk_fir_idx) {
			size_t fir_idx = blk_idx * FIRINGS_PER_BLOCK + blk_fir_idx;

			double azimuth_diff = 0.0;
			if (fir_idx < FIRINGS_PER_PACKET - 1)
				azimuth_diff = firings[fir_idx + 1].firing_azimuth -
				firings[fir_idx].firing_azimuth;
			else
				azimuth_diff = firings[fir_idx].firing_azimuth -
				firings[fir_idx - 1].firing_azimuth;
			if (azimuth_diff < 0)
			{
				azimuth_diff += 2 * M_PI;
			}

			for (size_t scan_fir_idx = 0; scan_fir_idx < SCANS_PER_FIRING; ++scan_fir_idx) {
				size_t byte_idx = RAW_SCAN_SIZE * (
					SCANS_PER_FIRING*blk_fir_idx + scan_fir_idx);

				//// Azimuth
				//firings[fir_idx].azimuth[scan_fir_idx] = firings[fir_idx].firing_azimuth +
				//	(scan_fir_idx*DSR_TOFFSET / FIRING_TOFFSET) * azimuth_diff;

				//// Distance
				//TwoBytes raw_distance;
				//raw_distance.bytes[0] = raw_block.data[byte_idx];
				//raw_distance.bytes[1] = raw_block.data[byte_idx + 1];
				//firings[fir_idx].distance[scan_fir_idx] = static_cast<double>(
				//	raw_distance.distance) * DISTANCE_RESOLUTION;

				//// Intensity
				//firings[fir_idx].intensity[scan_fir_idx] = static_cast<double>(
				//	raw_block.data[byte_idx + 2]);

				printf("%ld\n", firings[fir_idx].intensity[scan_fir_idx]);
			}
		}
	}
	return;

}

int decodeCircle(const u_char* pkt_data) {

	const RawPacket* raw_packet;
	pData = pkt_data + 42;
	raw_packet = (const RawPacket*)(pData);
	decodePacket(raw_packet);
	num_packet++;

	return 0;
}

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

			decodeCircle(pkt_data);



		}
		if (res == -1) {
			printf("读文件错误:&s\n", pcap_geterr(fp));
			return -1;
		}

		pcap_close(fp);
	}
}