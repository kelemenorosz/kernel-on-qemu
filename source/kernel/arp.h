#ifndef ARP_H
#define ARP_H

void* arp_lookup(uint32_t ip);
void arp_req(uint32_t ip);

uint8_t arp_decode(void* buf);

#endif /* ARP_H */