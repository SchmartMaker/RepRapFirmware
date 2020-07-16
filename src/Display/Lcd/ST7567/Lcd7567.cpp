/*
 * Lcd7567.cpp
 *
 *  Created on: 16 Jul 2020
 *      Author: Martijn
 */

#include "Lcd7567.h"

#if SUPPORT_12864_LCD

#define TILE_WIDTH 1
#define TILE_HEIGHT 8

Lcd7567::Lcd7567(const LcdFont * const fnts[], size_t nFonts) noexcept
	: Lcd(128, 64, fnts, nFonts)
{
}

void Lcd7567::HardwareInit() noexcept
{
	// Set DC/A0 pin to be an output with initial LOW state (command: 0, data: 1)
	pinMode(a0Pin, OUTPUT_LOW);

#ifdef SRC_DUETM_PINS_DUETM_H_
	// Only for the Duet 2 Maestro since the LCD_CS gates the SPI0_SCK and SPI0_MOSI.
	pinMode(LcdCSPin, OUTPUT_LOW);
#endif

	SelectDevice();

	// 11100010 System reset
	SendLcdCommand(0xE2);
	// 10101110 Set display enable to off
	SendLcdCommand(0xAE);
	// 01000000 Set scroll line to 0 (6-bit value)
	SendLcdCommand(0x40);
	// 10100000 Set SEG (column) direction to MX (mirror = 0)
	SendLcdCommand(0xA0);
	// 11001000 Set COM (row) direction not to MY (mirror = 0)
	SendLcdCommand(0xC8);
	// 10100110 Set inverse display to false
	SendLcdCommand(0xA6);
	// 10100010 Set LCD bias ratio BR=0 (1/9th at 1/65 duty)
	SendLcdCommand(0xA2);
	// 00101111 Set power control to enable XV0, V0 and VG charge pumps
	SendLcdCommand(0x2F);
	// 11111000 Set booster ratio (2-byte command) to 4x
	SendLcdCommand(0xF8);
	SendLcdCommand(0x00);
	// 00100011 Set Vlcd resistor ratio 1+Rb/Ra to 6.5 for the voltage regulator (contrast)
	SendLcdCommand(0x23);
	// 10000001 Set electronic volume (2-byte command) 6-bit contrast value
	SendLcdCommand(0x81);
	SendLcdCommand(0x27);
	// 10101100 Set static indicator off
	SendLcdCommand(0xAC);

	// Enter sleep mode
	// 10101110 Set display enable to off
	SendLcdCommand(0xAE);
	// 10100101 Set all pixel on
	SendLcdCommand(0xA5);

	DeselectDevice();

	Clear();
	FlushAll();

	SelectDevice();

	// Enable display
	// 10100100 Set all pixel off
	SendLcdCommand(0xA4);
	// 10101111 Set display enable to on
	SendLcdCommand(0xAF);

	DeselectDevice();
}

// Flush some of the dirty part of the image to the LCD, returning true if there is more to do
bool Lcd7567::FlushSome() noexcept
{
	// See if there is anything to flush
	if (endCol > startCol && endRow > startRow)
	{
		// Decide which row to flush next
		if (nextFlushRow < startRow || nextFlushRow >= endRow)
		{
			nextFlushRow = startRow;	// start from the beginning
		}

		if (nextFlushRow == startRow)	// if we are starting from the beginning
		{
			startRow += TILE_HEIGHT;	// flag this row as flushed because it will be soon
		}

		uint8_t startCom = nextFlushRow & ~(TILE_HEIGHT - 1);

		// Flush that row (which is 8 pixels high)
		{
			SelectDevice();
			SetGraphicsAddress(startCom, startCol);
			StartDataTransaction();

			// Send tiles of 1x8 for the desired (quantized) width of the dirty rectangle
			for(int x = startCol; x < endCol; x += TILE_WIDTH)
			{
				uint8_t data = 0;

				// Gather the bits for a vertical line of 8 pixels (LSB is the top pixel)
				for(uint8_t i = 0; i < 8; i++)
				{
					if(ReadPixel(x, startCom + i)) {
						data |= (1u << i);
					}
				}

				SendLcdData(data);
			}

			EndDataTransaction();
			DeselectDevice();
		}

		// Check if there is still area to flush
		if (startRow != endRow)
		{
			nextFlushRow += TILE_HEIGHT;
			return true;
		}

		startRow = numRows;
		startCol = numCols;
		endCol = endRow = nextFlushRow = 0;
	}

	return false;
}

void Lcd7567::SelectDevice() noexcept
{
#ifdef SRC_DUETM_PINS_DUETM_H_
	// Only for the Duet 2 Maestro since the LCD_CS gates the SPI0_SCK and SPI0_MOSI.
	digitalWrite(LcdCSPin, true);
#endif

	delayMicroseconds(1);
	device.Select();
	delayMicroseconds(1);
}

void Lcd7567::DeselectDevice() noexcept
{
	delayMicroseconds(1);
	device.Deselect();

	#ifdef SRC_DUETM_PINS_DUETM_H_
	// Only for the Duet 2 Maestro since the LCD_CS gates the SPI0_SCK and SPI0_MOSI.
	digitalWrite(LcdCSPin, false);
#endif
}

// Set the address to write to.
// The display memory is organized in 8+1 pages (of horizontal rows) and 0-131 columns
void Lcd7567::SetGraphicsAddress(unsigned int r, unsigned int c) noexcept
{
	// 0001#### Set Column Address MSB
	SendLcdCommand(0x10 | ((c >> 4) & 0b00001111));
	// 0000#### Set Column Address LSB
	SendLcdCommand(0x00 | (c & 0b00001111));
	// 1011#### Set Page Address
	SendLcdCommand(0xB0 | ((r >> 3) & 0b00001111));

	CommandDelay();
}

void Lcd7567::CommandDelay() noexcept
{
	delayMicroseconds(64);
}

void Lcd7567::DataDelay() noexcept
{
	delayMicroseconds(4);
}

void Lcd7567::SendLcdCommand(uint8_t byteToSend) noexcept
{
	//digitalWrite(a0Pin, false);
	uint8_t data[1] = { byteToSend };
	device.TransceivePacket(data, nullptr, 1);
}

void Lcd7567::StartDataTransaction() noexcept
{
	digitalWrite(a0Pin, true);
}

void Lcd7567::SendLcdData(uint8_t byteToSend) noexcept
{
	//digitalWrite(a0Pin, true);
	uint8_t data[1] = { byteToSend };
	device.TransceivePacket(data, nullptr, 1);
}

void Lcd7567::EndDataTransaction() noexcept
{
	digitalWrite(a0Pin, false);
}

#endif

// End
