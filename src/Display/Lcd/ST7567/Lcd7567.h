/*
 * Lcd7567.h
 *
 *  Created on: 16 Jul 2020
 *      Author: Martijn
 */

#ifndef SRC_DISPLAY_LCD_ST7567_LCD7567_H_
#define SRC_DISPLAY_LCD_ST7567_LCD7567_H_

#include "RepRapFirmware.h"

#if SUPPORT_12864_LCD

#include <Display/Lcd/Lcd.h>

class Lcd7567 : public Lcd
{
public:
	// Construct a GLCD driver.
	Lcd7567(const LcdFont * const fnts[], size_t nFonts) noexcept;

	// Flush just some data, returning true if this needs to be called again
	bool FlushSome() noexcept override;

protected:
	void HardwareInit() noexcept override;
	void CommandDelay() noexcept override;
	void DataDelay() noexcept override;
	void SendLcdCommand(uint8_t byteToSend) noexcept override;
	void SendLcdData(uint8_t byteToSend) noexcept override;

private:
	void SelectDevice() noexcept;
	void DeselectDevice() noexcept;
	void SetGraphicsAddress(unsigned int r, unsigned int c) noexcept;
	void StartDataTransaction() noexcept;
	void EndDataTransaction() noexcept;
};

#endif

#endif /* SRC_DISPLAY_LCD_ST7567_LCD7567_H_ */
