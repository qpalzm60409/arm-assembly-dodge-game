/* -*-C-*-
 *
 * $Revision: 1.15.2.2 $
 *   $Author: rivimey $
 *     $Date: 1998/10/08 17:39:41 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Public client interface to devices
 */

#include "devclnt.h"
#include "devdriv.h"

#include "logging.h"

/* private globals */

/* all the read callback info for a device */
typedef struct ReadCallbacks
{
    DevRead_CB_Fn callback;
    void *cb_data;
    DevGetBuff_Fn get_buff;
    void *getb_data;
} ReadCallbacks;

/* all the callback info for a device */
typedef struct DevCallbacks
{
    DevWrite_CB_Fn write_callback;      /* current write */
    void *write_cb_data;                /* current write */
    
    ReadCallbacks read[DC_NUM_CHANNELS];
} DevCallbacks;

/* stored callbacks and callback data for each device */
static DevCallbacks dev_cb[DI_NUM_DEVICES];


/* useful for debugging packets */
#if DEBUG == 1
static void debug_packet(log_id mod, p_Buffer buffer, unsigned int length)
{
#ifndef NO_LOG_INFO
    log_dump_buffer(WL_INFO, mod, 16, (char*)buffer, length);
#endif
}
#else
#define debug_packet(mod, buff, length)
#endif

DevError 
angel_DeviceWrite(DeviceID devID, p_Buffer buff,
                  unsigned length, DevWrite_CB_Fn callback,
                  void *cb_data, DevChanID chanID)
{
#ifdef CHECK_PARAMS
    if (devID >= DI_NUM_DEVICES)
    {
        return DE_NO_DEV;
    }
    
    if (chanID >= DC_NUM_CHANNELS)
    {
        return DE_BAD_CHAN;
    }

    if (angel_Device[devID]->type != DT_ANGEL)
    {
        return DE_BAD_DEV;
    }
#endif
    
    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_WRITE_BUSY)
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_WRITE_BUSY;  /* cleared by device */
    Angel_ExitToUSR();
    
    LogInfo(LOG_DEVCLNT, ("angel_DeviceWrite: devid %d, cb %x, devchan %d\n",
                          devID, callback, chanID));
    
    debug_packet(LOG_DEVCLNT, buff, length);

    /* Okay, we've passed the checks, let's get on with it */
    dev_cb[devID].write_callback = callback;
    dev_cb[devID].write_cb_data = cb_data;

    LogInfo(LOG_DEVCLNT, ("angel_DeviceWrite: chaining to device async write.\n"));
    
    return angel_Device[devID]->rw.angel.async_write(devID,
                                                     buff, length, chanID);
}


DevError 
angel_DeviceRegisterRead(DeviceID devID,
                         DevRead_CB_Fn callback, void *cb_data,
                         DevGetBuff_Fn get_buff, void *getb_data,
                         DevChanID devchanID)
{
    LogInfo(LOG_DEVCLNT, ("angel_DeviceRegisterRead: devID %d, cb %x, devchan %d\n",
                            devID, callback, devchanID));

#ifdef CHECK_PARAMS
    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;

    if (devchanID >= DC_NUM_CHANNELS)
        return DE_BAD_CHAN;

    if (angel_Device[devID]->type != DT_ANGEL)
        return DE_BAD_DEV;
#endif
    
    Angel_EnterSVC();
    if (angel_DeviceStatus[devID] & DEV_READ_BUSY(devchanID))
    {
        Angel_ExitToUSR();
        return DE_BUSY;
    }
    angel_DeviceStatus[devID] |= DEV_READ_BUSY(devchanID);
    Angel_ExitToUSR();

    /* Okay, we've passed the checks, let's get on with it */
    dev_cb[devID].read[devchanID].callback = callback;
    dev_cb[devID].read[devchanID].cb_data = cb_data;
    dev_cb[devID].read[devchanID].get_buff = get_buff;
    dev_cb[devID].read[devchanID].getb_data = getb_data;

    LogInfo(LOG_DEVCLNT, ("angel_DeviceRegisterRead: calling device register.\n"));
    
    return angel_Device[devID]->rw.angel.register_read(devID, devchanID);
}


/*
 *  Function: angel_DeviceControl
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
DevError 
angel_DeviceControl(DeviceID devID, DeviceControl op, void *arg)
{
    LogInfo(LOG_DEVCLNT, ("angel_DeviceControl: devID %d, op %d, arg %x\n",
                            devID, op, arg));
#ifdef CHECK_PARAMS

    if (devID >= DI_NUM_DEVICES)
        return DE_NO_DEV;
#endif
    
    return angel_Device[devID]->control(devID, op, arg);
}


/*
 *  Function: angel_ControlAll
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
static void 
angel_ControlAll(DeviceControl op, void *arg)
{
    int i;

    LogInfo(LOG_DEVCLNT, ( "angel_ControlAll: op %d, arg %x\n", op, arg));

    for (i = 0; i < DI_NUM_DEVICES; ++i)
        angel_Device[i]->control(i, op, arg);
}


/*
 *  Function: angel_ReceiveMode
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_ReceiveMode(DevRecvMode mode)
{
    angel_ControlAll(DC_RECEIVE_MODE, (void *)mode);
}


/*
 *  Function: angel_ResetDevices
 *   Purpose: 
 *
 *    Params:
 *       Input: 
 *
 *   Returns: .
 */
void 
angel_ResetDevices(void)
{
    angel_ControlAll(DC_RESET, NULL);
}


/*
 *  Function: angel_InitialiseDevices
 *   Purpose: To call the device initialise code of all devices
 *            configured under Angel.
 *
 *    Params:
 *       Input: none
 *
 *   Returns: none
 */
void 
angel_InitialiseDevices(void)
{
    int i;

    LogInfo(LOG_DEVCLNT, ( "angel_InitialiseDevices\n"));

    Angel_InitChannels();
    Angel_InitBuffers();

    for (i = 0; i < DI_NUM_DEVICES; ++i)
    {
        angel_DeviceStatus[i] = 0;
        angel_Device[i]->control(i, DC_INIT, NULL);
    }

    /* err, thats all? */
    LogInfo(LOG_DEVCLNT, ( "angel_InitialiseDevices complete.\n"));
}


/*
 *  Function: angel_IsAngelDevice
 *
 *   Purpose: To return TRUE if the device is an Angel Packet device,
 *            FALSE if it is a RAW device.
 *
 *    Params:
 *       Input: devid - the device ID code associated with the device.
 *
 *   Returns: bool - TRUE or FALSE.
 */
bool 
angel_IsAngelDevice(DeviceID devID)
{
    ASSERT(devID < DI_NUM_DEVICES, ("illegal device"));
    return (angel_Device[devID]->type == DT_ANGEL);
}


/*
 *  Function: angel_DD_GotPacket
 *
 *   Purpose: Handle receipt of a packet callback. The routine 
 *            is called from the High priority task called from
 *            the device ISR, which is expected to convert the
 *            byte- (or other-) formatted data into packet form.
 *
 *            Various checks are performed to ensure the call
 *            is correct before indexing the callback array to
 *            find the function which will process the packet.
 *
 *            The callback task will be queued for later action;
 *            this routine *does* return to the caller.
 *
 *    Params:
 *       Input: devID - the ID code of device which read the data
 *              buff -  the buffer in which the data has been placed
 *              length - the length of the data in the buffer
 *              status - DS_DONE if data considered good, else
 *                       data is considered bad (e.g. CRC fail, bad
 *                       length).
 *              devchanID - the device channel. typically DC_DBUG
 *                      for debug (RDI,etc) data, DC_APPL for
 *                      other (application) data.
 *
 *   Returns: nothing.
 */
void 
angel_DD_GotPacket(DeviceID devID, p_Buffer buff, unsigned length,
                   DevStatus status, DevChanID devchanID)
{
    angel_TaskType tasktype;

    LogInfo(LOG_DEVCLNT, ("angel_DD_GotPacket: dev %d, buff %x, len %d, stat %s, "
                            "chan %d, [reason %s]\n",
                            devID, buff, length, status == DS_DONE ? "ok" : "*BAD*",
                            devchanID, log_adpname(*(long*)(buff+4))));

    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));
    ASSERT(dev_cb[devID].read[devchanID].callback != NULL,
           ("no read callback"));

    if (buff != NULL && length > 0)
        debug_packet(LOG_DEVCLNT, buff, length);

    tasktype = (devchanID == DC_DBUG ? TP_AngelCallBack : TP_ApplCallBack);

    /* 
     * do the call: "callback(buff, length, status, cb_data);"
     */
    Angel_QueueCallback(dev_cb[devID].read[devchanID].callback, tasktype,
                        (void *)buff, (void *)length, (void *)status,
                        (void *)dev_cb[devID].read[devchanID].cb_data);
}


/*
 *  Function: angel_DD_SentPacket
 *
 *   Purpose: Handle transmission completion. Called as a result of
 *            the device having pulled all the data from the device
 *            ring buffer (more generally: called when a packet send,
 *            initiated with AsyncWrite, has completed).
 *
 *            Various checks are performed to ensure the call
 *            is correct before indexing the callback array to
 *            find the function which will process the packet.
 *
 *            The call is used to initiate further action by the code
 *            sending the data. It must also ensure the device BUSY
 *            flag has been reset, indicating that the device can be
 *            used to send something else.
 *
 *            The callback task will be queued for later action;
 *            this routine *does* return to the caller.
 *
 *    Params:
 *       Input: devID - which device
 *              buff -  pointer to data
 *              length - how much done
 *              status -  success code
 *              devchanID - appl or chan
 *
 *   Returns: nothing
 */
void 
angel_DD_SentPacket(DeviceID devID, p_Buffer buff, unsigned length,
                    DevStatus status, DevChanID devchanID)
{
    unsigned int s;
    angel_TaskType tasktype;

    LogInfo(LOG_DEVCLNT, ("angel_DD_SentPacket: dev %d, buff %x, len %d, stat %d, "
                          "chan %d, [reason %s]\n",
                          devID, buff, length, status,
                          devchanID, log_adpname(*(long*)(buff+4))));
    
    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));
    ASSERT(dev_cb[devID].write_callback != NULL, ("no write callback"));

    /*
     * We need to protect this
     */
    s = Angel_DisableInterruptsFromSVC();
    angel_DeviceStatus[devID] &= ~DEV_WRITE_BUSY;
    Angel_RestoreInterruptsFromSVC(s);

    tasktype = (devchanID == DC_DBUG ? TP_AngelCallBack : TP_ApplCallBack);
    
    /* 
     * do the call: "callback(buff, length, status, cb_data);"
     */
    Angel_QueueCallback(dev_cb[devID].write_callback, tasktype,
                        (void *)buff, (void *)length, (void *)status,
                        (void *)dev_cb[devID].write_cb_data);
}


/*
 *  Function: angel_DD_GetBuffer
 *
 *   Purpose: To get a buffer from the Angel buffer pool. This will
 *            usually imply a call to the Angel Buffer management
 *            functions, but the exact implementation is hidden behind
 *            the specified "get_buff" function pointer, which allows
 *            a caller to specify it's own buffer management.
 *
 *
 *    Params:
 *       Input: devID - the ID of the device requesting the buffer
 *              devchanID - the device channel; typically DC_DBUG
 *              req_size - the minimum number of bytes buffer space
 *                         needed
 *
 *   Returns: a pointer to the buffer, or NULL if not avaiable.
 *            (no distinction is made between no buffer available,
 *            and no buffer of a sufficient size).
 */
p_Buffer 
angel_DD_GetBuffer(DeviceID devID, DevChanID devchanID,
                   unsigned req_size)
{
    p_Buffer ret_buffer;

    LogInfo(LOG_DEVCLNT, ("angel_DD_GetBuffer: dev %d, chanID %d, size %d\n",
                            devID, devchanID, req_size));

    ASSERT(devID < DI_NUM_DEVICES, ("devID bad"));
    ASSERT(devchanID < DC_NUM_CHANNELS, ("devchanID bad"));

    {
        struct ReadCallbacks *rcb = &dev_cb[devID].read[devchanID];

        if (rcb->get_buff == NULL)
        {
            LogError(LOG_DEVCLNT, ( "angel_DD_GetBuffer: No alloc fn! (rcb = %p)\n", rcb));
            return NULL;
        }

        ret_buffer = rcb->get_buff(req_size, rcb->getb_data);
    }

    if (ret_buffer == NULL)
        LogError(LOG_DEVCLNT, ( "angel_DD_GetBuffer: get_buff returned NULL\n"));
    else
        LogInfo(LOG_DEVCLNT, ( "angel_DD_GetBuffer: returning buffer 0x%x\n", ret_buffer));

    return ret_buffer;
}


/* EOF devclnt.c */
