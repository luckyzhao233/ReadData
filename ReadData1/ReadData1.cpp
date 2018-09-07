#include "pch.h"
#define LINE_LEN 16
#include <stdio.h>
#include <tchar.h>
#include "pcap.h"
#include <fstream>
#include "String"
#include <vector>
using namespace std;

int _tmain(int argc, _TCHAR* argv[]) {
	pcap_if_t *alldevs, *d;
	pcap_t *fp;
	u_int inum, i = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	int res;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;

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
		while ((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0) {
			if (res == 0)
				continue;
			printf("%1d:%1d(%1d)\n", header->ts.tv_sec, header->ts.tv_usec, header->len);

			//打印包
			for (i = 1; (i < header->caplen + 1); i++) {
				printf("%.2x", pkt_data[i - 1]);
				if ((i%LINE_LEN) == 0)
					printf("\n");
			}
			printf("\n\n");
		}
		if (res == -1) {
			printf("读文件错误:&s\n", pcap_geterr(fp));
			return -1;
		}

		pcap_close(fp);
	}
}