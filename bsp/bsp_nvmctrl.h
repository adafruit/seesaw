#ifndef LIB_NVMCTRL_H
#define LIB_NVMCTRL_H

#include "sam.h"
#include "string.h"

#define EEPROM_SIZE 256
#define EEPROM_ADDR (FLASH_SIZE - EEPROM_SIZE)

#define NVM_MEMORY ((volatile uint16_t *)FLASH_ADDR)

//#define NVMCTRL_ASSERT(t_) ((t_) ? (void)0 : Q_onAssert("bsp_nvmctrl.h", (int_t)__LINE__))

static inline void eeprom_init()
{
	uint32_t ctrlb  = NVMCTRL->CTRLB.reg & ~(NVMCTRL_CTRLB_RWS_Msk | NVMCTRL_CTRLB_MANW);
	ctrlb |= NVMCTRL->CTRLB.reg & (NVMCTRL_CTRLB_RWS_Msk | NVMCTRL_CTRLB_MANW);
	
	NVMCTRL->CTRLB.reg = ctrlb;
}

static void eeprom_erase()
{
	while (!NVMCTRL->INTFLAG.bit.READY);

	/* Clear flags */
	NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

	/* Set address and command */
	NVMCTRL->ADDR.reg = EEPROM_ADDR / 2;
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_ER | NVMCTRL_CTRLA_CMDEX_KEY;
}

static void eeprom_read(uint8_t addr, uint8_t *buf, uint8_t size)
{
	uint32_t src_addr = (EEPROM_ADDR  + addr);
	uint32_t nvm_address = src_addr / 2;
	int32_t i;
	uint16_t data;

	while (!NVMCTRL->INTFLAG.bit.READY);

	/* Clear flags */
	NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

	/* Check whether byte address is word-aligned*/
	if (src_addr % 2) {
		data      = NVM_MEMORY[nvm_address++];
		buf[0] = data >> 8;
		i         = 1;
	} else {
		i = 0;
	}

	/* NVM _must_ be accessed as a series of 16-bit words, perform manual copy
	 * to ensure alignment */
	while (i < size) {
		data      = NVM_MEMORY[nvm_address++];
		buf[i] = (data & 0xFF);
		if (i < ((int32_t)size - 1)) {
			buf[i + 1] = (data >> 8);
		}
		i += 2;
	}
}

static void eeprom_program(uint8_t *buf, uint8_t size)
{

	uint32_t nvm_address = EEPROM_ADDR / 2;
	uint16_t i, data;

	while (!NVMCTRL->INTFLAG.bit.READY);
	
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_PBC | NVMCTRL_CTRLA_CMDEX_KEY;

	while (!NVMCTRL->INTFLAG.bit.READY);
	
	/* Clear flags */
	NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

	for (i = 0; i < size; i += 2) {
		data = buf[i];
		if (i < NVMCTRL_PAGE_SIZE - 1) {
			data |= (buf[i + 1] << 8);
		}
		NVM_MEMORY[nvm_address++] = data;
	}

	while (!NVMCTRL->INTFLAG.bit.READY);

	NVMCTRL->ADDR.reg = EEPROM_ADDR / 2;
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMD_WP | NVMCTRL_CTRLA_CMDEX_KEY;
}
static void eeprom_write(uint8_t addr, uint8_t *buf, uint8_t size)
{
	uint8_t  tmp_buffer[NVMCTRL_PAGE_SIZE];
	eeprom_read(0, tmp_buffer, NVMCTRL_PAGE_SIZE);
	
	memcpy(tmp_buffer + addr, buf, size);

	/* erase row before write */
	eeprom_erase();

	/* write buffer to flash */
	eeprom_program(tmp_buffer, NVMCTRL_PAGE_SIZE);
}

static inline uint32_t eeprom_read_word(uint8_t addr)
{
	//TODO:
    return 0;
}

static inline uint8_t eeprom_read_byte(uint8_t addr)
{
	uint8_t c;
	eeprom_read(addr, &c, 1);
	return c;
}

static inline void eeprom_write_word(uint8_t addr, uint32_t val)
{
	//TODO:
}

static inline void eeprom_write_byte(uint8_t addr, uint8_t val)
{
	eeprom_write(addr, &val, 1);
}

#endif
