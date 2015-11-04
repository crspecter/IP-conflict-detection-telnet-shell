

#ifndef _PCAP_H_
#define _PCAP_H_

extern "C" {
#include <pcap.h>
#include <pfring.h>
}

class Pcap
{
public:
  Pcap();
  ~Pcap();
  void close();
  int open_offline(const char *fname);  
  const u_char *next_packet(struct pfring_pkthdr *hdr);
  static int parse_pkt(u_char *pkt, struct pfring_pkthdr *hdr, u_int8_t level=4 /* l4 */, 
		       u_int8_t add_timestamp=1 /* 0,1 */, u_int8_t add_hash=1 /* 0,1 */)
	  { return pfring_parse_pkt(pkt, hdr, level, add_timestamp, add_hash); }
private:
  pcap_t	*handler_;
};


class PcapDumper
{
public:
  PcapDumper();
  ~PcapDumper();
  int open(const char *fname, int linktype=DLT_EN10MB, u_int32_t snaplen=1600);
  void dump(struct pfring_pkthdr *hdr, const u_char *pkt)
    { if (dumper_) pcap_dump((u_char*)dumper_, (struct pcap_pkthdr*)hdr, pkt); }
private:
  pcap_dumper_t *dumper_;
};

#endif // --- end #ifndef _PCAP_H_ ---

