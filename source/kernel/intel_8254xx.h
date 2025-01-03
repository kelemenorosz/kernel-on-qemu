#ifndef INTEL_8254XX_H
#define INTEL_8254XX_H

#include "err.h"
#include "pci.h"

ERR_CODE intel_8254xx_init(PCI_ENUM_TOKEN* pci_token);

#endif /* INTEL_8254XX_H */