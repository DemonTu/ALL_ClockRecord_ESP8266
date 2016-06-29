#ifndef __SWTOOL_H
	#define __SWTOOL_H

uint_fast16_t BigEndingBuf_To_U16(void const *src);	
uint_fast32_t BigEndingBuf_To_U32(void const *src);
void *U16_To_BigEndingBuf(void *dest, uint_fast16_t bin);
void *U32_To_BigEndingBuf(void *dest, uint_fast32_t bin);

uint_fast8_t Add8Sum_To_U08(void const *src, uint_fast16_t len);
uint_fast16_t TcpIp_CheckSum(void const *src, uint_fast16_t length); 
uint_fast16_t Crc16_1021_Sum(void const *src, uint_fast16_t len);

#endif	
