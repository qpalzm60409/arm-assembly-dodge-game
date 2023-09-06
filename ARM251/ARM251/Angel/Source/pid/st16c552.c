/* -*-C-*-
 *
 * $Revision: 1.26.2.3 $
 *   $Author: rivimey $
 *     $Date: 1998/10/15 18:00:26 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Serial and Parallel device driver for PID board, originally
 *              derived from PIE board serial driver.
 */

#include "devdriv.h"            /* device driver support */
#include "logging.h"
#include "serlock.h"            /* serialisation, etc. */
#include "params.h"             /* parameter structures and utilities */

#include "st16c552.h"
#include "pid.h"
#include "serring.h"
#include "serial.h"
#include "support.h"


/* General purpose constants, macros, enums, typedefs */

#define DEFBAUD               9600      /* default baud rate */


#define ST16C552_FILL_TX_FIFO 1         /* when writing, fill Tx FIFO; alt. write 1 byte */

/*
 * forward declarations of necessary functions
 */
static void st16c552_ControlTx(const DeviceID devid);
static void st16c552_ControlRx(const unsigned int devid);
static DevError st16c552_Control(DeviceID devid, 
                                 DeviceControl op,
                                 void *arg);
static void st16c552_KickStartFn(DeviceID devid);



/*
 * The set of parameter options supported by the device
 * Note: 56000 is 57600 with a 2.77% error -- see ST16552 manual -- ric
 */
static unsigned int baud_options[] =
{
    1200,
    2400,
    4800,
    9600,
    19200,
    38400,
    56000,
    57600,
    115200
};

static ParameterList param_list[] = {
    {
        AP_BAUD_RATE,
        sizeof(baud_options) / sizeof(baud_options[0]),
        baud_options
    }
};

/* 
 * The default parameter config for the device
 */
static Parameter param_default[] = {
    { AP_BAUD_RATE, 9600 }
};

/* the state shared between the Int. handler and the deferred processor */
static RingBuffer st16c552_rx_ring[ST16C552_NUM_PORTS];
static RingBuffer st16c552_tx_ring[ST16C552_NUM_PORTS];

#if (HAVE_ANGEL_ST16C552)

/*
 * the configuration needed by the TX and RX engines
 */
#define SERIAL_FC_SET  ((1 << serial_XON) | (1 << serial_XOFF))
#define SERIAL_CTL_SET ((1 << serial_STX) | (1 << serial_ETX) |\
                        (1 << serial_ESC))
#define SERIAL_ESC_SET (SERIAL_FC_SET | SERIAL_CTL_SET)


static const struct re_config engine_config[ST16C552_NUM_PORTS] =
{
  { 
    serial_STX, serial_ETX, serial_ESC, /* self-explanatory?               */
    SERIAL_FC_SET,                      /* set of flow-control characters  */
    SERIAL_ESC_SET,                     /* set of characters to be escaped */
    serpkt_flow_control, (void *)DI_ST16C552_A,/* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, (void *)DI_ST16C552_A /* how to get a buffer */
  }
#if (ST16C552_NUM_PORTS > 1)
 ,{ 
    serial_STX, serial_ETX, serial_ESC, /* self-explanatory?               */
    SERIAL_FC_SET,                      /* set of flow-control characters  */
    SERIAL_ESC_SET,                     /* set of characters to be escaped */
    serpkt_flow_control, (void *)DI_ST16C552_B,/* what to do with FC chars */
    angel_DD_RxEng_BufferAlloc, (void *)DI_ST16C552_B /* how to get a buffer */
  }
#endif
};

/* the state of the rx engine */
static struct re_state rx_engine_state[ST16C552_NUM_PORTS];

/* the state of the tx engine */
static struct te_state tx_engine_state[ST16C552_NUM_PORTS];

/* packet for actual rx in progress */
static struct data_packet rx_packet[ST16C552_NUM_PORTS];

/* the current write packet */
static struct data_packet tx_packet[ST16C552_NUM_PORTS];

/* the above, collected */
static RxTxState st16c552_rx_tx_state[ST16C552_NUM_PORTS] =
{
    {
        &engine_config[ST16C552_IDENT_A],
        &rx_engine_state[ST16C552_IDENT_A],
        &tx_engine_state[ST16C552_IDENT_A],
        &rx_packet[ST16C552_IDENT_A],
        &tx_packet[ST16C552_IDENT_A],
        1 /* rx_flow */
    }
#if (ST16C552_NUM_PORTS > 1)
    ,{ 
        &engine_config[ST16C552_IDENT_B],
        &rx_engine_state[ST16C552_IDENT_B],
        &tx_engine_state[ST16C552_IDENT_B],
        &rx_packet[ST16C552_IDENT_B],
        &tx_packet[ST16C552_IDENT_B],
        1 /* rx_flow */
    }
#endif
};
#endif /* HAVE_ANGEL_ST16C552 */


#if (HAVE_RAW_ST16C552)
static RawState st16c552_raw_state[ST16C552_NUM_PORTS];
#endif

/*
 * The control functions and interface
 */
static const SerialControl st16c552_Ctrl[ST16C552_NUM_PORTS] =
{
    {
#if (RAW_ST16C552_A == 0)
        &st16c552_rx_tx_state[ST16C552_IDENT_A],
        NULL,
        serpkt_int_tx_processing,
        serpkt_int_rx_processing,
#else
        NULL,
        &st16c552_raw_state[ST16C552_IDENT_A],
        serraw_int_tx_processing,
        serraw_int_rx_processing,
#endif
        ST16C552_IDENT_A,
        &st16c552_tx_ring[ST16C552_IDENT_A],
        &st16c552_rx_ring[ST16C552_IDENT_A],
        st16c552_ControlTx,
        st16c552_ControlRx,
        st16c552_Control,
        st16c552_KickStartFn
    }
#if ST16C552_NUM_PORTS > 1
   ,{
#if (RAW_ST16C552_B == 0)
        &st16c552_rx_tx_state[ST16C552_IDENT_B],
        NULL,
        &packet_state[ST16C552_IDENT_B],
        serpkt_int_tx_processing,
        serpkt_int_rx_processing,
#else
        NULL,
        &st16c552_raw_state[ST16C552_IDENT_B],
        NULL,
        serraw_int_tx_processing,
        serraw_int_rx_processing,
#endif
        ST16C552_IDENT_B,
        &st16c552_tx_ring[ST16C552_IDENT_B],
        &st16c552_rx_ring[ST16C552_IDENT_B],
        st16c552_ControlTx,
        st16c552_ControlRx,
        st16c552_Control,
        st16c552_KickStartFn
    }
#endif
};

/* Publically-accessible globals */

/*
 * The device table entry for this device
 */
const struct angel_DeviceEntry angel_ST16C552Serial[ST16C552_NUM_PORTS] =
{
    {
#if (RAW_ST16C552_A == 0)
        DT_ANGEL,
        {
            serpkt_AsyncWrite,
            serpkt_RegisterRead
        },
        serpkt_Control,
#else
        DT_RAW,
        {
            /* nasty but necessary casts, as can only statically initialise
             * the first member of a union
             */
            (angel_DeviceWriteFn)       serraw_Write,
            (angel_DeviceRegisterReadFn)serraw_Read
        },
        serraw_Control,
#endif
        &st16c552_Ctrl[ST16C552_IDENT_A],
        {
            sizeof(param_list) / sizeof(param_list[0]), param_list
        },
        {
            sizeof(param_default) / sizeof(param_default[0]), param_default
        }
    }
#if ST16C552_NUM_PORTS > 1
   ,{
#if (RAW_ST16C552_B == 0)
        DT_ANGEL,
        {
            serpkt_AsyncWrite,
            serpkt_RegisterRead
        },
        serpkt_Control,
#else
        DT_RAW,
        {
            /* nasty but necessary casts, as can only statically initialise
             * the first member of a union
             */
            (angel_DeviceWriteFn)       serraw_Write,
            (angel_DeviceRegisterReadFn)serraw_Read
        },
        serraw_Control,
#endif
        &st16c552_Ctrl[ST16C552_IDENT_B],
        {
            sizeof(param_list) / sizeof(param_list[0]), param_list
        },
        {
            sizeof(param_default) / sizeof(param_default[0]), param_default
        }
    }
#endif
};


/* Private globals */
static int st16c552_ppcontrol_shadow = 0;

/*
 * the Interrupt Enable register is write-only, so we use the
 * scratch register to keep track of currently enabled Ints
 */
#define IER_set(u, f)   ((u)->ier = ((u)->spr |= (f)))
#define IER_clr(u, f)   ((u)->ier = ((u)->spr &= (~(f))))
#define IER_reset(u)    ((u)->ier = ((u)->spr = 0));

/*
 * macros to control Interrupt Enable Register in various sensible ways
 */
#define st16c552_EnableTxInterrupt(u)  (IER_set((u), TxReadyInt))
#define st16c552_DisableTxInterrupt(u) (IER_clr((u), TxReadyInt))
#define st16c552_EnableRxInterrupt(u)  (IER_set((u), RxReadyInt))
#define st16c552_DisableRxInterrupt(u) (IER_clr((u), RxReadyInt))

/*
 * macros to get and put serial characters
 */
#define st16c552_SerGetChar(u)     ((u)->rhr)
#define st16c552_SerPutChar(u, c)  ((u)->thr = (c))
#define st16c552_GetLineStatus(u) ((u)->lsr)

/*
 * macros to get and put parallel port characters
 */
#define st16c552_ParGetChar(u)     ((u)->port)
#define st16c552_ParPutChar(u, c)  ((u)->port = (c))
#define st16c552_ParStatus(u)      ((u)->ppstatus)

/*
 * macros to control Parallel Port Control Register in various ways
 */
#define CON_set(u, f)   ((u)->ppcontrol = (st16c552_ppcontrol_shadow |= (f)))
#define CON_clr(u, f)   ((u)->ppcontrol = (st16c552_ppcontrol_shadow &= (~(f))))
#define CON_reset(u)    ((u)->ppcontrol = (st16c552_ppcontrol_shadow = 0));

#define st16c552_EnableParInterrupt(u)  (CON_set(u, CONInterrupt))
#define st16c552_DisableParInterrupt(u) (CON_clr(u, CONInterrupt))


/*
 *  Function: st16c552_KickStartFn
 *
 *   Purpose: Kick-start tx by sending first character.
 *            When the character has been sent, the chip
 *            will interrupt and thus the ISR will send the
 *            rest of the data.
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: Nothing
 */
static void st16c552_KickStartFn(const DeviceID devid)
{
    char c;
    const unsigned int port = SerCtrl(devid)->port;
    RingBuffer *ring = st16c552_tx_ring + port;
    ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    c = ringBufGetChar( ring );
    st16c552_SerPutChar(serchip, c);
}

/*
 *  Function: st16c552_ControlTx
 *   Purpose: enable async transmission if not already enabled
 *
 *  Pre-conditions: Interrupts should be disabled
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: Nothing
 */
static void st16c552_ControlTx(const DeviceID devid)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    ST16C552Reg *const serchip =
        (SerCtrl(devid)->port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    /*
     * We enable tx whenever we want to send a flow control char,
     * OR when we are sending a packet and we have not been XOFF'ed
     */
    if ((*status & SER_TX_FLOW_CONTROL) != 0 ||
        (*status & SER_TX_MASK) == SER_TX_DATA )
    {
        /*
         * tx interrupt should be enabled, if not already
         */
        if ((*status & SER_TX_IRQ_EN) == 0)
        {
            st16c552_EnableTxInterrupt(serchip);
            *status |= SER_TX_IRQ_EN;
        }
    }
    else
    {
        /*
         * tx interrupt should be disabled, if not already
         */
        if ((*status & SER_TX_IRQ_EN) != 0)
        {
            st16c552_DisableTxInterrupt(serchip);
            *status &= ~SER_TX_IRQ_EN;
        }
    }
}

/*
 *  Function: st16c552_ControlRx
 *   Purpose: Control Rx interrupt mask according to flow control
 *              and read interest.
 *
 *  Pre-conditions: Interrupts should be disabled
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: Nothing
 */
static void st16c552_ControlRx(const unsigned int devid)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;

    if ((*status & SER_RX_DISABLED) == 0 &&
        (*status & DEV_READ_BUSY_MASK) != 0)
    {
        /*
         * rx interrupt should be enabled, if not already
         */
        if ((*status & SER_RX_IRQ_EN) == 0)
        {
#if PARALLEL_SUPPORTED
            if (devid == DI_PARALLEL)
            {
                ST16C552PP *const parchip = (ST16C552PP *)NISA_PAR;

                /*
                 * Enable interrupts, while keeping chip
                 * set for input mode.
                 */
                st16c552_EnableParInterrupt(parchip);
            }
#endif
            /* always do this, as serial-only is always supported */
            {
                ST16C552Reg *const serchip = (SerCtrl(devid)->port == ST16C552_IDENT_A) ?
                    (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
                
                st16c552_EnableRxInterrupt(serchip);
            }
            *status |= SER_RX_IRQ_EN;
            LogInfo(LOG_ST16C552, ("RX Interrupts now enabled (status %02x).\n", *status));
        }
    }
    else
    {
        /*
         * rx interrupt should be disabled, if not already
         */
        if ((*status & SER_RX_IRQ_EN) != 0)
        {
#if PARALLEL_SUPPORTED
            if (devid == DI_PARALLEL)
            {
                ST16C552PP *const parchip = (ST16C552PP *)NISA_PAR;

                /*
                 * Disable interrupts, while keeping chip
                 * set for input mode.
                 */
                st16c552_DisableParInterrupt(parchip);
            }
#endif
            /* always do this, as serial-only is always supported */
            {
                ST16C552Reg *const serchip = (SerCtrl(devid)->port == ST16C552_IDENT_A) ?
                    (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;
                
                st16c552_DisableRxInterrupt(serchip);
            }
            *status &= ~SER_RX_IRQ_EN;
            LogInfo(LOG_ST16C552, ("RX Interrupts now disabled (status %02x).\n", *status));
        }
    }
}

/*
 *  Function: st16c552_set_speed
 *   Purpose: Line Speed control
 *
 *    Params:
 *       Input: port    serial port identifier
 *
 *              speed   desired line speed
 *
 *   Returns:
 *          OK: DE_OKAY
 *       Error: DE_BAD_MODE
 */
static DevError st16c552_set_params(const unsigned int port,
                                    const ParameterConfig *const config)
{
    unsigned int speed, baud_value;
    ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    if (!Angel_FindParam(AP_BAUD_RATE, config, &speed))
        LogError(LOG_ST16C552, ("Baud rate not specified in config\n"));

    switch (speed)
    {
      case 1200:
          baud_value = Baud1200;
          break;

      case 2400:
          baud_value = Baud2400;
          break;

      case 4800:
          baud_value = Baud4800;
          break;

      case 9600:
          baud_value = Baud9600;
          break;

      case 19200:
          baud_value = Baud19200;
          break;

      case 38400:
          baud_value = Baud38400;
          break;

      case 56000:
          baud_value = Baud56000;
          break;

      case 57600:
          baud_value = Baud57600;
          break;

      case 115200:
          baud_value = Baud115200;
          break;

      case 230400:
      default:
          return DE_BAD_OP;
    }

    LogInfo(LOG_ST16C552, ("Set baud rate to %d; programming with divisor: 0x%x\n",
                             speed, baud_value));

    /*
     * wait for current transmits to finish
     */
    while ((serchip->lsr & LSRTxEmpty) == 0)
        /* do nothing */
        ;

    /*
     * Enable divisor latch via the Line Control Register, and
     * set the baud rates
     */
    serchip->lcr = LCRSelectDivisor;
    serchip->dll = (unsigned char)(baud_value & 0xff);
    serchip->dlm = (unsigned char)((baud_value >> 8) & 0xff);

    /*
     * OK, set the operational values for the LCR: 8-bit words,
     * 1 stop bit, no parity
     */
    serchip->lcr = LCRWord8 | LCRStopBit1;

    /*
     * take the chip out of loopback, and enable interrupts; DTR* and
     * RTS* outputs are forced low.
     */
    serchip->mcr = MCRForceDTR | MCRForceRTS | MCREnableINT;
   
    /*
     * This is a weird place to do this, but problems with the PID hardware
     * mean this is where it must be.
     *
     * enable ST16C552 interrupts in the PID interrupt controller. If
     * user has requested using FIQ, in serpar mode we have to use IRQ
     * too, as only one source can be used.
     *
     * Note: we never really service on both FIQ and IRQ: If allowed, we
     * prefer FIQ unless using two serial ports and/or serial/parallel
     * mode, in which case we use IRQ only. We don't set the interrupts
     * to 0 (*EnableSet = 0) because the appl might want to do things too.
     */

#if SERIAL_INTERRUPTS_ON_FIQ
    *FIQSourceIRQ = ST16C552_FIQSELECT;
    *FIQEnableSet = 1;
#else
    *IRQEnableSet = ST16C552_IRQMASK;
#endif
    
    return DE_OKAY;
}

/*
 *  Function: st16c552_ResetDriver
 *   Purpose: Give the UART a hardware reset
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *              port    Serial port identifier
 *
 *   Returns: Nothing
 */
static void st16c552_ResetDriver(const unsigned int devid,
                                 const unsigned int port)
{
    volatile unsigned int *const status = angel_DeviceStatus + port;
    ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    /*
     * reset the tx and rx FIFO logic, set mode 1 ('16550) operation
     * and enable FIFO's
     */
    serchip->fcr = FCRFIFOEnable | FCRClearRX | FCRClearTX | FCRMode1;
    
    /*
     * Interrupt Enable Register - all interrupts off
     */
    IER_reset(serchip);

    /*
     * reset all private flags
     */
    *status = (*status << 24) >> 24;

    /*
     * FIFO Control Register - FIFO enabled, but Rx and Tx
     * FIFOs empty; RxRDY and TxRDY mode 1; FIFO receive
     * trigger level = 4.
     */
    serchip->fcr = FCRFIFOEnable | FCRMode1 | FCRRxTrig08;

    /*
     * Line Control Register is initialised in st16c552_set_params.
     *
     * Modem Control Register - DTR* and RTS* outputs high; INT
     * pin tri-stated; Rx and Tx "disabled" by setting chip in
     * loopback mode.
     */
    serchip->mcr = MCREnableLoop;

    /*
     * Once the chip is started, we always want Line status interrupts
     * to come through, RxReady and TxReady interrupts are turned on
     * and off as needed elsewhere.
       IER_set(serchip, RxLineInt);
     */

    (void)st16c552_set_params(port, &(angel_Device[devid]->default_config));

#if PARALLEL_SUPPORTED
    if (devid == DI_PARALLEL)
    {
        ST16C552PP *parchip = (ST16C552PP *)NISA_PAR;

        /*
         * select input mode
         */
        CON_reset(parchip);
        parchip->ppselect = IOSELinput;
        CON_set(parchip, CONDirInput);

        /*
         * now turn on AUTOFDXT and SLCTIN to let both the host
         * and the active cable know that we are ready
         *
         * Momentarily assert INIT, which is used to reset the Parallel
         * Port adapter hardware.
         */
        CON_set(parchip, CONAutoFDxt | CONSelectIn | CONInit);
        delay(1);
        CON_clr(parchip, CONInit);
    }
#endif /* PARALLEL_SUPPORTED */
    
}

/*
 *  Function: st16c552_reset
 *   Purpose: Reset a serial port
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *              port    Serial port identifier
 *
 *   Returns: DE_OKAY
 */
static DevError st16c552_reset(const DeviceID devid,
                               const unsigned int port)
{
    (void)st16c552_ResetDriver(devid, port);    /* low_level reset */
    st16c552_ControlRx(devid);

    return DE_OKAY;
}

/*
 *  Function: st16c552_init
 *   Purpose: Initialise a serial port
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *   Returns: status from device reset
 */
static DevError st16c552_init(const DeviceID devid)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    DevError err;

    /* do one-time start-up initialisation */

    /* reset private flags (LEAVE lowest 8 bits alone) */
    *status &= 0xFF;

    Angel_ExitToUSR();          /* because DeviceControl has done EnterSVC */

#if defined(MINIMAL_ANGEL) && MINIMAL_ANGEL != 0
    err = Angel_RawDeviceControl(devid, DC_RESET, NULL);
#else
    err = angel_DeviceControl(devid, DC_RESET, NULL);
#endif

    Angel_EnterSVC();

    return err;
}

/*
 *  Function: st16c552_recv_mode
 *   Purpose: Receive Mode control
 *
 *    Params:
 *       Input: devid   device ID of the driver
 *
 *              mode    the desired mode
 *
 *   Returns:
 *          OK: DE_OKAY
 *       Error: DE_BAD_OP - Unknown mode
 */
static DevError st16c552_recv_mode(const DeviceID devid,
                                   const DevRecvMode mode)
{
    volatile unsigned int *const status = angel_DeviceStatus + devid;

    if (mode == DR_DISABLE)
    {
        if ((*status & SER_RX_DISABLED) == 0)
        {
            /*
             * disable reception and schedule send of XOFF
             */
            *status |= (SER_RX_DISABLED | SER_TX_FLOW_CONTROL);
            st16c552_ControlRx(devid);
            st16c552_ControlTx(devid);
        }
    }
    else if (mode == DR_ENABLE)
    {
        if ((*status & SER_RX_DISABLED) != 0)
        {
            /*
             * enable reception and schedule send of XON
             */
            *status &= ~SER_RX_DISABLED;
            *status |= SER_TX_FLOW_CONTROL;
            st16c552_ControlRx(devid);
            st16c552_ControlTx(devid);
        }
    }
    else
        /* bad mode */
        return DE_BAD_OP;
    
    return DE_OKAY;
}

/*
 *  Function: st16c552_Control
 *   Purpose: Entry point for device control functions
 *            See documentation for angel_DeviceControl in devdriv.h
 */
static DevError st16c552_Control(DeviceID devid, 
                                 DeviceControl op,
                                 void *arg)
{
    const unsigned int port = SerCtrl(devid)->port;
    DevError ret_code;

    LogInfo(LOG_ST16C552, ("st16c552_Control: op %d arg %x\n", op, arg ));

    /* we only implement the standard ones here */
    switch (op)
    {
      case DC_INIT:          
          ret_code = st16c552_init(devid);
          break;

      case DC_RESET:         
          ret_code = st16c552_reset(devid, port);
          break;

      case DC_RECEIVE_MODE:  
          ret_code = st16c552_recv_mode(devid, (DevRecvMode)((int)arg));
          break;

      case DC_SET_PARAMS:     
          ret_code = st16c552_set_params(port,
                                         (const ParameterConfig *const)arg);
          break;

      case DC_SET_LED:
          /* Not implemented - but don't complain */
          ret_code = DE_OKAY;
          break;

      default:
          ret_code = DE_BAD_OP;
          break;
    }

    return ret_code;
}

/**********************************************************************/

#if PARALLEL_SUPPORTED

/*
 *  Function: st16c552_int_pp_processing
 *
 *   Purpose: Read a stream of characters from the parallel port into
 *            the parallel port ring buffer.
 *
 *            The constant PP_POLL_TIMEOUT exists to protect the driver
 *            from weird conditions on the port. 
 *
 *            Expects to run as a SVC mode Serialised task, run from
 *            the interrupt handler.
 *
 *    Params: (via args)
 *       Input: parchip The address of the controller.
 *
 *   Returns: Nothing
 */

/* 96 * 1024 is 98304, a nicer constant for compiler to load than 100000 */
#define PP_POLL_TIMEOUT_COUNT (96*1024)

static void st16c552_int_pp_processing(void *args)
{
    bool done = FALSE;
    bool timeout;
    unsigned int i, j;
    ST16C552PP *const parchip = (ST16C552PP *)args;
    RingBuffer *ring = st16c552_rx_ring + (SerCtrl(DI_PARALLEL)->port);
  
    /*
     * make sure interrupts are disabled while we poll data in
     */
    *IRQEnableClear = IRQ_PARALLEL;
  
    /*
     * When we first enter, we know there is a character waiting
     */
    do
    {
        timeout = TRUE;
    
        for ( i = PP_POLL_TIMEOUT_COUNT; i > 0 ; i-- )
        {
            if ((st16c552_ParStatus(parchip) & SRAck) == 0)
            {
                char rx_ch;
                unsigned psr_val;

                /*
                 * We need to protect this bit from interrupts, else we could
                 * miss ACK going TRUE then FALSE with the next character
                 */
                psr_val = Angel_DisableInterruptsFromSVC();
        
                /* get a character */
                rx_ch = st16c552_ParGetChar(parchip);
                
                /* toggle strobe. Spec states the minimum pulse width is 0.5us */
                CON_set(parchip, CONStrobe);
                delay(1);  /* delay at least 1us, probably longer */
                CON_clr(parchip, CONStrobe);
        
                /*
                 * Technically, we have finished, but the active cable has a
                 * slow risetime on ACK, so we should wait until ACK is TRUE
                 * again before returning (this guards against returning too
                 * soon and reading the same character again because ACK hasn't
                 * been properly cycled).
                 */
                j = PP_POLL_TIMEOUT_COUNT;
                while (((st16c552_ParStatus(parchip) & SRAck) == 0) && (j > 0))
                    j--;
                
                Angel_RestoreInterruptsFromSVC(psr_val);
        
                /*
                 * if the ring buffer has space, we can put the character
                 * just received into it and check for end-of-packet
                 * conditions. If not the driver shouldn't really have read
                 * the character...
                 */
                if (!ringBufFull( ring ))
                {
                    ringBufPutChar( ring, rx_ch );
            
                    /* Note tbis check only makes sense for a non-raw parallel port,
                     * ie. one which is used only for Angel packets.
                     */
                    if (rx_ch == serial_ETX || 
                        (ringBufCount(ring) >= RING_HIGH_WATER_MARK) )
                    {
                        /* We have finished the packet / end of ring buffer */
                        done = TRUE;
                    }
                }
                else
                {
                    LogError(LOG_ST16C552, ( "rx ring buffer overflow" ));
                }
        
                if (done)
                {
                    /*
                     * Since we're already serialised, we can call the
                     * rx_processing code inline.
                     */
                    SerCtrl(DI_PARALLEL)->rx_processing(DI_PARALLEL);
                }

                timeout = FALSE;
                break;          /* out of the for loop */
            }
        }
    
    } while ( ! timeout && ! done );
  
    /*
     * re-enable interrupts now it is safe to do so
     */
    *IRQEnableSet = IRQ_PARALLEL;
}
#endif


/**********************************************************************/

/*
 *  Function: int_*
 *   Purpose: Set of handlers for the various prioritised interrupts.
 *              These routines do all the urgent processing required
 *              for the interrupt condition, and then schedule deferred
 *              routines for the non-urgent processing.
 *
 *  Pre-conditions: Processor is in IRQ / FIQ mode, with a minimal stack,
 *                      AND NO STACK LIMIT SET UP.
 *
 *    Params:
 *       Input: devid           device ID of the driver
 *
 *              port            serial port identifier
 *
 *              serchip            address of the controller for the given port
 *
 *              empty_stack     top of the stack
 *
 *   Returns: Nothing
 */

static void int_msr(unsigned int devid,
                    unsigned int port,
                    ST16C552Reg *const serchip,
                    unsigned int empty_stack)
{
    unsigned int msr;

    IGNORE(devid);
    IGNORE(port);
    IGNORE(empty_stack);

    /*
     * we really couldn't care less about these signals (in
     * fact, we shouldn't really ever get this interrupt
     * because it is never enabled); read the status to clear
     * the interrupt and go away again
     */
    msr = serchip->msr;
}

static void int_txrdy(const unsigned int devid,
                      const unsigned int port,
                      ST16C552Reg *const serchip,
                      const unsigned int empty_stack)
{
    bool queue_tx_deferred = FALSE;
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RingBuffer *ring = st16c552_tx_ring + port;
    int numsent = 0;

    /*
     * this ifdef is for backwards compatibility; the code used
     * to have a while loop which would only write one character
     * per interrupt to the chip
     */
#ifdef ST16C552_FILL_TX_FIFO
    /*
     * only do the following loop if the FIFO is empty to begin with.
     */
    if ((serchip->lsr & LSRFIFOEmpty) != 0)
    {
        /*
         * LSRFIFOEmpty only indicates that the holding register or
         * FIFO is empty, not when the FIFO is full. So, as we know there
         * are 16 chars in the FIFO and start off empty we can just fill
         * it up.
         */
        for (numsent = 0; numsent < 15; )
        {
#else
    {
        /* if the holding register is empty, stuff in one new character
         */
        while ((serchip->lsr & LSRFIFOEmpty) != 0)
        {
#endif
            /*
             * there is room, do we have anything to give?
             */
            if ((*status & SER_TX_FLOW_CONTROL) != 0)
            {
                /*
                 * send a flow control character
                 */
                st16c552_SerPutChar(serchip,
                                 ((*status & SER_RX_DISABLED) != 0) ?
                                 serial_XOFF : serial_XON);
                numsent++;

                *status &= ~SER_TX_FLOW_CONTROL;
            }
            else if (ringBufNotEmpty(ring))
            {
                /* stuff next character into Tx FIFO */
                st16c552_SerPutChar(serchip, ringBufGetChar(ring));
                numsent++;
            }
        
            /*
             * if we have reached to low-water mark in the
             * ring buffer then it is time to ask for a refill,
             * as long as there is more of the packet to come
             */
            if (ringBufCount(ring) < RING_LOW_WATER_MARK &&
                (*status & SER_TX_EOD) == 0)
                queue_tx_deferred = TRUE;

            /*
             * is it time to give up yet?
             */
            if (ringBufEmpty(ring))
            {
                /*
                 * if the end of the packet has been sent,
                 * then queue deferred processing of this fact
                 */
                if ((*status & SER_TX_EOD) != 0)
                {
                    queue_tx_deferred = TRUE;

                    /*
                     * disable Tx if necessary
                     */
                    *status &= ~SER_TX_DATA;
                    st16c552_ControlTx(devid);
                }
                else
                    /*
                 * queue_tx_deferred will already be set, but
                 * we need to set a flag to tell fill_tx_ring
                 * that it needs to restart character Tx.
                 */
                    *status |= SER_TX_KICKSTART;

                /* we have nothing more to send */
                break;
            }
        }
    }
    
    /* LogInfo(LOG_ST16C552, ("txrdy: sent %d chars\n", numsent)); */

    if (queue_tx_deferred && (*status & SER_TX_QUEUED) == 0)
    {
        *status |= SER_TX_QUEUED;

#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        /*
         * NOTE - this routine will never return
         */
        Angel_SerialiseISR( SerCtrl(devid)->tx_processing, (void *)devid );
#else
        SerCtrl(devid)->tx_processing((void *)devid);
#endif
    }
}

static void int_rxrdy(const unsigned int devid,
                      const unsigned int port,
                      ST16C552Reg *const serchip,
                      const unsigned int empty_stack)
{
    bool queue_rx_processing = FALSE;
    volatile unsigned int *const status = angel_DeviceStatus + devid;
    RingBuffer *ring = st16c552_rx_ring + port;
    unsigned char lsr, ch;
    RawState *const raw_state = SerCtrl(devid)->raw_state;
    int numrx = 0;
    
    /*
     * Keep looping for as long as there are characters
     * in the Rx FIFO.
     */
    if (raw_state != NULL)
    {   
        while ( ((lsr = st16c552_GetLineStatus(serchip)) & LSRRxData) &&
                (ringBufCount(ring) < RING_HIGH_WATER_MARK))
        {
            ch = st16c552_SerGetChar(serchip);

            if (lsr & LSROverrun)
                ringBufSetOvr(ring);
            ringBufPutChar(ring, ch);
            numrx++;
        
            if (raw_state->rx_to_go > 0)
                --(raw_state->rx_to_go);

            if (raw_state->rx_to_go == 0 && raw_state->rx_data != NULL)
            {
                queue_rx_processing = TRUE;
            }
        }
        
        if (ringBufCount(ring) >= RING_HIGH_WATER_MARK)
        {
            queue_rx_processing = TRUE;
        }
    }
    else /* raw_state == NULL */
    {
        int count;
        
        while ( ((lsr = st16c552_GetLineStatus(serchip)) & LSRRxData) &&
                (ringBufCount(ring) < RING_SIZE-1))
        {
            ch = st16c552_SerGetChar(serchip);

            if (lsr & LSROverrun)
                ringBufSetOvr(ring);
            ringBufPutChar(ring, ch);
            numrx++;
            
            if (ch == serial_ETX)
            {
                queue_rx_processing = TRUE;
            }
        }
        
        count = ringBufCount(ring);
        if (count >= RING_HIGH_WATER_MARK)
        {
            queue_rx_processing = TRUE;
        }
        
        if ((count >= RING_SIZE-1) && (lsr & LSRRxData))
        {
            int c = 0;
            
            do
            {
                ch = st16c552_SerGetChar(serchip);
                c++;
            } while ( st16c552_GetLineStatus(serchip) & LSRRxData);
            
            LogWarning(LOG_ST16C552, ("Overflow reading from chip. (read %d) ring->count = %d\n",
                                      c, count));
            
            ringBufSetOvr(ring);
            ringBufPutChar(ring, ch);
            numrx++;
        }
    }
    
    /* LogInfo(LOG_ST16C552, ("rxrdy: got %d chars\n", numrx)); */
    
    /*
     * if we've detected the end of a packet, and there isn't an rx task
     * already processing packets, then queue another one...
     */
    if (queue_rx_processing && (*status & SER_RX_QUEUED) == 0)
    {
        *status |= SER_RX_QUEUED;

#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        /*
         * NOTE - this routine will never return
         */
        Angel_SerialiseISR( SerCtrl(devid)->rx_processing, (void *)devid );
#else
        SerCtrl(devid)->rx_processing((void *)devid);
#endif
    }
}

static void int_lsr(unsigned int devid,
                    const unsigned int port,
                    ST16C552Reg *const serchip,
                    unsigned int empty_stack)
{
    unsigned int lsr;
    IGNORE(devid);
    IGNORE(empty_stack);

    /*
     * this is a slightly more sophisticated version of IGNORE,
     * one which can cope with const variables.
     */
    if (0)
        lsr = port;
    
    /*
     * read the Line Status Register, and keep the interesting
     * bits (bits 1->4,7); test whether a break has been signalled
     */
    lsr = st16c552_GetLineStatus(serchip);
    
    if (lsr & LSROverrun)
        LogInfo(LOG_ST16C552, ( "int_lsr: Overrun\n" ));

    if (lsr & LSRParity)
        LogInfo(LOG_ST16C552, ( "int_lsr: Parity\n" ));
    
    if (lsr & LSRFraming)
        LogInfo(LOG_ST16C552, ( "int_lsr: Framing\n" ));
    
    if (lsr & LSRBreak)
        LogInfo(LOG_ST16C552, ( "int_lsr: Break\n" ));
}

/*
 *  Function: angel_ST16C552IntHandler
 *   Purpose: Entry point for interrupts from the ST16C552 UART
 *            See documentation for angel_IntHandlerFn in devdriv.h
 */
void angel_ST16C552IntHandler(unsigned int ident, unsigned int devid,
                              unsigned int empty_stack)
{
#if PARALLEL_SUPPORTED
    if (ident == IH_PARALLEL)
    {
        ST16C552PP *parchip = (ST16C552PP *)NISA_PAR;
        unsigned char bytebucket;

        /*
         * read the status register: this supposedly clears the
         * interrupt, which should not re-appear until the next
         * Hi->Lo transition of ACK* (i.e. when the host starts
         * another burst of data).
         */
        bytebucket = st16c552_ParStatus(parchip);

        /*
         * All parallel processing is serialised.  Note that this
         * routine will never return.
         */
#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0
        Angel_SerialiseISR(st16c552_int_pp_processing, (void *)parchip );
#else
        st16c552_int_pp_processing((void *)parchip);
#endif
    }
    else
#else
        IGNORE(ident);
#endif
    {
        /*
         * jump table for prioritised interrupts
         */
        static void (*const intjump[4])(const unsigned int devid,
                                        const unsigned int port,
                                        ST16C552Reg *const serchip,
                                        const unsigned int empty_stack) =
        {
            int_msr, int_txrdy, int_rxrdy, int_lsr
        };

        unsigned int intsrc;
        const unsigned int port = SerCtrl(devid)->port;
        ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
            (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

        IGNORE(ident);

        /*
         * we are only interested in the first three bits of the
         * ISR: bit 0 signals whether another Int. is pending,
         * bits 1-2 indicate the prioritised interrupt; bit 3 is
         * only used to differentiate between a FIFO triggered
         * RxRDY Int and a character timeout RxRDY Int, and we
         * are not interested in this difference
         */
        while (((intsrc = (serchip->isr & 0x07)) & 0x01) == 0)
        {
            intjump[intsrc >> 1](devid, port, serchip, empty_stack);
        }
    }
}

#if DEBUG == 1
/* Routines for logging debug information to a seperate serial port */

void logserial_Reset(int port, int baudvalue)
{
    ST16C552Reg *const serchip = (port == ST16C552_IDENT_A) ?
        (ST16C552Reg *)NISA_SER_A : (ST16C552Reg *)NISA_SER_B;

    /*
     * reset the tx and rx FIFO logic, set mode 1 ('16550) operation
     * and enable FIFO's
     */
    serchip->fcr = FCRFIFOEnable | FCRClearRX | FCRClearTX | FCRMode1;
    
    /*
     * Interrupt Enable Register - all interrupts off
     */
    IER_reset(serchip);

    /*
     * wait for current transmits to finish
     */
    while ((serchip->lsr & LSRTxEmpty) == 0)
        /* do nothing */
        ;

    /*
     * Modem Control Register - DTR* and RTS* outputs high; INT
     * pin tri-stated; Rx and Tx "disabled" by setting chip in
     * loopback mode.
     */
    serchip->mcr = MCREnableLoop;

    /*
     * FIFO Control Register - FIFO enabled, but Rx and Tx
     * FIFOs empty; RxRDY and TxRDY mode 1; FIFO interrupt
     * trigger level = 8.
     */
    serchip->fcr = FCRFIFOEnable | FCRClearRX | FCRClearTX | FCRMode1 | FCRRxTrig08;

    /*
     * Enable divisor latch via the Line Control Register, and
     * set the baud rates
     */
    serchip->lcr = LCRSelectDivisor;
    serchip->dll = (unsigned char)(baudvalue & 0xff);
    serchip->dlm = (unsigned char)((baudvalue >> 8) & 0xff);

    /*
     * OK, set the operational values for the LCR: 8-bit words,
     * 1 stop bit, no parity
     */
    serchip->lcr = LCRWord8 | LCRStopBit1;

    /*
     * take the chip out of loopback, and enable interrupts; DTR* and
     * RTS* outputs are forced low.
     */
    serchip->mcr = MCRForceDTR | MCRForceRTS | MCREnableINT;
}

#endif

/* EOF st16c552.c */
