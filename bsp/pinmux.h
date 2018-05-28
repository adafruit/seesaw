/**
 * \file
 *
 * \brief SAM Pin Multiplexer Driver
 *
 * Copyright (C) 2012-2015 Atmel Corporation. All rights reserved.
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
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#ifndef PINMUX_H_INCLUDED
#define PINMUX_H_INCLUDED

// THIS IS A PARED-DOWN VERSION OF PINMUX.H FROM ATMEL ASFCORE 3.
// Please keep original copyright and license intact!

#include "sam.h"
#include <stdint.h> // Includes stdbool and a LOT of other vitals

#ifdef __cplusplus
extern "C" {
#endif

/*@{*/
#if (SAML21) || (SAMC20) || (SAMC21) || defined(__DOXYGEN__)
/** Output Driver Strength Selection feature support */
#  define FEATURE_SYSTEM_PINMUX_DRIVE_STRENGTH
#endif
/*@}*/

/** Peripheral multiplexer index to select GPIO mode for a pin */
#define SYSTEM_PINMUX_GPIO    (1 << 7)

/**
 * \brief Port pin direction configuration enum.
 *
 * Enum for the possible pin direction settings of the port pin configuration
 * structure, to indicate the direction the pin should use.
 */
enum system_pinmux_pin_dir {
	/** The pin's input buffer should be enabled, so that the pin state can
	 *  be read */
	SYSTEM_PINMUX_PIN_DIR_INPUT,
	/** The pin's output buffer should be enabled, so that the pin state can
	 *  be set (but not read back) */
	SYSTEM_PINMUX_PIN_DIR_OUTPUT,
	/** The pin's output and input buffers should both be enabled, so that the
	 *  pin state can be set and read back */
	SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK,
};

/**
 * \brief Port pin input pull configuration enum.
 *
 * Enum for the possible pin pull settings of the port pin configuration
 * structure, to indicate the type of logic level pull the pin should use.
 */
enum system_pinmux_pin_pull {
	/** No logical pull should be applied to the pin */
	SYSTEM_PINMUX_PIN_PULL_NONE,
	/** Pin should be pulled up when idle */
	SYSTEM_PINMUX_PIN_PULL_UP,
	/** Pin should be pulled down when idle */
	SYSTEM_PINMUX_PIN_PULL_DOWN,
};

/**
 * \brief Port pin configuration structure.
 *
 * Configuration structure for a port pin instance. This structure should
 * be initialized by the \ref system_pinmux_get_config_defaults() function
 * before being modified by the user application.
 */
struct system_pinmux_config {
	/** MUX index of the peripheral that should control the pin, if peripheral
	 *  control is desired. For GPIO use, this should be set to
	 *  \ref SYSTEM_PINMUX_GPIO. */
	uint8_t mux_position;

	/** Port buffer input/output direction */
	enum system_pinmux_pin_dir direction;

	/** Logic level pull of the input buffer */
	enum system_pinmux_pin_pull input_pull;

	/** Enable lowest possible powerstate on the pin.
	 *
	 *  \note All other configurations will be ignored, the pin will be disabled.
	 */
	bool powersave;
};

/** \name Configuration and Initialization
 * @{
 */

/**
 * \brief Initializes a Port pin configuration structure to defaults.
 *
 * Initializes a given Port pin configuration structure to a set of
 * known default values. This function should be called on all new
 * instances of these configuration structures before being modified by the
 * user application.
 *
 * The default configuration is as follows:
 *  \li Non peripheral (i.e. GPIO) controlled
 *  \li Input mode with internal pull-up enabled
 *
 * \param[out] config  Configuration structure to initialize to default values
 */
static inline void system_pinmux_get_config_defaults(
		struct system_pinmux_config *const config)
{
	/* Sanity check arguments */
//	Assert(config);

	/* Default configuration values */
	config->mux_position = SYSTEM_PINMUX_GPIO;
	config->direction    = SYSTEM_PINMUX_PIN_DIR_INPUT;
	config->input_pull   = SYSTEM_PINMUX_PIN_PULL_UP;
	config->powersave    = false;
}

void system_pinmux_pin_set_config(
		const uint8_t gpio_pin,
		const struct system_pinmux_config *const config);

/** @} */

/** \name Special Mode Configuration (Physical Group Orientated)
 *  @{
 */

/**
 * \brief Retrieves the PORT module group instance from a given GPIO pin number.
 *
 * Retrieves the PORT module group instance associated with a given logical
 * GPIO pin number.
 *
 * \param[in] gpio_pin  Index of the GPIO pin to convert
 *
 * \return Base address of the associated PORT module.
 */
static inline PortGroup* system_pinmux_get_group_from_gpio_pin(
		const uint8_t gpio_pin)
{
	uint8_t port_index  = (gpio_pin / 128);
	uint8_t group_index = (gpio_pin / 32);

	/* Array of available ports */
	Port *const ports[PORT_INST_NUM] = PORT_INSTS;

	if (port_index < PORT_INST_NUM) {
		return &(ports[port_index]->Group[group_index]);
	} else {
//		Assert(false);
		return ((PortGroup*)0);
	}
}

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */

#endif // PINMUX_H_INCLUDED

