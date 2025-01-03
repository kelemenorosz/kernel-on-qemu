#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#include "err.h"
#include "networking.h"

ERR_CODE dhcp_client(NETWORK_INTERFACE* intf);

#endif /* DHCP_CLIENT_H */