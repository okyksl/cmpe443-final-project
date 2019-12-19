#include "Utility.h"

void BYTE_WRITE(volatile uint32_t* reg, uint32_t mask, uint32_t data) {
	uint32_t value = *reg;
	value &= ~mask;
	value |= (mask & data);
	*reg = value;
}
