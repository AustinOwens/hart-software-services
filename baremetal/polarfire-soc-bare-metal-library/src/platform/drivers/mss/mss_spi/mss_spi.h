/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * PolarFire SoC (PSE) microprocessor subsystem SPI bare metal software
 * driver public API.
 *
 * SVN $Revision$
 * SVN $Date$
 */
/*=========================================================================*//**
  @mainpage PolarFire SoC MSS SPI Bare Metal Driver.

  @section intro_sec Introduction
  The PolarFire SoC microprocessor subsystem (MSS) includes two serial
  peripheral interface (SPI) peripherals for serial communication. This driver
  provides a set of functions for controlling the MSS SPIs as part of a bare
  metal system where no operating system is available. These drivers can be
  adapted for use as part of an operating system, but the implementation of the
  adaptation layer between this driver and the operating system's driver model
  is outside the scope of this driver.

  @section hw_dependencies Hardware Flow Dependencies
  The configuration of all features of the MSS SPI peripherals is covered by
  this driver with the exception of the PolarFire SoC IOMUX configuration.
  PolarFire SoC allows multiple non-concurrent uses of some external pins through
  IOMUX configuration. This feature allows optimization of external pin usage by
  assigning external pins for use by either the microprocessor subsystem or the
  FPGA fabric. The MSS SPI serial signals are routed through IOMUXs to the
  PolarFire SoC device external pins. The MSS SPI serial signals may also be
  routed through IOMUXs to the PolarFire SoC FPGA fabric.
  The IOMUXs are configured using the PolarFire SoC MSS configurator tool. You
  must ensure that the MSS SPI peripherals are enabled and configured in the
  PolarFire SoC MSS configurator if you wish to use them. For more information
  on IOMUXs, refer to the IOMUX section of the PolarFire SoC microprocessor
  Subsystem (MSS) Users Guide.
  The base address, register addresses and interrupt number assignment for the
  MSS SPI peripherals are defined as constants in the PolarFire SoC HAL.
  You must ensure that the latest PolarFire SoC HAL is included in the
  project settings of the software tool chain used to build your project and
  that it is generated into your project.

  @section theory_op Theory of Operation
The MSS SPI driver functions are grouped in following categories:
    *   Initialization
    *   Configure either master or slave mode
    *   SPI Master frame transfer control
    *   SPI Master block transfer control
    *   SPI Slave frame transfer control
    *   SPI Slave block transfer control
  Frame transfer allows the MSS SPI to write or read up to 32 bits of data in a
  SPI transaction. For example, a frame transfer of 12 bits might be used to
  read the result of ADC conversion from SPI analog to digital converter.
  Block transfer allows the MSS SPI to write and read several bytes in a SPI
  transaction. Block transfer transaction allow the data transfer in multiple of
  8 bits (8, 16, 24, 32 ....). Block transfers are typically used with the
  byte-oriented devices such as SPI FLASH device.


  Initialization
  The MSS SPI driver is initialized through a call to the MSS_SPI_init()
  function. The MSS_SPI_init() function takes only one parameter, a pointer to
  one of two global data structures used by the driver to store state
  information for each MSS SPI. A pointer to these data structures is also used
  as first parameter to any of the driver functions to identify which MSS SPI
  will be used by the called function. The names of these two data structures
  are g_mss_spi0 and g_mss_spi1. Therefore any call to an MSS SPI driver
  function should be of the form MSS_SPI_function_name(&g_mss_spi0, ... ) or
  MSS_SPI_function_name( &g_mss_spi1, ... ).
  The user must reset the MSS SPI core before calling MSS_SPI_init() function.
  The MSS_SPI_init() function resets the specified MSS SPI hardware block and
  clears any pending interrupts from local interrupt controller or PLIC.
  The MSS_SPI_init() function must be called before any other MSS SPI driver
  functions can be called.

  Configuration
  An MSS SPI block can operate either as a master or slave SPI device. There are
  two distinct functions for configuring a MSS SPI block for master or slave
  operations.

  Master configuration
  The MSS_SPI_configure_master_mode() function configures the specified MSS SPI
  block for operations as a SPI master. It must be called once for each remote
  SPI slave device which the MSS SPI block will communicate with. It is used to
  provide the following information about each SPI slave's communication
  characteristics:
    *   The SPI protocol mode
    *   The SPI clock speed
    *   The frame bit length
    *   The SPI overflow handler
  This information is held by the driver and will be used to alter the
  configuration of the MSS SPI block each time a slave is selected through a
  call to MSS_SPI_set_slave_select(). The SPI protocol mode defines the initial
  state of the clock signal at the start of a transaction and which clock edge
  will be used to sample the data signal (Motorola SPI modes), or it defines
  whether the SPI block will operate in Texas Instruments (TI) synchronous
  serial mode or in National Semiconductor (NSC) MICROWIRE mode.
  The MSS_SPI_configure_master_mode() function will register the
  MSS_SPI_overflow_handler() function to the SPI instance,
  MSS_SPI_overflow_handler() function is used by driver in case of buffer
  overflow.

  Slave configuration
  The MSS_SPI_configure_slave_mode() function configures the specified MSS SPI
  block for operations  as a SPI slave. It configures the following SPI
  communication characteristics:
    *   The SPI protocol mode
    *   The frame bit length
    *   The SPI overflow handler
  The SPI protocol mode defines the initial state of the clock signal at the
  start of a transaction and which clock edge will be used to sample the data
  signal (Motorola SPI modes), or it defines whether the SPI block will operate
  in TI synchronous serial mode or in NSC MICROWIRE mode.
  The MSS_SPI_configure_slave_mode() function will register the
  MSS_SPI_overflow_handler() function to the SPI instance,
  MSS_SPI_overflow_handler() function is used by driver in case of buffer
  overflow.

  SPI master frame transfer control
  The following functions are used as part of SPI master frame transfers:
    - MSS_SPI_set_slave_select()
    - MSS_SPI_transfer_frame()
    - MSS_SPI_clear_slave_select()
  The master must first select the target slave through a call to
  MSS_SPI_set_slave_select(). This causes the relevant slave select line to
  become asserted while data is clocked out onto the SPI data line.
  A call is then made to MSS_SPI_transfer_frame() specifying the value of the
  data frame to be sent.
  The function MSS_SPI_clear_slave_select() can be used after the transfer is
  complete to prevent this slave select line from being asserted during
  subsequent SPI transactions. A call to this function is only required if the
  master is communicating with multiple slave devices.

  SPI master block transfer control
  The following functions are used as part of SPI master block transfers:
    - MSS_SPI_set_slave_select()
    - MSS_SPI_clear_slave_select()
    - MSS_SPI_transfer_block()
  The master must first select the target slave through a call to
  MSS_SPI_set_slave_select(). This causes the relevant slave select line to
  become asserted while data is clocked out onto the SPI data line.
  A call is then made to MSS_SPI_transfer_block (). The parameters of
  this function specify:
    - the number of bytes to be transmitted
    - a pointer to the buffer containing the data to be transmitted
    - the number of bytes to be received
    - a pointer to the buffer where received data will be stored
  The number of bytes to be transmitted can be set to zero to indicate that the
  transfer is purely a block read transfer. Alternatively, the number of bytes
  to be received can be set to zero to specify that the transfer is purely a
  block write transfer.
  The function MSS_SPI_clear_slave_select() can be used after the transfer is
  complete to prevent this slave select line from being asserted during
  subsequent SPI transactions. A call to this function is only required if the
  master is communicating with multiple slave devices.
  Note: Unlike in previous versions of this driver, the SPS bit is set in the
        CONTROL register in Motorola modes so that the Slave Select line remains
        asserted throughout block transfers.

  SPI slave frame transfer control
  The following functions are used as part of SPI slave frame transfers:
    - MSS_SPI_set_slave_tx_frame()
    - MSS_SPI_set_frame_rx_handler()
  The MSS_SPI_set_slave_tx_frame() function specifies the frame data that will
  be returned to the SPI master. The frame data specified through this function
  is the value that will be read over the SPI bus by the remote SPI master when
  it initiates a transaction. A call to MSS_SPI_set_slave_tx_frame() is only
  required if the MSS SPI slave is the target of SPI  read transactions, i.e. if
  data is meant to be read from the PolarFire SoC device over SPI.
  The MSS_SPI_set_frame_rx_handler() function specifies the receive handler
  function that will called when a frame of data has been received by the MSS
  SPI when it is configured as a slave. The receive handler function specified
  through this call will process the frame data written, over the SPI bus, to
  the MSS SPI slave by the remote SPI master. The receive handler function must
  be implemented as part of the application. It is only required if the MSS SPI
  slave is the target of SPI frame write transactions.
  Successive master writes need to take into account the time taken to execute
  the receive handler if the interface is to work reliably.
  SPI slave block transfer control
  The following functions are used as part of SPI slave block transfers:
    - MSS_SPI_set_slave_block_buffers()
    - MSS_SPI_set_cmd_handler()
    - MSS_SPI_set_cmd_response()
  The MSS_SPI_set_slave_block_buffers() function is used to configure a MSS SPI
  slave for block transfer operations. It specifies:
    - The buffer containing the data that will be returned to the remote SPI
      master
    - The buffer where data received from the remote SPI master will be stored
    - The handler function that will be called after the receive buffer has been
      filled
  The MSS_SPI_set_cmd_handler() function specifies a command handler function
  that will be called by the driver once a specific number of bytes has been
  received after the SPI chip select signal becoming active. The number of bytes
  making up the command part of the transaction is specified as part of the
  parameters to MSS_SPI_set_cmd_handler(). The command handler function
  is implemented as part of the application making use of the SPI driver and
  would typically call the MSS_SPI_set_cmd_response() function.
  The MSS_SPI_set_cmd_response() function specifies the data that will be
  returned to the master. Typically the MSS_SPI_set_slave_block_buffers() will
  have been called as part of the system initialization to specify the data sent
  to the master while the command bytes are being received. The transmit buffer
  specified through the call to MSS_SPI_set_slave_block_buffers() would also
  typically include one or more bytes allowing for the turn around time required
  for the command handler function to execute and call
  MSS_SPI_set_cmd_response().

 *//*=========================================================================*/
#ifndef MSS_SPI_H_
#define MSS_SPI_H_

#include <stddef.h>
#include <stdint.h>
#include "mss_plic.h"

/*Register map of the PSE MSS SPI*/
typedef struct
{
    volatile uint32_t CONTROL;
    volatile uint32_t FRAMESIZE;/* SPEC 2.13 , removed TXRXDF_SIZE;*/
    volatile uint32_t STATUS;
    volatile uint32_t INT_CLEAR;
    volatile uint32_t RX_DATA;
    volatile uint32_t TX_DATA;
    volatile uint32_t CLK_GEN;
    volatile uint32_t SLAVE_SELECT;
    volatile uint32_t MIS;
    volatile uint32_t RIS;
    volatile uint32_t CONTROL2;
    volatile uint32_t COMMAND;
    volatile uint32_t PKTSIZE;
    volatile uint32_t CMDSIZE;
    volatile uint32_t HWSTATUS;
    volatile uint32_t STAT8;
    volatile uint32_t CTRL0;
    volatile uint32_t CTRL1;
    volatile uint32_t CTRL2;
    volatile uint32_t CTRL3;
    volatile uint32_t FRAMESUP; /* SPEC 2.13 */
} SPI_TypeDef;

#define SPI0_LO_BASE               0x20108000u
#define SPI1_LO_BASE               0x20109000u
#define SPI0_HI_BASE               0x28108000u
#define SPI1_HI_BASE               0x28109000u

#define MSS_SPI0_LO_BASE                    ((SPI_TypeDef *) SPI0_LO_BASE)
#define MSS_SPI1_LO_BASE                    ((SPI_TypeDef *) SPI1_LO_BASE)
#define MSS_SPI0_HI_BASE                    ((SPI_TypeDef *) SPI0_HI_BASE)
#define MSS_SPI1_HI_BASE                    ((SPI_TypeDef *) SPI1_HI_BASE)

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
  This defines the function prototype that must be followed by MSS SPI slave
  frame receive handler functions. These functions are registered with the MSS
  SPI driver through the MSS_SPI_set_frame_rx_handler () function.

  Declaring and Implementing Slave Frame Receive Handler Functions:
    Slave frame receive handler functions should follow the following prototype:
        void slave_frame_receive_handler ( uint32_t rx_frame );
    The actual name of the receive handler is unimportant. You can use any name
    of your choice for the receive frame handler. The rx_frame parameter will
    contain the value of the received frame.
 */
typedef void (*mss_spi_frame_rx_handler_t)( uint32_t rx_frame );

/***************************************************************************//**
  This defines the function prototype that must be followed by MSS SPI slave
  block receive handler functions. These functions are registered with the MSS
  SPI driver through the MSS_SPI_set_slave_block_buffers() function.

  Declaring and Implementing Slave Block Receive Handler Functions
    Slave block receive handler functions should follow the following prototype:
        void mss_spi_block_rx_handler ( uint8_t * rx_buff, uint16_t rx_size );
    The actual name of the receive handler is unimportant. You can use any name
    of your choice for the receive frame handler. The rx_buff parameter will
    contain a pointer to the start of the received block. The rx_size parameter
    indicates the number of bytes in the received block.

 */
typedef void (*mss_spi_block_rx_handler_t)(uint8_t * rx_buff, uint32_t rx_size);

/**************************************************************************//**
  This defines the function prototype that must be used by the MSS SPI init
  functions. This handler functions is registered with the MSS SPI driver
  through the MSS_SPI_init() function.
  Declaring and Implementing Overflow handler functions
     The buffer overflow handler functions must use the following prototype
          void mss_spi0_overflow_handler(uint8_t mss_spi_core);
     The actual name of the overflow handler is not important. User can use any
     name of choice. This function is passed as a argument to the MSS_SPI_init(),
     which registers overflow handler to the MSS SPI instance.The parameter
     passed with the function informs the handler function about which SPI core
     to reset.This variable can later be used by the driver to recover from
     buffer overflow situations.
 */
typedef void (*mss_spi_oveflow_handler_t)(uint8_t mss_spi_core);

/***************************************************************************//**
  This enumeration is used to define the settings for the SPI protocol mode
  bits which select the different modes of operation for the MSS SPI. It is used
  as a parameter to the MSS_SPI_configure_master_mode() and
  MSS_SPI_configure_slave_mode() functions.

  - MSS_SPI_MODE0:
        Clock starts low, data read on clock's rising edge, data changes on
        falling edge.

  - MSS_SPI_MODE1:
        Clock starts low, data read on clock's falling edge, data changes on
        rising edge.

  - MSS_SPI_MODE2:
        Clock starts high, data read on clock's falling edge, data changes on
        rising edge.

  - MSS_SPI_MODE3:
        Clock starts high, data read on clock's rising edge, data changes on
        falling edge.

  - MSS_TI_MODE:
        TI synchronous serial mode. Slave select is pulsed at start of transfer.

  - MSS_NSC_MODE:
        NSC Microwire mode.
 */
typedef enum __mss_spi_protocol_mode_t
{
    MSS_SPI_MODE0    = 0x00000000,
    MSS_SPI_TI_MODE  = 0x01000004,
    MSS_SPI_NSC_MODE = 0x00000008,
    MSS_SPI_MODE2    = 0x01000000,
    MSS_SPI_MODE1    = 0x02000000,
    MSS_SPI_MODE3    = 0x03000000
} mss_spi_protocol_mode_t;

/***************************************************************************//**
 This enumeration is used to select a specific SPI slave device (0 to 7). It is
 used as a parameter to the MSS_SPI_configure_master_mode(),
 MSS_SPI_set_slave_select() and MSS_SPI_clear_slave_select () functions.
 */
 typedef enum __mss_spi_slave_t
 {
    MSS_SPI_SLAVE_0             = 0,
    MSS_SPI_SLAVE_1             = 1,
    MSS_SPI_SLAVE_2             = 2,
    MSS_SPI_SLAVE_3             = 3,
    MSS_SPI_SLAVE_4             = 4,
    MSS_SPI_SLAVE_5             = 5,
    MSS_SPI_SLAVE_6             = 6,
    MSS_SPI_SLAVE_7             = 7,
    MSS_SPI_MAX_NB_OF_SLAVES    = 8
} mss_spi_slave_t;

/***************************************************************************//**
  This constant defines a frame size of 8 bits when configuring an MSS SPI to
  perform block transfer data transactions.
  It must be used as the value for the frame_bit_length parameter of function
  MSS_SPI_configure_master_mode() when performing block transfers between the
  MSS SPI master and the target SPI slave.
  It must also be used as the value for the frame_bit_length parameter of
  MSS_SPI_configure_slave_mode() when performing block transfers between the
  MSS SPI slave and the remote SPI master.
 */
#define MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE   8u

/***************************************************************************//**
  The mss_spi_slave_cfg_t holds the MSS SPI configuration that must be used to
  communicate with a specific SPI slave.
 */
typedef struct __mss_spi_slave_cfg_t
{
    uint32_t ctrl_reg;
    uint8_t txrxdf_size_reg;
    uint8_t clk_gen;
} mss_spi_slave_cfg_t;

/***************************************************************************//**
 This enumeration is used to indicate the current slave mode transfer type so
 that we are not relying on buffer comparisons to dictate the logic of the driver.
 */
typedef enum __mss_spi_sxfer_mode_t
{
    MSS_SPI_SLAVE_XFER_NONE  = 0, /* Not configured yet */
    MSS_SPI_SLAVE_XFER_BLOCK = 1, /* Block transfers, with SSEND delimiting end of block */
    MSS_SPI_SLAVE_XFER_FRAME = 2  /* Single frame transfers */
} mss_spi_sxfer_mode_t;

/***************************************************************************//**
  There is one instance of this structure for each of the microprocessor
  subsystem's SPIs. Instances of this structure are used to identify a specific
  SPI. A pointer to an instance of the mss_spi_instance_t structure is passed as
  the first parameter to MSS SPI driver functions to identify which SPI should
  perform the requested operation.
 */
typedef struct __mss_spi_instance_t
{
    /* SPI hardware identification definitions . */
    SPI_TypeDef *           hw_reg;     /*!< Pointer to SPI registers. */
    PLIC_IRQn_Type          irqn;       /*!< SPI's PLIC interrupt number. */

    /* Internal transmit state: */
    const uint8_t * slave_tx_buffer;    /*!< Pointer to slave transmit buffer. */
    uint32_t slave_tx_size;             /*!< Size of slave transmit buffer. */
    uint32_t slave_tx_idx;              /*!< Current index into slave transmit buffer. */

    /* Slave command response buffer: */
    const uint8_t * resp_tx_buffer;
    uint32_t resp_buff_size;
    uint32_t resp_buff_tx_idx;
    mss_spi_block_rx_handler_t cmd_handler;
    uint32_t cmd_done;                  /*!< Flag which indicates response has been set up and
                                             it is safe to pad with 0s once the response is sent. */

    /* Internal receive state: */
    uint8_t * slave_rx_buffer;          /*!< Pointer to buffer where data received by a slave will be stored. */
    uint32_t slave_rx_size;             /*!< Slave receive buffer siSze. */
    uint32_t slave_rx_idx;              /*!< Current index into slave receive buffer. */

    /* Configuration for each target slave. */
    mss_spi_slave_cfg_t slaves_cfg[MSS_SPI_MAX_NB_OF_SLAVES];

    /* Slave received frame handler: */
    mss_spi_frame_rx_handler_t frame_rx_handler;    /*!< Pointer to function that will be called when a frame is received when the SPI block is configured as slave. */

    uint32_t slave_tx_frame;                        /*!< Value of the data frame that will be transmitted when the SPI block is configured as slave. */

    /* Slave block rx handler: */
    mss_spi_block_rx_handler_t block_rx_handler;    /*!< Pointer to the function that will be called when a data block has been received. */

    /* How we are expecting to deal with slave transfers */
    mss_spi_sxfer_mode_t slave_xfer_mode;               /*!< Current slave mode transfer configuration. */

    /* MSS SPI reset handler*/
    mss_spi_oveflow_handler_t buffer_overflow_handler;

} mss_spi_instance_t;


/***************************************************************************//**
  This instance of mss_spi_instance_t holds all data related to the operations
  performed by the MSS SPI. The function MSS_SPI_init() initializes this structure.
  A pointer to g_mss_spi0_lo is passed as the first parameter to MSS SPI driver
  functions to indicate that SPI0 should perform the requested operation.
 */

extern mss_spi_instance_t g_mss_spi0_lo;
extern mss_spi_instance_t g_mss_spi1_lo;

extern mss_spi_instance_t g_mss_spi0_hi;
extern mss_spi_instance_t g_mss_spi1_hi;
/***************************************************************************//**
  The MSS_SPI_init() function initializes and hardware and data structures of
  one of the MSS SPIs. The MSS_SPI_init() function must be called before any
  other MSS SPI driver functions can be called.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to be initialized. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  Example:
  @code
  MSS_SPI_init(&g_mss_spi0);
  @endcode
 */
void MSS_SPI_init
(
    mss_spi_instance_t * this_spi
);

/***************************************************************************//**
  The MSS_SPI_configure_slave_mode() function configure a MSS SPI block for
  operations as a slave SPI device. It configures the SPI hardware with the
  selected SPI protocol mode and frame size for communication with a specific
  SPI master.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to be configured. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  @param protocol_mode
    This parameter is used to specify the SPI operating mode. Allowed values are:
        - MSS_SPI_MODE0
        - MSS_SPI_MODE1
        - MSS_SPI_MODE2
        - MSS_SPI_MODE3
        - MSS_TI_MODE
        - MSS_NSC_MODE

  @param frame_bit_length
    Number of bits making up the frame. You must use the
    MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE constant as the value for frame_bit_length
    when configuring the MSS SPI master for block transfer transactions with the
    target SPI slave.

  @param recieve_buffer_overflow_handler
  The recieve_buffer_overflow_handler parameter is a pointer to the callback
  function which is called when rx overflow occurs. The
  MSS_SPI_configure_master_mode() function registers user implemented receive
  buffer overflow handler to the mss_spi_instance_t structure. This registered
  handler is used by driver to recover from buffer overflow situation.
      Note: There is no restriction in naming the buffer overflow handler,
            User can name as per wish.


  Example:
  @code
  MSS_SPI_init(&g_mss_spi0);
  MSS_SPI_configure_slave_mode
    (
        &g_mss_spi0,
        MSS_SPI_MODE2,
        MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
        mss_spi_overflow_handler
    );
  @endcode

 */
void MSS_SPI_configure_slave_mode
(
    mss_spi_instance_t * this_spi,
    mss_spi_protocol_mode_t protocol_mode,
    uint8_t frame_bit_length,
    mss_spi_oveflow_handler_t recieve_buffer_overflow_handler
);

/***************************************************************************//**
  The MSS_SPI_configure_master_mode() function configures the protocol mode,
  serial clock speed and frame size for a specific target SPI slave device. It
  is used when the MSS SPI hardware block is used as a SPI master. This function
  must be called once for each target SPI slave which the MSS SPI master is
  wishes to communicate with. The SPI master hardware will be configured with
  the configuration specified by this function during calls to
  MSS_SPI_set_slave_select().

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to be configured. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  @param slave
    The slave parameter is used to identify a target SPI slave. The driver will
    hold the MSS SPI master configuration required to communicate with this
    slave, as specified by the other function parameters. Allowed values are:
       * MSS_SPI_SLAVE_0
       * MSS_SPI_SLAVE_1
       * MSS_SPI_SLAVE_2
       * MSS_SPI_SLAVE_3
       * MSS_SPI_SLAVE_4
       * MSS_SPI_SLAVE_5
       * MSS_SPI_SLAVE_6
       * MSS_SPI_SLAVE_7

  @param protocol_mode
    This parameter is used to specify the SPI operating mode. Allowed values are:
       * MSS_SPI_MODE0
       * MSS_SPI_MODE1
       * MSS_SPI_MODE2
       * MSS_SPI_MODE3
       * MSS_SPI_TI_MODE
       * MSS_SPI_NSC_MODE

  @param clk_div
    SPI clock divider value used to generate serial interface clock signal from
    PCLK. Allowed values are even numbers in the range from 2 to 512. The PCLK
    frequency is divided by the specified value to give the serial interface
    clock frequency.

  @param frame_bit_length
    Number of bits making up the frame. The maximum frame length is 32 bits. You
    must use the MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE constant as the value for
    frame_bit_length when configuring the MSS SPI master for block transfer
    transactions with the target SPI slave.

  @param recieve_buffer_overflow_handler
    The recieve_buffer_overflow_handler parameter is a pointer to the callback
    function receive buffer overflow handler. User must implement a local
    function to handle buffer overflow. The MSS_SPI_configure_master_mode()
    function registers user implemented receive buffer overflow handler to
    the mss_spi_instance_t structure. This registered handler is used by driver
    to prevent in state of buffer overflow.

    Note: There is no limitation in naming the buffer overflow handler, User
          can name as per wish  .

  Example:
  @code
  MSS_SPI_init(&g_mss_spi0);

  MSS_SPI_configure_master_mode
    (
        &g_mss_spi0,
        MSS_SPI_SLAVE_0,
        MSS_SPI_MODE2,
        64u,
        12,
        mss_spi_overflow_handler
     );

  MSS_SPI_configure_master_mode
    (
        &g_mss_spi0,
        MSS_SPI_SLAVE_1,
        MSS_SPI_TI_MODE,
        128u,
        MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
        mss_spi_overflow_handler
     );

  MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  MSS_SPI_transfer_frame(&g_mss_spi0, 0xaaa);
  MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);

  MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_1);
  MSS_SPI_transfer_frame(&g_mss_spi0, 0x55);
  MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_1);
  @endcode
 */
void MSS_SPI_configure_master_mode
(
    mss_spi_instance_t *    this_spi,
    mss_spi_slave_t         slave,
    mss_spi_protocol_mode_t protocol_mode,
    uint32_t                clk_div,
    uint8_t                 frame_bit_length,
    mss_spi_oveflow_handler_t recieve_buffer_overflow_handler
);
/*==============================================================================
 * Master functions
 *============================================================================*/

/***************************************************************************//**
  The MSS_SPI_set_slave_select() function is used by a MSS SPI master to select
  a specific slave. This function causes the relevant slave select signal to be
  asserted while data is clocked out onto the SPI data line. This function also
  configures the MSS SPI master with the configuration settings necessary for
  communication with the specified slave. These configuration settings must be
  specified in a previous call to the MSS_SPI_configure_master_mode() function.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  @param slave
    The slave parameter is one of the mss_spi_slave_t enumerated constants
    identifying the slave.

  Example:
  @code
  const uint8_t frame_size = 25;
  const uint32_t master_tx_frame = 0x0100A0E1;

  MSS_SPI_init(&g_mss_spi0);
  MSS_SPI_configure_master_mode
    (
        &g_mss_spi0,
        MSS_SPI_SLAVE_0,
        MSS_SPI_MODE1,
        256u,
        frame_size,
        mss_spi_overflow_handler
     );

  MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  MSS_SPI_transfer_frame(&g_mss_spi0, master_tx_frame);
  MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  @endcode
 */
void MSS_SPI_set_slave_select
(
    mss_spi_instance_t * this_spi,
    mss_spi_slave_t slave
);

/***************************************************************************//**
  The MSS_SPI_clear_slave_select() function is used by a MSS SPI Master to
  deselect a specific slave. This function causes the relevant slave select
  signal to be de-asserted.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.


  @param slave
    The slave parameter is one of mss_spi_slave_t enumerated constants
    identifying a slave.

  Example:
  @code
  const uint8_t frame_size = 25;
  const uint32_t master_tx_frame = 0x0100A0E1;

  MSS_SPI_init(&g_mss_spi0);
  MSS_SPI_configure_master_mode
    (
        &g_mss_spi0,
        MSS_SPI_SLAVE_0,
        MSS_SPI_MODE1,
        256u,
        frame_size,
        mss_spi_overflow_handler
     );
  MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  MSS_SPI_transfer_frame(&g_mss_spi0, master_tx_frame);
  MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  @endcode
 */
void MSS_SPI_clear_slave_select
(
    mss_spi_instance_t * this_spi,
    mss_spi_slave_t slave
);

/***************************************************************************//**
  The MSS_SPI_disable() function is used to temporarily disable a MSS SPI
  hardware block.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  Example:
  @code
  uint32_t transfer_size;
  uint8_t tx_buffer[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

  transfer_size = sizeof(tx_buffer);

  MSS_SPI_disable(&g_mss_spi0);
  MSS_SPI_set_transfer_byte_count(&g_mss_spi0, transfer_size);

  @endcode
 */
void MSS_SPI_disable
(
    mss_spi_instance_t * this_spi
);

/***************************************************************************//**
  The MSS_SPI_enable() function is used to re-enable a MSS SPI hardware block
  after it was disabled using the SPI_disable() function.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  Example:
  @code
  uint32_t transfer_size;
  uint8_t tx_buffer[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

  transfer_size = sizeof(tx_buffer);

  MSS_SPI_disable(&g_mss_spi0);
  MSS_SPI_set_transfer_byte_count(&g_mss_spi0, transfer_size);

  MSS_SPI_enable(&g_mss_spi0);

  @endcode
 */
void MSS_SPI_enable
(
    mss_spi_instance_t * this_spi
);

/***************************************************************************//**
  The MSS_SPI_transfer_frame() function is used by a MSS SPI master to transmit
  and receive a frame up to 32 bits long. This function is typically used for
  transactions with a SPI slave where the number of transmit and receive bits is
  not divisible by 8.

  Note: The maximum frame size in NSC Microwire mode is 24 bits organized as an
        8 bit command followed by up to 16 bits of data .
  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.


  @param tx_bits
    The tx_bits parameter is a 32 bits word containing the data that will be
    transmitted.
    Note:   The bit length of the value to be transmitted to the slave must be
            specified as the frame_bit_length parameter in a previous call to
            the MSS_SPI_configure_master_mode() function.

  @return
    This function returns a 32 bits word containing the value that is received
    from the slave.

  Example:
  @code
      const uint8_t frame_size = 25;
      const uint32_t master_tx_frame = 0x0100A0E1;
      uint32_t master_rx;

      MSS_SPI_init(&g_mss_spi0);
      MSS_SPI_configure_master_mode
        (
            &g_mss_spi0,
            MSS_SPI_SLAVE_0,
            MSS_SPI_MODE1,
            256u,
            frame_size,
            mss_spi_overflow_handler
         );

      MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
      master_rx = MSS_SPI_transfer_frame(&g_mss_spi0, master_tx_frame);
      MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  @endcode
 */
uint32_t MSS_SPI_transfer_frame
(
    mss_spi_instance_t * this_spi,
    uint32_t tx_bits
);

/***************************************************************************//**
  The MSS_SPI_transfer_block() function is used by MSS SPI masters to transmit
  and receive blocks of data organized as a specified number of bytes. It can
  be used for:
    *   Writing a data block to a slave
    *   Reading a data block from a slave
    *   Sending a command to a slave followed by reading the response to the
        command in a single SPI transaction.


  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  @param cmd_buffer
    The cmd_buffer parameter is a pointer to the buffer containing the data that
    will be sent by the master from the beginning of the transfer.

  @param cmd_byte_size
    The cmd_byte_size parameter specifies the number of bytes contained in
    cmd_buffer that will be sent. A value of 0 indicates that no data needs to
    be sent to the slave.

  @param rd_buffer
    The rd_buffer parameter is a pointer to the buffer where the data received
    from the slave after the command has been sent will be stored.

  @param rd_byte_size
    The rd_byte_size parameter specifies the number of bytes to be received from
    the slave and stored in the rd_buffer. A value of 0 indicates that no data
    is to be read from the slave.

  Polled write transfer example:
  @code
      uint8_t master_tx_buffer[MASTER_TX_BUFFER] =
      {
          0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A
      };
      MSS_SPI_init(&g_mss_spi0);
      MSS_SPI_configure_master_mode
        (
            &g_mss_spi0,
            MSS_SPI_SLAVE_0,
            MSS_SPI_MODE1,
            256u,
            MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
            mss_spi_overflow_handler
         );

      MSS_SPI_set_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
      MSS_SPI_transfer_block
        (
            &g_mss_spi0,
            master_tx_buffer,
            sizeof(master_tx_buffer),
            0,
            0
        );
      MSS_SPI_clear_slave_select(&g_mss_spi0, MSS_SPI_SLAVE_0);
  @endcode
 */
void MSS_SPI_transfer_block
(
    mss_spi_instance_t * this_spi,
    const uint8_t cmd_buffer[],
    uint32_t cmd_byte_size,
    uint8_t rd_buffer[],
    uint32_t rd_byte_size
);

/*==============================================================================
 * Slave functions
 *============================================================================*/

/***************************************************************************//**
  The MSS_SPI_set_frame_rx_handler() function is used by MSS SPI slaves to
  specify the receive handler function that will be called by the MSS SPI driver
  interrupt handler when a a frame of data is received by the MSS SPI slave.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.


  @param rx_handler
    The rx_handler parameter is a pointer to the frame receive handler that must
    be called when a frame is received by the MSS SPI slave.

  Example:
  @code
      uint32_t g_slave_rx_frame = 0;

      void slave_frame_handler(uint32_t rx_frame)
      {
          g_slave_rx_frame = rx_frame;
      }

      int setup_slave(void)
      {
          const uint16_t frame_size = 25;
          MSS_SPI_init(&g_mss_spi1);
          MSS_SPI_configure_slave_mode
            (
                &g_mss_spi0,
                MSS_SPI_MODE2,
                frame_size,
                mss_spi_overflow_handler
            );
          MSS_SPI_set_frame_rx_handler(&g_mss_spi1, slave_frame_handler);
      }
  @endcode
 */
void MSS_SPI_set_frame_rx_handler
(
    mss_spi_instance_t * this_spi,
    mss_spi_frame_rx_handler_t rx_handler
);

/***************************************************************************//**
  The MSS_SPI_set_slave_tx_frame() function is used by MSS SPI slaves to specify
  the frame that will be transmitted when a transaction is initiated by the SPI
  master.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.


  @param frame_value
    The frame_value parameter contains the value of the frame to be sent to the
    master.
    Note:   The bit length of the value to be transmitted to the master must be
            specified as the frame_bit_length parameter in a previous call to
            the MSS_SPI_configure_slave_mode() function.

  Example:
  @code
      const uint16_t frame_size = 25;
      const uint32_t slave_tx_frame = 0x0110F761;
      uint32_t master_rx;

      MSS_SPI_init(&g_mss_spi1);
      MSS_SPI_configure_slave_mode
        (
            &g_mss_spi0,
            MSS_SPI_MODE2,
            frame_size,
            mss_spi_overflow_handler
        );
      MSS_SPI_set_slave_tx_frame(&g_mss_spi1, slave_tx_frame);
  @endcode
 */
void MSS_SPI_set_slave_tx_frame
(
    mss_spi_instance_t * this_spi,
    uint32_t frame_value
);

/***************************************************************************//**
  The MSS_SPI_set_slave_block_buffers() function is used to configure an MSS
  SPI slave for block transfer operations. It specifies one or more of the
  following:
    - The data that will be transmitted when accessed by a master.
    - The buffer where data received from a master will be stored.
    - The handler function that must be called after the receive buffer has been
      filled.
    - The maximum number of bytes that the slave will accept from the master
      (excess bytes are discarded).
  These parameters allow the following use cases:
    - Slave performing an action after receiving a block of data from a master
      containing a command. The action will be performed by the receive handler
      based on the content of the receive data buffer.
    - Slave returning a block of data to the master. The type of information is
      always the same but the actual values change over time. For example,
      returning the voltage of a predefined set of analog inputs.
    - Slave returning data based on a command contained in the first part of the
      SPI transaction. For example, reading the voltage of the analog input
      specified by the first data byte by the master. This is achieved by using
      the MSS_SPI_set_slave_block_buffers() function in conjunction with
      functions MSS_SPI_set_cmd_handler() and MSS_SPI_set_cmd_response(). Please
      refer to the MSS_SPI_set_cmd_handler() function description for details of
      this use case.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to operate on. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.

  @param tx_buffer
    The tx_buffer parameter is a pointer to a buffer containing the data that
    will be sent to the master. This parameter can be set to 0 if the MSS SPI
    slave is not intended to be the target of SPI read.

  @param tx_buff_size
    The tx_buff_size parameter specifies the number of bytes that will be
    transmitted by the SPI slave. It is the number of bytes contained in the
    tx_buffer. This parameter can be set to 0 if the MSS SPI slave is not
    intended to be the target of SPI read transactions.

  @param rx_buffer
    The rx_buffer parameter is a pointer to the buffer where data received from
    the master will be stored. This parameter can be set to 0 if the MSS SPI
    slave is not intended to be the target of SPI write or write-read
    transactions.

  @param rx_buff_size
    The rx_buff_size parameter specifies the size of the receive buffer. It is
    also the number of bytes that must be received before the receive handler
    is called, if a receive handler is specified using the block_rx_handler
    parameter. This parameter can be set to 0 if the MSS SPI slave is not
    intended to be the target of SPI write or write-read transactions.

  @param block_rx_handler
    The block_rx_handler parameter is a pointer to a function that will be
    called when the receive buffer has been filled. This parameter can be set to
    0 if the MSS SPI slave is not intended to be the target of SPI write or
    write-read transactions.

  Slave performing operation based on master command:
  In this example the SPI slave is configured to receive 10 bytes of data or
  command from the SPI master and process the data received.
  @code
     uint32_t nb_of_rx_handler_calls = 0;

     void spi1_block_rx_handler_b
     (
         uint8_t * rx_buff,
         uint16_t rx_size
     )
     {
         ++nb_of_rx_handler_calls;
     }

     void setup_slave(void)
     {
         uint8_t slave_rx_buffer[10] =
         {
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
         };

         MSS_SPI_init(&g_mss_spi1);
         MSS_SPI_configure_slave_mode
            (
                &g_mss_spi0,
                MSS_SPI_MODE2,
                MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
                mss_spi_overflow_handler
            );

         MSS_SPI_set_slave_block_buffers
             (
                 &g_mss_spi1,
                 0,
                 0,
                 slave_rx_buffer,
                 sizeof(slave_rx_buffer),
                 spi1_block_rx_handler_b
             );
     }
  @endcode
 */
void MSS_SPI_set_slave_block_buffers
(
    mss_spi_instance_t * this_spi,
    const uint8_t * tx_buffer,
    uint32_t tx_buff_size,
    uint8_t * rx_buffer,
    uint32_t rx_buff_size,
    mss_spi_block_rx_handler_t spi_block_rx_handler
);


/***************************************************************************//**
  The MSS_SPI_set_cmd_handler() function specifies a command handler function
  that will be called when the number of bytes received reaches the command size
  specified as parameter.
  This function is used by SPI slaves performing block transfers. Its purpose is
  to allow a SPI slave to decide the data that will be returned to the master
  while a SPI transaction is taking place. Typically, one of more command bytes
  are sent by the master to request some specific data. The slave interprets the
  command byte(s) while one or more turn-around bytes are transmitted. The
  slave adjusts its transmit data buffer based on the command during the
  turnaround time.
  The diagram below provides an example of the use of this function where the
  SPI slave returns data bytes D0 to D6 based on the value of a command. The
  3 bytes long command is made up of a command opcode byte followed by an address
  byte followed by a size byte. The cmd_handler() function specified through an
  earlier call to MSS_SPI_set_cmd_handler() is called by the SPI driver once the
  third byte is received. The cmd_handler() function interprets the command bytes
  and calls MSS_SPI_set_cmd_response() to set the SPI slave's response transmit
  buffer with the data to be transmitted after the turnaround bytes (T0 to T3).
  The number of turnaround bytes must be sufficient to give enough time for the
  cmd_handler() to execute. The number of turnaround bytes is specified by the
  protocol used on top of the SPI transport layer, so both the master and slave
  must adhere to this.

    t0          t1       t2         t3                                 t4
    |           |        |          |                                  |
    |------------------------------------------------------------------|
    |  COMMAND  |    TURN-AROUND    |               DATA               |
    |------------------------------------------------------------------|
    | C | A | S | T0 | T1 | T2 | T4 | D0 | D1 | D2 | D3 | D4 | D5 | D6 |
    |------------------------------------------------------------------|
                |
                |
                --> cmd_handler() called here.
                         |
                         |
                         --> MSS_SPI_set_cmd_response() called here by
                             implementation of cmd_handler() to set the data
                             that will be transmitted by the SPI slave.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block used. There are two such data
    structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and MSS SPI
    1 respectively. This parameter must point to either the g_mss_spi0 or
    g_mss_spi1 global data structure defined within the SPI driver.

  @param cmd_handler
    The cmd_handler parameter is a pointer to a function with prototype:
        void cmd_handler(uint8_t * rx_buff, uint32_t rx_size);
    It specifies the function that will be called when the number of bytes
    specified by parameter cmd_size has been received.

    Passing in a null pointer for this disables the command handler and the
    associated interrupt.

 @param cmd_size
    The cmd_size parameter specifies the number of bytes that must be received
    before the command handler function specified by cmd_handler is called.

  The example below demonstrates how to configure SPI1 to implement the protocol
  given as example in the diagram above.
  The configure_slave() function configures SPI1. It sets the receive and transmit
  buffers. The transmit buffer specified through the call to
  MSS_SPI_set_slave_block_buffers() specifies the data that will be returned to
  the master in bytes between t0 and t3. These are the bytes that will be sent
  to the master while the master transmits the command and dummy bytes.
  The spi1_slave_cmd_handler() function will be called by the driver at time t1
  after the 3 command bytes have been received.
  The spi1_block_rx_handler() function will be called by the driver at time t4
  when the transaction completes when the slave select signal becomes de-asserted.

  @code
    #define COMMAND_SIZE            3
    #define NB_OF_DUMMY_BYTES       4
    #define MAX_TRANSACTION_SIZE    16

    uint8_t slave_tx_buffer[COMMAND_SIZE + NB_OF_DUMMY_BYTES];
    uint8_t slave_rx_buffer[MAX_TRANSACTION_SIZE];

    void configure_slave(void)
    {
        MSS_SPI_init(&g_mss_spi1);

        MSS_SPI_configure_slave_mode
          (
            &g_mss_spi1,
            MSS_SPI_MODE1,
            MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
            mss_spi_overflow_handler
          );

        MSS_SPI_set_slave_block_buffers
            (
                &g_mss_spi1,
                slave_tx_buffer,
                COMMAND_SIZE + NB_OF_DUMMY_BYTES,
                slave_rx_buffer,
                sizeof(slave_rx_buffer),
                spi1_block_rx_handler
            );


        MSS_SPI_set_cmd_handler
            (
                &g_mss_spi1,
                spi1_slave_cmd_handler,
                COMMAND_SIZE
            );
    }

    void spi1_slave_cmd_handler
    (
        uint8_t * rx_buff,
        uint32_t rx_size
    )
    {
        uint8_t command;
        uint8_t address;
        uint8_t size;

        uint8_t * p_response;
        uint32_t response_size;

        command = rx_buff[0];
        address = rx_buff[1];
        size = rx_buff[2];

        p_response = get_response_data(command, address, size, &response_size);

        MSS_SPI_set_cmd_response(&g_mss_spi1, p_response, response_size);
    }

    void spi1_block_rx_handler
    (
        uint8_t * rx_buff,
        uint32_t rx_size
    )
    {
        process_rx_data(rx_buff, rx_size);
    }
  @endcode
 */
void MSS_SPI_set_cmd_handler
(
    mss_spi_instance_t * this_spi,
    mss_spi_block_rx_handler_t cmd_handler,
    uint32_t cmd_size
);

/***************************************************************************//**
 The MSS_SPI_set_cmd_response() function specifies the data that will be
 returned to the master, when a command has been received by the slave. This
 function is called as part of the MSS_SPI_set_cmd_handler().
 See the description of MSS_SPI_set_cmd_handler() for more details.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block used. There are two such data
    structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and MSS SPI
    1 respectively. This parameter must point to either the g_mss_spi0 or
    g_mss_spi1 global data structure defined within the SPI driver.

  @param resp_tx_buffer
    The resp_tx_buffer parameter is a pointer to the buffer containing the data
    that must be returned to the host in the data phase of a SPI command oriented
    transaction.

  @param resp_buff_size
    The resp_buff_size parameter specifies the size of the buffer pointed to by
    the resp_tx_buffer parameter.
 */
void MSS_SPI_set_cmd_response
(
    mss_spi_instance_t * this_spi,
    const uint8_t * resp_tx_buffer,
    uint32_t resp_buff_size
);


#ifdef __cplusplus
}
#endif

#endif /* MSS_SPI_H_*/
