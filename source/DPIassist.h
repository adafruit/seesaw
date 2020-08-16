void DPIassist_init(uint8_t clockpin, uint8_t datapin, uint8_t cspin, uint8_t resetpin);
void DPIassist_reset(void);
void DPIassist_sendInit(void);
void DPIassist_transfer(uint8_t data, bool dc);
void delay_ms(uint32_t d);
void DPIassist_sendInit(void);
void DPIassist_tick(void);
