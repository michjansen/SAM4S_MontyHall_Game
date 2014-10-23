/**
 * \file
 *
 * \brief Monty Hall Game (started from Starter Kit Demo project)
 *
 * Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage Monty Hall Game
 *
 * \section Purpose
 *
 * This project uses the Starter Kit Demo as base for a Monty Hall game as a way to get
 * familiar the Atmel Studio environment while having some fun.
 *
 * This demo features the IO1 and OLED1 extension boards for the SAM4 Xplained Pro.
 *
 * \section Requirements
 *
 * This package can be used with SAM Xplained Pro evaluation kits.
 *
 * \section Description
 *
 * The game will use the buttons on the OLED1 extension to select a door, one of the other
 * two doors will be opened and the player can choose to switch doors.  Statistics will be
 * maintained to show which choice is statistically better.  Theoretically switching doors
 * has slightly better odds of winning.
 *
 * IO1 extension must be connected on EXT2.
 * OLED1 extension must be connected on EXT3.
 *
 */

#include <asf.h>
#include <string.h>

enum DOOR_PRESSED_EVENTS
{
	DOOR_PRESSED_MIN = 1,
	DOOR_PRESSED_MAX = 3,
	DOOR_NOT_PRESSED
};

volatile uint32_t g_door_pressed = DOOR_NOT_PRESSED;

typedef enum 
{
	MONTY_GAME_STARTED,
	FIRST_DOOR_OPEN,
	GAME_OVER_WON,
	GAME_OVER_LOST
} MONTY_HALL_STATE;

typedef struct 
{
	uint32_t number_of_games;
	uint32_t times_switched;
	uint32_t times_switched_won;
	uint32_t times_won;
	
	MONTY_HALL_STATE state;
	uint32_t first_door;
	uint32_t open_door;
	uint32_t winning_door;
	
} monty_hall_state;
	
uint32_t pick_open_door( uint32_t winning_door, uint32_t first_door )
{
	uint32_t open_door = DOOR_NOT_PRESSED;
	if( first_door != winning_door )
	{
		// Since the winning door is not the selected door,
		//  we need simply pick the opposite unselected door
		//  There is probably a more efficient algorithm for this,
		//  but this will work for now.
		if (first_door == 1)
		{
			if (winning_door == 2)
			{
				open_door = 3;
			}
			else
			{
				open_door = 2;
			}
		}
		else if (first_door == 2)
		{
			if (winning_door == 3)
			{
				open_door = 1;
			}
			else
			{
				open_door = 3;
			}
		}
		else if (first_door == 3)
		{
			if (winning_door == 1)
			{
				open_door = 2;
			}
			else
			{
				open_door = 1;
			}
		}
	}
	else
	{
		open_door = 1;
		if( open_door == winning_door )
		{
			// we can't pick this door, since it is the winning one
			open_door++;
		}
		
		// Since Monty can open either door, we need to randomly select
		//  a door.
		int random_value = rand();
		if( random_value & 0x1 )
		{
			open_door++;
		}
		if( open_door == winning_door )
		{
			// we can't pick this door, since it is the winning one
			open_door++;
		}
	}
	return open_door;
}

int32_t handle_door_press( monty_hall_state *p_game_state, uint32_t new_door_press )
{
	if( p_game_state == NULL )
	{
		return -1;
	}
	
	switch( p_game_state->state )
	{
		case MONTY_GAME_STARTED:
		{
			p_game_state->winning_door = (rand() % 3) + 1;
			p_game_state->first_door = new_door_press;
			p_game_state->state = FIRST_DOOR_OPEN;
			p_game_state->open_door = pick_open_door( p_game_state->winning_door, new_door_press );
			break;
		}
		case FIRST_DOOR_OPEN:
		{
			if( p_game_state->open_door == new_door_press )
			{
				// Invalid button press, stay in this state and wait for another press
				return -1;
			}
			if( p_game_state->winning_door == new_door_press )
			{
				p_game_state->state = GAME_OVER_WON;
				p_game_state->times_won++;
			}
			else
			{
				p_game_state->state = GAME_OVER_LOST;
			}
			if( p_game_state->first_door != new_door_press )
			{
				p_game_state->times_switched++;
				if( p_game_state->state == GAME_OVER_WON )
				{
					p_game_state->times_switched_won++;
				}
			}
			p_game_state->number_of_games++;
			break;
		}
		default:
		case GAME_OVER_LOST:
		case GAME_OVER_WON:
		{
			p_game_state->state = MONTY_GAME_STARTED;
			break;
		}
	}
	return 0;
}

/**
 * \brief Process Buttons Events.
 *
 * \param uc_button The button number.
 */
static void ProcessButtonEvt(uint8_t uc_button)
{
	if ((uc_button >= DOOR_PRESSED_MIN) && 
	    (uc_button <= DOOR_PRESSED_MAX))
	{
		g_door_pressed = uc_button;
	}
	else
	{
		g_door_pressed = DOOR_NOT_PRESSED;
	}
}

/**
 * \brief Handler for Button 1 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button1_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_1_ID == id) && (PIN_PUSHBUTTON_1_MASK == mask))
		ProcessButtonEvt(1);
}

/**
 * \brief Handler for Button 2 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button2_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_2_ID == id) && (PIN_PUSHBUTTON_2_MASK == mask))
		ProcessButtonEvt(2);
}

/**
 * \brief Handler for Button 3 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button3_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_3_ID == id) && (PIN_PUSHBUTTON_3_MASK == mask))
		ProcessButtonEvt(3);
}

/* IRQ priority for PIO (The lower the value, the greater the priority) */
#define IRQ_PRIOR_PIO    0

/**
 * \brief Configure the Pushbuttons.
 *
 * Configure the PIO as inputs and generate corresponding interrupt when
 * pressed or released.
 */
static void configure_buttons(void)
{
	/* Configure Pushbutton 1. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_1_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID,
			PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR, Button1_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_1_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_1_PIO, (IRQn_Type) PIN_PUSHBUTTON_1_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);

	/* Configure Pushbutton 2. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_2_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_ID,
			PIN_PUSHBUTTON_2_MASK, PIN_PUSHBUTTON_2_ATTR, Button2_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_2_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_2_PIO, (IRQn_Type) PIN_PUSHBUTTON_2_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK);

	/* Configure Pushbutton 3. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_3_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_ID,
			PIN_PUSHBUTTON_3_MASK, PIN_PUSHBUTTON_3_ATTR, Button3_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_3_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_3_PIO, (IRQn_Type) PIN_PUSHBUTTON_3_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_MASK);

}

/**
 * \brief Draw graph on the OLED screen using the provided point array.
 * \param col X coordinate.
 * \param page Y coordinate (please refer to OLED datasheet for page description).
 * \param width Graph width.
 * \param height Graph height.
 * \param tab Data to draw. Must contain width elements.
 */
static void ssd1306_draw_graph(uint8_t col, uint8_t page, uint8_t width, uint8_t height, uint8_t *tab)
{
	uint8_t page_start;
	uint8_t i, j, k, s;
	uint8_t scale;

	for (i = col; i < width; ++i) {
		for (page_start = page; page_start <= height; ++page_start) {
			ssd1306_write_command(SSD1306_CMD_SET_PAGE_START_ADDRESS(page_start));
			ssd1306_set_column_address(i);
			j = tab[i];
			scale = 8 * (height - page_start + 1);
			if (j > scale)
				j = 8;
			else
				j -= (scale - 8);

			for (k = 0, s = j; j > 0; --j)
				k = (k << 1) + 1;
			for (s = 8 - s; s > 0; --s)
				k <<= 1;
			ssd1306_write_data(k);
		}
	}
}

/**
 * \brief Clear one character at the cursor current position on the OLED
 * screen.
 */
static void ssd1306_clear_char(void)
{
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
}

/**
 * \brief Initializes the UART for transmitting characters
 */
sam4s_console_uart_init()
{
    const sam_uart_opt_t uart_console_settings = {
        sysclk_get_cpu_hz(),
        9600,
        UART_MR_PAR_NO
    };

    uart_init(UART1,&uart_console_settings);
    uart_enable_tx(UART1);                 
    uart_enable(UART1);
}

/**
 * \brief Transmits a line characters through console UART (appends a line feed to the end)
 * Waits until all characters have been sent before returning (i.e. not buffered), ideally the
 * timeout would be implement in actual time and be based on the BAUD rate that the UART has
 * been configured.  However, this is sufficient for now and ensures the board won't hang forever.
 *
 * \param p_string - buffer of characters to transmit
 * \param max_len - maximum number of characters that may be in the buffer
 * \param uart_timeout_cnt - number of times to try to write to the UART before timing out
 */
void print_uart( char * p_string, uint32_t max_len, uint32_t uart_timeout_cnt )
{
    uint32_t len = strnlen(p_string, max_len);
    for( uint32_t i = 0; i < len; i++ )
    {
        for( uint32_t count = 0; count < uart_timeout_cnt; count++ )
        {
            if( uart_write(UART1, p_string[i]) == 0 )
            {
                break;
            }
        }
    }
    for( uint32_t count = 0; count < uart_timeout_cnt; count++ )
    {
        if( uart_write(UART1, '\n') == 0 )
        {
            break;
        }
    }
}


int main(void)
{
	const uint32_t max_disp_string = 120;
    const uint32_t max_uart_tries  = 1000000;
    char result_disp[max_disp_string];

	// Initialize clocks.
	sysclk_init();

	// Initialize GPIO states.
	board_init();

	// Initialize at30tse.
	at30tse_init();

	// Configure IO1 buttons.
	configure_buttons();

	// Initialize SPI and SSD1306 controller.
	ssd1306_init();
	ssd1306_clear();


	monty_hall_state game_state = { 0, 0, 0, 0, MONTY_GAME_STARTED,
								DOOR_NOT_PRESSED, DOOR_NOT_PRESSED, DOOR_NOT_PRESSED };
	for( ;; )
	{
		int32_t result = 0;
		if( g_door_pressed != DOOR_NOT_PRESSED )
		{
			result = handle_door_press( &game_state, g_door_pressed );
			g_door_pressed = DOOR_NOT_PRESSED;
			if( game_state.state == FIRST_DOOR_OPEN )
			{
				sprintf( result_disp, "Game State %d: selected door %d open door %d", 
				         game_state.state,
						 game_state.first_door,
						 game_state.open_door );
				print_uart( result_disp, max_disp_string, max_uart_tries );
			}
			else if( game_state.state == GAME_OVER_WON )
			{
				sprintf( result_disp, "Won: Game State %d: selected door %d open door %d", 
				         game_state.state,
						 game_state.first_door,
						 game_state.open_door );
				print_uart( result_disp, max_disp_string, max_uart_tries );
			}
			else if( game_state.state == GAME_OVER_LOST )
			{
				sprintf( result_disp, "Lost: Game State %d: selected door %d open door %d", 
				         game_state.state,
						 game_state.first_door,
						 game_state.open_door );
				print_uart( result_disp, max_disp_string, max_uart_tries );
			}
		}

#if 0		
		/* Refresh page title only if necessary. */
		if (app_mode_switch > 0)
		{
			app_mode = (app_mode + 1) % 3;

			// Clear screen.
			ssd1306_clear();
			ssd1306_set_page_address(0);
			ssd1306_set_column_address(0);


			/* Temperature mode. */
			if (app_mode == 0)
			{
				ioport_set_pin_level(IO1_LED1_PIN, IO1_LED1_ACTIVE);
				ioport_set_pin_level(IO1_LED2_PIN, !IO1_LED2_ACTIVE);
				ioport_set_pin_level(IO1_LED3_PIN, !IO1_LED3_ACTIVE);
				ssd1306_write_text("Temperature sensor:");
			}
			/* Light mode. */
			else if (app_mode == 1)
			{
				ioport_set_pin_level(IO1_LED2_PIN, IO1_LED2_ACTIVE);
				ioport_set_pin_level(IO1_LED1_PIN, !IO1_LED1_ACTIVE);
				ioport_set_pin_level(IO1_LED3_PIN, !IO1_LED3_ACTIVE);
				ssd1306_write_text("Light sensor:");
			}
			/* SD mode. */
			else
			{
				ioport_set_pin_level(IO1_LED3_PIN, IO1_LED3_ACTIVE);
				ioport_set_pin_level(IO1_LED1_PIN, !IO1_LED1_ACTIVE);
				ioport_set_pin_level(IO1_LED2_PIN, !IO1_LED2_ACTIVE);

				sd_listing_pos = 0;
				display_sd_info();
			}

			app_mode_switch = 0;

		}

		// Print temperature in text format.
		if (app_mode == 0)
		{
			sprintf(value_disp, "%d", (uint8_t)temp);
			ssd1306_set_column_address(95);
			ssd1306_write_command(SSD1306_CMD_SET_PAGE_START_ADDRESS(0));
			ssd1306_write_text(" ");
			// Avoid character overlapping.
			if (temp < 10)
				ssd1306_clear_char();
			ssd1306_write_text(value_disp);
			// Display degree symbol.
			ssd1306_write_data(0x06);
			ssd1306_write_data(0x06);
			ssd1306_write_text("c");

			// Refresh graph.
			ssd1306_draw_graph(0, 1, BUFFER_SIZE, 3, temperature);
		}
#endif

		/* Wait and stop screen flickers. */
		delay_ms(50);
	}
}
