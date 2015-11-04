

#include "Pcap.h"

Pcap::Pcap() : handler_(NULL)
{
	
}

Pcap::~Pcap()
{
	close();
}

void Pcap::close()
{
	if(handler_)
	{
		pcap_close(handler_);
		handler_ = NULL;
	}
}

int Pcap::open_offline(const char *fname)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	handler_ = pcap_open_offline(fname, errbuf);
	if ( !handler_)
	{
		fprintf(stderr, "pcap_open_offline %s failed, reason: %s\n", fname, errbuf);
		return -1;
	}
	return 0;
}

const u_char *Pcap::next_packet(struct pfring_pkthdr *hdr)
{
	return(handler_ ? pcap_next(handler_, (struct pcap_pkthdr *)hdr) : NULL);
}

PcapDumper::PcapDumper() : dumper_(NULL)
{
	
}

PcapDumper::~PcapDumper()
{
	if (dumper_) 
	{
		pcap_dump_close(dumper_);
		dumper_ = NULL;
	}
}

int PcapDumper::open(const char *fname, int linktype, u_int32_t snaplen)
{
	if (dumper_)
	{
		fprintf(stderr, "Twice open, fname: %s\n", fname);
		return -1;
	}
	dumper_ = pcap_dump_open(pcap_open_dead(linktype, snaplen /* MTU */), fname);
	return (dumper_ ? 0 : -1);
}

