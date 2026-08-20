#ifndef DAIMON_KCORE_IO_H
#define DAIMON_KCORE_IO_H

#include "kcore.h"

#define PDP10_DEV_CTY          0120U

kword_t pdp10_coni_cty(void);
void pdp10_cono_cty(kword_t word);
kword_t pdp10_datai_cty(void);
void pdp10_datao_cty(kword_t word);
void pdp10_io_wait(unsigned int spins);

#define pdp10_coni(dev)        pdp10_coni_cty()
#define pdp10_cono(dev, word)  pdp10_cono_cty(word)
#define pdp10_datai(dev)       pdp10_datai_cty()
#define pdp10_datao(dev, word) pdp10_datao_cty(word)

#endif
