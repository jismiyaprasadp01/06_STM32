/**
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************
  File:	      FLASH_SECTOR_H7.c
  Modifier:   ControllersTech.com
  Updated:    27th MAY 2021
  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com
  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.
  ***************************************************************************************************************
*/

#ifndef INC_FLASH_SECTOR_F4_H_
#define INC_FLASH_SECTOR_F4_H_

#include "stdint.h"

uint32_t Flash_Write_Only(uint32_t Address, uint32_t *Data, uint16_t words);

void Flash_Read_Data (uint32_t StartSectorAddress, uint32_t *RxBuf, uint16_t numberofwords);

void Flash_Erase_Sector_Only(uint32_t Address);


#endif /* INC_FLASH_SECTOR_F4_H_ */
