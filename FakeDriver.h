#include <PcapFileDevice.h>
#include <RawPacket.h>

vector<RawPacket> packet_list;
uint64_t packet_counter = 0;

bool init(uint16_t dev_id = 0, bool enable_dhcp = false){
	// create a pcap file reader
	pcpp::PcapFileReaderDevice pcapReader("input.pcap");
	pcapReader.open();

	// create a pcapng file writer
	pcpp::PcapNgFileWriterDevice pcapNgWriter("output.pcapng");
	pcapNgWriter.open();

	// raw packet object
	pcpp::RawPacket rawPacket;

	// read packets from pcap reader and write pcapng writer
	while (pcapReader->getNextPacket(rawPacket)) {
	  packet_list.append(rawPacket)
	}

	return true;
}





driver_event receive()
{
	RawPacket *rawPacket = &packet_list[packet_counter];
	packet_counter++;

	vector<uint8_t> pcap_buffer(rawPacket->getRawData(), rawPacket->getRawData() + rawPacket->getRawDataLen());

	return driver_event({NL_CMD_RX, pcap_buffer, &pcap_buffer[0], rawPacket->getRawDataLen()}); 
}