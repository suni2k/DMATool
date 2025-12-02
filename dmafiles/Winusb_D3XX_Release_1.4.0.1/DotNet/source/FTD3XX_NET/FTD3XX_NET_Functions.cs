/*
** FTD3XX_NET_Functions.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for .NET wrapper of the Windows FTD3XX.dll API calls.
** Public functions
**
** Author: FTDI
** Project: D3XX Windows Driver Package
** Module: FTD3XX_NET Managed Wrapper
** Requires: 
** Comments:
**
** History:
**  1.0.0.0	- Initial version
**
*/

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using System.Text;



namespace FTD3XXWU_NET
{
    public partial class FTDI
    {
        //**************************************************************************
        // GetNumberOfDevices
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Returns the number of D3XX devices connected to the system.  
        /// The count includes both unopen and open D3XX devices.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ListDevices in FTD3XX.DLL</returns>
        /// <param name="pulNumDevices">The number of D3XX devices connected to the system.</param>
        public FT_STATUS GetNumberOfDevicesConnected(out UInt32 pulNumDevices)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            pulNumDevices = 0;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ListDevices != IntPtr.Zero)
            {
                var FT_ListDevices = (tFT_ListDevices)Marshal.GetDelegateForFunctionPointer(
                    pFT_ListDevices, typeof(tFT_ListDevices));

                // Call FT_ListDevices
                ftStatus = FT_ListDevices(ref pulNumDevices, IntPtr.Zero, 0x80000000);
           }

            return ftStatus;
        }


        //**************************************************************************
        // CreateDeviceInfoList
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Builds a device information list and returns the number of D3XX devices connected to the system.
        /// The list contains information about both unopen and open D3XX devices.
        /// </summary>
        /// <returns>FT_STATUS value from FT_CreateDeviceInfoList in FTD3XX.DLL</returns>
        /// <param name="pulNumDevices">The number of D3XX devices connected to the system.</param>
        public FT_STATUS CreateDeviceInfoList(out UInt32 pulNumDevices)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            pulNumDevices = 0;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_CreateDeviceInfoList != IntPtr.Zero)
            {
                var FT_CreateDeviceInfoList = (tFT_CreateDeviceInfoList)Marshal.GetDelegateForFunctionPointer(
                    pFT_CreateDeviceInfoList, typeof(tFT_CreateDeviceInfoList));

                // Call FT_CreateDeviceInfoList
                ftStatus = FT_CreateDeviceInfoList(ref pulNumDevices);
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetDeviceInfoList
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Returns a list of device information for all D3XX devices connected to the system.
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetDeviceInfoList in FTD3XX.DLL</returns>
        /// <param name="listDevices">The list of device information for all D3XX devices connected to the system.</param>
        public FT_STATUS GetDeviceInfoList(out List<FT_DEVICE_INFO> listDevices)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            listDevices = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetDeviceInfoList != IntPtr.Zero)
            {
                var FT_GetDeviceInfoList = (tFT_GetDeviceInfoList)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetDeviceInfoList, typeof(tFT_GetDeviceInfoList));

                UInt32 ulNumDevices = 0;

                // Call FT_GetDeviceInfoList
                ftStatus = FT_GetDeviceInfoList(null, ref ulNumDevices);
                if (ftStatus != FT_STATUS.FT_OK)
                {
                    return ftStatus;
                }

                if (ulNumDevices == 0)
                {
                    return FT_STATUS.FT_DEVICE_NOT_CONNECTED;
                }

                Int32 structSize = Marshal.SizeOf(typeof(FT_DEVICE_INFO));
                byte[] DevicesByteArray = new byte[structSize * ulNumDevices];

                // Call FT_GetDeviceInfoList
                ftStatus = FT_GetDeviceInfoList(DevicesByteArray, ref ulNumDevices);

                if (ftStatus == FT_STATUS.FT_OK)
                {
                    listDevices = new List<FT_DEVICE_INFO>();

                    for (UInt32 i = 0; i < ulNumDevices; i++)
                    {
                        byte[] DeviceByteArray = new byte[structSize];
                        Array.Copy(DevicesByteArray, i * structSize, DeviceByteArray, 0, structSize);

                        var pDevice = Deserialize<FT_DEVICE_INFO>(DeviceByteArray);
                        listDevices.Add(pDevice);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // OpenByIndex
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Opens a D3XX device located at the specified index in relation to the list of device information list.
        /// </summary>
        /// <returns>FT_STATUS value from FT_Create in FTD3XX.DLL</returns>
        /// <param name="index">Index of the device to open in relation to the list of device information list.
        public FT_STATUS OpenByIndex(UInt32 ulIndex)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_Create != IntPtr.Zero)
            {
                var FT_Create = (tFT_Create)Marshal.GetDelegateForFunctionPointer(
                    pFT_Create, typeof(tFT_Create));

                // Call FT_Create
                ftStatus = FT_Create(ulIndex, FT_OPEN.BY_INDEX, ref ftHandle);

                // Appears that the handle value can be non-NULL on a fail, so set it explicitly
                if (ftStatus == FT_STATUS.FT_OK)
                {
                    InitializeProperties();
                }
                else
                {
                    ftHandle = IntPtr.Zero;
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // OpenBySerialNumber
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Opens a D3XX device with the specified serial number.  
        /// </summary>
        /// <returns>FT_STATUS value from FT_Create in FTD3XX.DLL</returns>
        /// <param name="szSerialNumber">Serial number of the device to open as specified in the device information list.</param>
        public FT_STATUS OpenBySerialNumber(string szSerialNumber)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_Create != IntPtr.Zero)
            {
                var FT_Create = (tFT_CreateEx)Marshal.GetDelegateForFunctionPointer(
                    pFT_Create, typeof(tFT_CreateEx));

                // Call FT_Create
                ftStatus = FT_Create(szSerialNumber, FT_OPEN.BY_SERIAL_NUMBER, ref ftHandle);

                // Appears that the handle value can be non-NULL on a fail, so set it explicitly
                if (ftStatus == FT_STATUS.FT_OK)
                {
                    InitializeProperties();
                }
                else
                {
                    ftHandle = IntPtr.Zero;
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // OpenByDescription
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Opens a D3XX device with the specified description.
        /// </summary>
        /// <returns>FT_STATUS value from FT_Create in FTD3XX.DLL</returns>
        /// <param name="szDescription">Description of the device to open as specified in the device information list.</param>
        public FT_STATUS OpenByDescription(string szDescription)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if ((pFT_Create != IntPtr.Zero))
            {
                var FT_Create = (tFT_CreateEx)Marshal.GetDelegateForFunctionPointer(
                    pFT_Create, typeof(tFT_CreateEx));

                // Call FT_Create
                ftStatus = FT_Create(szDescription, FT_OPEN.BY_DESCRIPTION, ref ftHandle);

                // Appears that the handle value can be non-NULL on a fail, so set it explicitly
                if (ftStatus == FT_STATUS.FT_OK)
                {
                    InitializeProperties();
                }
                else
                {
                    ftHandle = IntPtr.Zero;
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // Close
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Closes the handle to an open D3XX device.  
        /// </summary>
        /// <returns>FT_STATUS value from FT_Close in FTD3XX.DLL</returns>
        public FT_STATUS Close()
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_Close != IntPtr.Zero)
            {
                var FT_Close = (tFT_Close)Marshal.GetDelegateForFunctionPointer(
                    pFT_Close, typeof(tFT_Close));

                // Call FT_Close
                ftStatus = FT_Close(ftHandle);

                ftHandle = IntPtr.Zero;

                UninitializeProperties();
            }
            else
            {
                ftStatus = FT_STATUS.FT_INVALID_PARAMETER;
            }

            return ftStatus;
        }


        //**************************************************************************
        // WritePipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Write data to a pipe synchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_WritePipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to send data to. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        public FT_STATUS WritePipe(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_WritePipe != IntPtr.Zero)
            {
                var FT_WritePipe = (tFT_WritePipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_WritePipe, typeof(tFT_WritePipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_Write
                    ftStatus = FT_WritePipe(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, IntPtr.Zero);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ReadPipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Read data from a pipe synchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ReadPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to receive data from. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer to contain the data to read.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to read. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes read from the pipe.</param>
        public FT_STATUS ReadPipe(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ReadPipe != IntPtr.Zero)
            {
                var FT_ReadPipe = (tFT_ReadPipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_ReadPipe, typeof(tFT_ReadPipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ReadPipe
                    ftStatus = FT_ReadPipe(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, IntPtr.Zero);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // WritePipeAsync
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Write data to a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_WritePipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to send data to. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS WritePipeAsync(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_WritePipe != IntPtr.Zero)
            {
                var FT_WritePipe = (tFT_WritePipeAsync)Marshal.GetDelegateForFunctionPointer(
                    pFT_WritePipe, typeof(tFT_WritePipeAsync));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_Write
                    ftStatus = FT_WritePipe(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, ref pOverlapped);
                }
            }

            return ftStatus;
        }

        //**************************************************************************
        // WritePipeEx
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Write data to a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_WritePipeEx in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to send data to. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS WritePipeEx(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_WritePipeEx != IntPtr.Zero)
            {
                var FT_WritePipeEx = (tFT_WritePipeEx)Marshal.GetDelegateForFunctionPointer(
                    pFT_WritePipeEx, typeof(tFT_WritePipeEx));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_Write
                    ftStatus = FT_WritePipeEx(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, ref pOverlapped);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // WritePipeEx
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Write data to a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_WritePipeEx in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to send data to. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS WritePipeEx(byte bPipe, IntPtr pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_WritePipeEx != IntPtr.Zero)
            {
                var FT_WritePipeEx = (tFT_WritePipeExI)Marshal.GetDelegateForFunctionPointer(
                    pFT_WritePipeEx, typeof(tFT_WritePipeExI));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_Write
                    ftStatus = FT_WritePipeEx(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, pOverlapped);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ReadPipeAsync
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Read data from a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ReadPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to receive data from. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS ReadPipeAsync(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ReadPipe != IntPtr.Zero)
            {
                var FT_ReadPipe = (tFT_ReadPipeAsync)Marshal.GetDelegateForFunctionPointer(
                    pFT_ReadPipe, typeof(tFT_ReadPipeAsync));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ReadPipe
                    ftStatus = FT_ReadPipe(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, ref pOverlapped);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ReadPipeEx
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Read data from a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ReadPipeEx in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to receive data from. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS ReadPipeEx(byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ReadPipeEx != IntPtr.Zero)
            {
                var FT_ReadPipeEx = (tFT_ReadPipeEx)Marshal.GetDelegateForFunctionPointer(
                    pFT_ReadPipeEx, typeof(tFT_ReadPipeEx));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ReadPipeEx
                    ftStatus = FT_ReadPipeEx(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, ref pOverlapped);
                }
            }

            return ftStatus;
        }

        //**************************************************************************
        // ReadPipeEx
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Read data from a pipe asynchronously.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ReadPipeEx in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to receive data from. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        /// <param name="pOverlapped">A reference to a NativeOverlapped structure.</param>
        public FT_STATUS ReadPipeEx(byte bPipe, IntPtr pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ReadPipeEx != IntPtr.Zero)
            {
                var FT_ReadPipeEx = (tFT_ReadPipeExI)Marshal.GetDelegateForFunctionPointer(
                    pFT_ReadPipeEx, typeof(tFT_ReadPipeExI));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ReadPipeEx
                    ftStatus = FT_ReadPipeEx(ftHandle, bPipe, pBuffer, ulBytesToTransfer, ref pulBytesTransferred, pOverlapped);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // WaitAsync
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Retrieves the result of an overlapped operation to a pipe
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetOverlappedResult in FTD3XX.DLL</returns>
        /// <param name="pOverlapped">A reference to an NativeOverlapped structure that was specified when the overlapped operation was started using WritePipeAsync or ReadPipeAsync.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to that receives the number of bytes that were actually transferred by a read or write operation.</param>
        /// <param name="bWait">If this parameter is true, the function does not return until the asynchronous operation has completed. If this parameter is false, the function returns FT_IO_INCOMPLETE until the asynchronous operation has completed.</param>
        public FT_STATUS WaitAsync(ref NativeOverlapped pOverlapped, ref UInt32 pulBytesTransferred, bool bWait)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetOverlappedResult != IntPtr.Zero)
            {
                var FT_GetOverlappedResult = (tFT_GetOverlappedResult)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetOverlappedResult, typeof(tFT_GetOverlappedResult));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_GetOverlappedResult
                    ftStatus = FT_GetOverlappedResult(ftHandle, ref pOverlapped, ref pulBytesTransferred, bWait);
                }
            }

            return ftStatus;
        }

        //**************************************************************************
        // WaitAsync
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Retrieves the result of an overlapped operation to a pipe
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetOverlappedResult in FTD3XX.DLL</returns>
        /// <param name="pOverlapped">A reference to an NativeOverlapped structure that was specified when the overlapped operation was started using WritePipeAsync or ReadPipeAsync.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to that receives the number of bytes that were actually transferred by a read or write operation.</param>
        /// <param name="bWait">If this parameter is true, the function does not return until the asynchronous operation has completed. If this parameter is false, the function returns FT_IO_INCOMPLETE until the asynchronous operation has completed.</param>
        public FT_STATUS WaitAsync(IntPtr pOverlapped, ref UInt32 pulBytesTransferred, bool bWait)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetOverlappedResult != IntPtr.Zero)
            {
                var FT_GetOverlappedResult = (tFT_GetOverlappedResultI)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetOverlappedResult, typeof(tFT_GetOverlappedResultI));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_GetOverlappedResult
                    ftStatus = FT_GetOverlappedResult(ftHandle, pOverlapped, ref pulBytesTransferred, bWait);
                }
            }

            return ftStatus;
        }

        //**************************************************************************
        // AbortPipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Aborts all pending and ongoing transfers in a pipe.
        /// </summary>
        /// <returns>FT_STATUS value from FT_AbortPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to be aborted.</param>
        public FT_STATUS AbortPipe(byte bPipe)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_AbortPipe != IntPtr.Zero)
            {
                var FT_AbortPipe = (tFT_AbortPipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_AbortPipe, typeof(tFT_AbortPipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_AbortPipe
                    ftStatus = FT_AbortPipe(ftHandle, bPipe);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // FlushPipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Flushes data in a pipe.
        /// </summary>
        /// <returns>FT_STATUS value from FT_FlushPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to be flushed.</param>
        public FT_STATUS FlushPipe(byte bPipe)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_FlushPipe != IntPtr.Zero)
            {
                var FT_FlushPipe = (tFT_FlushPipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_FlushPipe, typeof(tFT_FlushPipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_FlushPipe
                    ftStatus = FT_FlushPipe(ftHandle, bPipe);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetStreamPipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Enable streaming protocol transfer for specified pipe. This is for applications that transfer (write or read) a fixed size of data to or from the device.
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetStreamPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to enable streaming data.</param>
        /// <param name="ulStreamSize">Size of the data to be streamed constantly to or from the device.</param>
        public FT_STATUS SetStreamPipe(byte bPipe, UInt32 ulStreamSize)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetStreamPipe != IntPtr.Zero)
            {
                var FT_SetStreamPipe = (tFT_SetStreamPipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetStreamPipe, typeof(tFT_SetStreamPipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_SetStreamPipe
                    ftStatus = FT_SetStreamPipe(ftHandle, false, false, bPipe, ulStreamSize);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ClearStreamPipe
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Disable streaming mode in a specified pipe.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ClearStreamPipe in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to disable streaming data.</param>
        public FT_STATUS ClearStreamPipe(byte bPipe)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ClearStreamPipe != IntPtr.Zero)
            {
                var FT_ClearStreamPipe = (tFT_ClearStreamPipe)Marshal.GetDelegateForFunctionPointer(
                    pFT_ClearStreamPipe, typeof(tFT_ClearStreamPipe));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ClearStreamPipe
                    ftStatus = FT_ClearStreamPipe(ftHandle, false, false, bPipe);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetChipConfiguration
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Gets the configuration of device
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetChipConfiguration in FTD3XX.DLL</returns>
        /// <param name="oConfiguration">A reference to a FT_CONFIGURATION class object.</param>
        public FT_STATUS GetChipConfiguration(FT_60XCONFIGURATION oConfiguration)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetChipConfiguration != IntPtr.Zero)
            {
                var FT_GetChipConfiguration = (tFT_GetChipConfiguration)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetChipConfiguration, typeof(tFT_GetChipConfiguration));

                if (ftHandle != IntPtr.Zero)
                {
                    // FT600/FT601 has bcdDevice of 0
                    if (oDeviceDescriptor.bcdDevice == 0)
                    {
                        byte[] ConfigurationByteArray = new byte[FT_60XCONFIGURATION_SIZE];

                        // Call FT_GetChipConfiguration
                        ftStatus = FT_GetChipConfiguration(ftHandle, ConfigurationByteArray);

                        if (ftStatus == FT_STATUS.FT_OK)
                        {

                            FT_60XCONFIGURATION_INTERNAL pConfigurationInternal;
                            GCHandle handle = GCHandle.Alloc(ConfigurationByteArray, GCHandleType.Pinned);
                            try
                            {
                                pConfigurationInternal = (FT_60XCONFIGURATION_INTERNAL)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(FT_60XCONFIGURATION_INTERNAL));
                            }
                            finally
                            {
                                handle.Free();
                            }

                            //var pConfigurationInternal = Deserialize<FT_60XCONFIGURATION_INTERNAL>(ConfigurationByteArray);

                            FT_60XCONFIGURATION pConfiguration = (FT_60XCONFIGURATION)oConfiguration;
                            pConfiguration.VendorID = (UInt16)pConfigurationInternal.VendorID;
                            pConfiguration.ProductID = (UInt16)pConfigurationInternal.ProductID;
                            pConfiguration.PowerAttributes = (byte)pConfigurationInternal.PowerAttributes;
                            pConfiguration.PowerConsumption = (UInt16)pConfigurationInternal.PowerConsumption;
                            pConfiguration.FIFOMode = (byte)pConfigurationInternal.FIFOMode;
                            pConfiguration.ChannelConfig = (byte)pConfigurationInternal.ChannelConfig;
                            pConfiguration.OptionalFeatureSupport = (UInt16)pConfigurationInternal.OptionalFeatureSupport;
                            pConfiguration.BatteryChargingGPIOConfig = (byte)pConfigurationInternal.BatteryChargingGPIOConfig;
                            pConfiguration.FlashEEPROMDetection = (byte)pConfigurationInternal.FlashEEPROMDetection;
                            pConfiguration.MSIO_Control = (UInt32)pConfigurationInternal.MSIO_Control;
                            pConfiguration.GPIO_Control = (UInt32)pConfigurationInternal.GPIO_Control;

                            pConfiguration.FIFOClock = (byte)pConfigurationInternal.FIFOClock;

                            byte offset = 4;
                            byte len = 0;
                            byte[] arr = null;

                            len = (byte)(ConfigurationByteArray[offset] - 2);
                            arr = new byte[len];
                            Array.Copy(ConfigurationByteArray, offset + 2, arr, 0, len);
                            pConfiguration.Manufacturer = Encoding.Unicode.GetString(arr);
                            offset += (byte)(len + 2);

                            len = (byte)(ConfigurationByteArray[offset] - 2);
                            arr = new byte[len];
                            Array.Copy(ConfigurationByteArray, offset + 2, arr, 0, len);
                            pConfiguration.Description = Encoding.Unicode.GetString(arr);
                            offset += (byte)(len + 2);

                            len = (byte)(ConfigurationByteArray[offset] - 2);
                            arr = new byte[len];
                            Array.Copy(ConfigurationByteArray, offset + 2, arr, 0, len);
                            pConfiguration.SerialNumber = Encoding.Unicode.GetString(arr);
                        }
                    }
                    else
                    {
                        // For future D3XX devices
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetChipConfiguration
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Sets the configuration of device
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetChipConfiguration in FTD3XX.DLL</returns>
        /// <param name="oConfiguration">A reference to a FT_CONFIGURATION class object.</param>
        public FT_STATUS SetChipConfiguration(FT_60XCONFIGURATION oConfiguration)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetChipConfiguration != IntPtr.Zero)
            {
                var FT_SetChipConfiguration = (tFT_SetChipConfiguration)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetChipConfiguration, typeof(tFT_SetChipConfiguration));

                if (ftHandle != IntPtr.Zero)
                {
                    // FT600/FT601 has bcdDevice of 0
                    if (oDeviceDescriptor.bcdDevice == 0)
                    {
                        FT_60XCONFIGURATION pConfiguration = (FT_60XCONFIGURATION)oConfiguration;

                        if (pConfiguration.Manufacturer.Length > FT_MANUFACTURER_MAXLEN ||
                            pConfiguration.Description.Length > FT_DESCRIPTION_MAXLEN ||
                            pConfiguration.SerialNumber.Length > FT_SERIALNUMBER_MAXLEN)
                        {
                            return FT_STATUS.FT_INVALID_PARAMETER;
                        }

                        FT_60XCONFIGURATION_INTERNAL pConfigurationInternal = new FT_60XCONFIGURATION_INTERNAL();
                        pConfigurationInternal.VendorID = pConfiguration.VendorID;
                        pConfigurationInternal.ProductID = pConfiguration.ProductID;
                        pConfigurationInternal.Reserved = 0;
                        pConfigurationInternal.PowerAttributes = pConfiguration.PowerAttributes;
                        pConfigurationInternal.PowerConsumption = pConfiguration.PowerConsumption;
                        pConfigurationInternal.Reserved2 = 0;
                        pConfigurationInternal.FIFOClock = pConfiguration.FIFOClock;
                        pConfigurationInternal.FIFOMode = pConfiguration.FIFOMode;
                        pConfigurationInternal.ChannelConfig = pConfiguration.ChannelConfig;
                        pConfigurationInternal.OptionalFeatureSupport = pConfiguration.OptionalFeatureSupport;
                        pConfigurationInternal.BatteryChargingGPIOConfig = pConfiguration.BatteryChargingGPIOConfig;
                        pConfigurationInternal.FlashEEPROMDetection = 0;
                        pConfigurationInternal.MSIO_Control = pConfiguration.MSIO_Control;
                        pConfigurationInternal.GPIO_Control = pConfiguration.GPIO_Control;

                        byte[] ConfigurationByteArray = Serialize(pConfigurationInternal);

                        byte[] Manufacturer = System.Text.Encoding.Unicode.GetBytes(pConfiguration.Manufacturer);
                        byte[] Description = System.Text.Encoding.Unicode.GetBytes(pConfiguration.Description);
                        byte[] SerialNumber = System.Text.Encoding.Unicode.GetBytes(pConfiguration.SerialNumber);

                        byte offset = 4;
                        byte length = 0;

                        length = (byte)(pConfiguration.Manufacturer.Length * 2);
                        ConfigurationByteArray[offset] = (byte)(length + 2);
                        ConfigurationByteArray[offset + 1] = 0x03;
                        Array.Copy(Manufacturer, 0, ConfigurationByteArray, offset + 2, length);
                        offset += (byte)(length + 2);

                        length = (byte)(pConfiguration.Description.Length * 2);
                        ConfigurationByteArray[offset] = (byte)(length + 2);
                        ConfigurationByteArray[offset + 1] = 0x03;
                        Array.Copy(Description, 0, ConfigurationByteArray, offset + 2, length);
                        offset += (byte)(length + 2);

                        length = (byte)(pConfiguration.SerialNumber.Length * 2);
                        ConfigurationByteArray[offset] = (byte)(length + 2);
                        ConfigurationByteArray[offset + 1] = 0x03;
                        Array.Copy(SerialNumber, 0, ConfigurationByteArray, offset + 2, length);

                        // Call FT_SetChipConfiguration
                        ftStatus = FT_SetChipConfiguration(ftHandle, ConfigurationByteArray);
                    }
                    else
                    {
                        // For future D3XX devices
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ResetChipConfiguration
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Sets the configuration of device to default values
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetChipConfiguration in FTD3XX.DLL</returns>
        public FT_STATUS ResetChipConfiguration()
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetChipConfiguration != IntPtr.Zero)
            {
                var FT_ResetChipConfiguration = (tFT_ResetChipConfiguration)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetChipConfiguration, typeof(tFT_ResetChipConfiguration));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ResetChipConfiguration
                    ftStatus = FT_ResetChipConfiguration(ftHandle, IntPtr.Zero);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // EnableGPIO
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Configures pin into GPIO mode and configure direction.
        /// </summary>
        /// <returns>FT_STATUS value from FT_EnableGPIO in FTD3XX.DLL</returns>
        /// <param name="ulMask">Mask to select the bits to be written (1=write, 0=ignore).</param>
        /// <param name="ulDirection">BIT0 controls the direction of GPIO0 and BIT1 controls the direction of GPIO1. (0=IN, 1=OUT)</param>
        public FT_STATUS EnableGPIO(UInt32 ulMask, UInt32 ulDirection)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_EnableGPIO != IntPtr.Zero)
            {
                var FT_EnableGPIO = (tFT_EnableGPIO)Marshal.GetDelegateForFunctionPointer(
                    pFT_EnableGPIO, typeof(tFT_EnableGPIO));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_EnableGPIO
                    ftStatus = FT_EnableGPIO(ftHandle, ulMask, ulDirection);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // WriteGPIO
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Sets the status of GPIO0 and GPIO1
        /// </summary>
        /// <returns>FT_STATUS value from FT_WriteGPIO in FTD3XX.DLL</returns>
        /// <param name="ulMask">Mask to select the bits to be written (1=write, 0=ignore).</param>
        /// <param name="ulData">Data to write the GPIO status. Bit0 and Bit1 corresponds to GPIO0 and GPIO1. (1=high, 0=low)</param>
        public FT_STATUS WriteGPIO(UInt32 ulMask, UInt32 ulData)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_WriteGPIO != IntPtr.Zero)
            {
                var FT_WriteGPIO = (tFT_WriteGPIO)Marshal.GetDelegateForFunctionPointer(
                    pFT_WriteGPIO, typeof(tFT_WriteGPIO));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_WriteGPIO
                    ftStatus = FT_WriteGPIO(ftHandle, ulMask, ulData);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ReadGPIO
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Returns the status of GPIO0 and GPIO1
        /// </summary>
        /// <returns>FT_STATUS value from FT_ReadGPIO in FTD3XX.DLL</returns>
        /// <param name="pulData">Pointer to contain the GPIO status. BIT0 and BIT1 reflect the GPIO pin status. (1=HIGH, 0=LOW).</param>
        public FT_STATUS ReadGPIO(ref UInt32 pulData)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ReadGPIO != IntPtr.Zero)
            {
                var FT_ReadGPIO = (tFT_ReadGPIO)Marshal.GetDelegateForFunctionPointer(
                    pFT_ReadGPIO, typeof(tFT_ReadGPIO));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ReadGPIO
                    ftStatus = FT_ReadGPIO(ftHandle, ref pulData);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetGPIOPull
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Set GPIO internal pull resisters
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetGPIOPull in FTD3XX.DLL</returns>
        /// <param name="u32Mask">Each bit represents one GPIO setting, GPIO0-GPIO2 from LSB to MSB, set bit to 0 to skip the GPIO, 1 to enable the GPIO.</param>
        /// <param name="u32Pull">Each two bits represents one GPIO setting, GPIO0-GPIO2 from LSB to MSB.</param>
        public FT_STATUS SetGPIOPull(UInt32 ulMask, UInt32 ulPull)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetGPIOPull != IntPtr.Zero)
            {
                var FT_SetGPIOPull = (tFT_SetGPIOPull)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetGPIOPull, typeof(tFT_SetGPIOPull));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_SetGPIOPull
                    ftStatus = FT_SetGPIOPull(ftHandle, ulMask, ulPull);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetNotificationCallback
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Sets a receive notification callback function which will be called when data is available for IN endpoints where no read requests are currently ongoing.
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetNotificationCallback in FTD3XX.DLL</returns>
        /// <param name="pCallback">A delegate to a callback function to be called by the library to indicate DATA status availability in one of the IN endpoints.</param>
        /// <param name="pvCallbackContext">A pointer to the user context that will be used when the callback function is called.</param>
        public FT_STATUS SetNotificationCallback(FT_NOTIFICATION_CALLBACK_DATA pCallback, IntPtr pvCallbackContext)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetNotificationCallback != IntPtr.Zero)
            {
                var FT_SetNotificationCallback = (tFT_SetNotificationCallback)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetNotificationCallback, typeof(tFT_SetNotificationCallback));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_SetNotificationCallback
                    ftStatus = FT_SetNotificationCallback(ftHandle, pCallback, pvCallbackContext);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ClearNotificationCallback
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Clears the notification callback set by GetGPIO or SetNotificationCallback.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ClearNotificationCallback in FTD3XX.DLL</returns>
        public FT_STATUS ClearNotificationCallback()
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ClearNotificationCallback != IntPtr.Zero)
            {
                var FT_ClearNotificationCallback = (tFT_ClearNotificationCallback)Marshal.GetDelegateForFunctionPointer(
                    pFT_ClearNotificationCallback, typeof(tFT_ClearNotificationCallback));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ClearNotificationCallback
                    ftStatus = FT_ClearNotificationCallback(ftHandle);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // CycleDevicePort
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Power cycles the device port. 
        /// This causes the device to be re-enumerated by the host system.
        /// </summary>
        /// <returns>FT_STATUS value from FT_CycleDevicePort in FTD3XX.DLL</returns>
        public FT_STATUS CycleDevicePort()
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_CycleDevicePort != IntPtr.Zero)
            {
                var FT_CycleDevicePort = (tFT_CycleDevicePort)Marshal.GetDelegateForFunctionPointer(
                    pFT_CycleDevicePort, typeof(tFT_CycleDevicePort));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_CycleDevicePort
                    ftStatus = FT_CycleDevicePort(ftHandle);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // ResetDevicePort
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Resets the device port.
        /// This also causes the device to be re-enumerated by the host system.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ResetDevicePort in FTD3XX.DLL</returns>
        public FT_STATUS ResetDevicePort()
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ResetDevicePort != IntPtr.Zero)
            {
                var FT_ResetDevicePort = (tFT_ResetDevicePort)Marshal.GetDelegateForFunctionPointer(
                    pFT_ResetDevicePort, typeof(tFT_ResetDevicePort));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_ResetDevicePort
                    ftStatus = FT_ResetDevicePort(ftHandle);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // IsDevicePath
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Checks if the device path provided is same as the device path of the current handle.
        /// This is used for Hotplugging with Win32 RegisterDeviceNotification
        /// </summary>
        /// <returns>FT_STATUS value from FT_IsDevicePath in FTD3XX.DLL</returns>
        /// <param name="szDevicePath">Device path to compare with.</param>
        public FT_STATUS IsDevicePath(string szDevicePath)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_IsDevicePath != IntPtr.Zero)
            {
                var FT_IsDevicePath = (tFT_IsDevicePath)Marshal.GetDelegateForFunctionPointer(
                    pFT_IsDevicePath, typeof(tFT_IsDevicePath));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] array = Encoding.ASCII.GetBytes(szDevicePath);

                    // Call FT_IsDevicePath
                    ftStatus = FT_IsDevicePath(ftHandle, array);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetPipeTimeout
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Configures the timeout value of a given endpoint.
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetPipeTimeout in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to configure. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="ulTimeoutInMs">Time out in milliseconds the request on the specified pipe will timeout.</param>
        public FT_STATUS SetPipeTimeout(byte bPipe, UInt32 ulTimeoutInMs)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetPipeTimeout != IntPtr.Zero)
            {
                var FT_SetPipeTimeout = (tFT_SetPipeTimeout)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetPipeTimeout, typeof(tFT_SetPipeTimeout));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_SetPipeTimeout
                    ftStatus = FT_SetPipeTimeout(ftHandle, bPipe, ulTimeoutInMs);
                }
            }
            else
            {
                ftStatus = FT_STATUS.FT_NOT_SUPPORTED;
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetPipeTimeout
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Gets the timeout value configured for a given endpoint.
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetPipeTimeout in FTD3XX.DLL</returns>
        /// <param name="bPipe">Pipe to configure. Corresponds to the PipeId field in the FT_PIPE_INFORMATION structure. In the PipeId field, Bit 7 indicates the direction of the endpoint: 0 for OUT; 1 for IN.</param>
        /// <param name="pulTimeoutInMs">Time out in milliseconds the request on the specified pipe will timeout.</param>
        public FT_STATUS GetPipeTimeout(byte bPipe, ref UInt32 pulTimeoutInMs)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetPipeTimeout != IntPtr.Zero)
            {
                var FT_GetPipeTimeout = (tFT_GetPipeTimeout)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetPipeTimeout, typeof(tFT_GetPipeTimeout));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_GetPipeTimeout
                    ftStatus = FT_GetPipeTimeout(ftHandle, bPipe, ref pulTimeoutInMs);
                }
            }
            else
            {
                ftStatus = FT_STATUS.FT_NOT_SUPPORTED;
            }

            return ftStatus;
        }


        //**************************************************************************
        // SetSuspendTimeout
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Configures the idle timeout value for USB Selective Suspend.
        /// </summary>
        /// <returns>FT_STATUS value from FT_SetSuspendTimeout in FTD3XX.DLL</returns>
        /// <param name="ulTimeout">Idle timeout value before the device will be suspended to save power.</param>
        public FT_STATUS SetSuspendTimeout(UInt32 ulTimeout)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_SetSuspendTimeout != IntPtr.Zero)
            {
                var FT_SetSuspendTimeout = (tFT_SetSuspendTimeout)Marshal.GetDelegateForFunctionPointer(
                    pFT_SetSuspendTimeout, typeof(tFT_SetSuspendTimeout));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_SetSuspendTimeout
                    ftStatus = FT_SetSuspendTimeout(ftHandle, ulTimeout);
                }
            }
            else
            {
                ftStatus = FT_STATUS.FT_NOT_SUPPORTED;
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetSuspendTimeout
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Gets the idle timeout value for USB Selective Suspend.
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetSuspendTimeout in FTD3XX.DLL</returns>
        /// <param name="ulTimeout">Idle timeout value before the device will be suspended to save power.</param>
        public FT_STATUS GetSuspendTimeout(ref UInt32 pulTimeout)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetSuspendTimeout != IntPtr.Zero)
            {
                var FT_GetSuspendTimeout = (tFT_GetSuspendTimeout)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetSuspendTimeout, typeof(tFT_GetSuspendTimeout));

                if (ftHandle != IntPtr.Zero)
                {
                    // Call FT_GetSuspendTimeout
                    ftStatus = FT_GetSuspendTimeout(ftHandle, ref pulTimeout);
                }
            }
            else
            {
                ftStatus = FT_STATUS.FT_NOT_SUPPORTED;
            }

            return ftStatus;
        }


        //**************************************************************************
        // ControlTransfer
        //**************************************************************************
        // Intellisense comments
        /// <summary>
        /// Send control request to control pipe.
        /// </summary>
        /// <returns>FT_STATUS value from FT_ControlTransfer in FTD3XX.DLL</returns>
        /// <param name="tSetupPacket">Control setup packet to send to the control pipe.</param>
        /// <param name="pBuffer">Buffer that contains the data to write.</param>
        /// <param name="ulBytesToTransfer">The number of bytes to write. This number must be less than or equal to the size, in bytes, of the Buffer.</param>
        /// <param name="pulBytesTransferred">A reference to an unsigned integer to contain the number of bytes written to the pipe.</param>
        public FT_STATUS ControlTransfer(FT_SETUP_PACKET tSetupPacket, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_ControlTransfer != IntPtr.Zero)
            {
                var FT_ControlTransfer = (tFT_ControlTransfer)Marshal.GetDelegateForFunctionPointer(
                    pFT_ControlTransfer, typeof(tFT_ControlTransfer));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] SetupPacketByteArray = Serialize(tSetupPacket);
                    UInt64 SetupPacketUInt64 = BitConverter.ToUInt64(SetupPacketByteArray, 0);

                    // Call FT_ControlTransfer
                    ftStatus = FT_ControlTransfer(ftHandle, SetupPacketUInt64, pBuffer, ulBytesToTransfer, ref pulBytesTransferred);
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetSerialNumber
        //**************************************************************************
        /// <summary>
        /// Get serial number string of the provided device info
        /// </summary>
        /// <param name="oDeviceInfo">A device info retrieved from the list of device info via GetDeviceInfoList</param>
        public string GetSerialNumber(FT_DEVICE_INFO oDeviceInfo)
        {
            if (oDeviceInfo == null)
            {
                return null;
            }

            int i;
            for (i = 0; i < oDeviceInfo.SerialNumber.Length; i++)
            {
                if (oDeviceInfo.SerialNumber[i] == 0x0)
                {
                    break;
                }
            }

            byte[] SerialNumber = new byte[i];
            Array.Copy(oDeviceInfo.SerialNumber, SerialNumber, i);

            return System.Text.Encoding.ASCII.GetString(SerialNumber);
        }


        //**************************************************************************
        // GetDescription
        //**************************************************************************
        /// <summary>
        /// Get description string of the provided device info
        /// </summary>
        /// <param name="oDeviceInfo">A device info retrieved from the list of device info via GetDeviceInfoList</param>
        public string GetDescription(FT_DEVICE_INFO oDeviceInfo)
        {
            if (oDeviceInfo == null)
            {
                return null;
            }

            int i;
            for (i = 0; i < oDeviceInfo.Description.Length; i++)
            {
                if (oDeviceInfo.Description[i] == 0x0)
                {
                    break;
                }
            }

            byte[] Description = new byte[i];
            Array.Copy(oDeviceInfo.Description, Description, i);

            return System.Text.Encoding.ASCII.GetString(Description);
        }


        //**************************************************************************
        // FT_EXCEPTION
        //**************************************************************************
        /// <summary>
        /// Exceptions thrown by errors within the FTDI class.
        /// </summary>
        [global::System.Serializable]
        public class FT_EXCEPTION : Exception
        {
            /// <summary>
            /// 
            /// </summary>
            public FT_EXCEPTION() { }
            /// <summary>
            /// 
            /// </summary>
            /// <param name="message"></param>
            public FT_EXCEPTION(string message) : base(message) { }
            /// <summary>
            /// 
            /// </summary>
            /// <param name="message"></param>
            /// <param name="inner"></param>
            public FT_EXCEPTION(string message, Exception inner) : base(message, inner) { }

        }
    }
}
