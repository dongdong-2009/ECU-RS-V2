1. support board
# STM32F107 yuneng debug board
   - 10M/100M ethernet (RMII)		-LAN8720A
		* ETH_MDIO		PA2	
		* RMII_RXD0		PC4
		* ETH_MDC		PC1	
		* RMII_RXD1		PC5
		* RMII_TXD0		PB12	
		* RMII_CRS_DV	PA7
		* RMII_TXD1		PB13	
		* RMII_REFCLK	PA1
		* RMII_TX_EN	PB11	
		* ETH_RESET		接地

   - SPI flash (IO simulation)		-W25Q32BVSSIG
     SPI: SPI1 
		* SPI1_MOSI: 	PA5
		* SPI1_MISO: 	PA4
		* SPI1_SCK : 	PA6
		* CS0: 			PA3
		
	-RTC(DS1302Z)					-DS1302Z
		* DS1302_SCLK:	PC0
		* DS1302_IO:	PC8
		* DS1302_RST:	PC2
		
	-Zigbee(Serial)
		* ZB_RX:		PC11
		* ZB_TX:		PC10

	-LED
		* LED_GREEN		PC9
		
	-I2C(IO simulation)				-EEPROM
		* EEPROM_WP		PB10
		* EEPROM_SDA	PB1
		* EEPROM_SCL	PB0
		