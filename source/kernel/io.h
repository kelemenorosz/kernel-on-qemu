#ifndef IO_H
#define IO_H

extern uint32_t ioread(uint32_t port_address);
extern uint8_t ioreadb(uint32_t port_address);
extern uint16_t ioreadw(uint32_t port_address);
extern void iowrite(uint32_t port_address, uint32_t to_write);
extern void iowriteb(uint32_t port_address, uint32_t to_write);
extern void iowritew(uint32_t port_address, uint32_t to_write);

#endif /* IO_H */