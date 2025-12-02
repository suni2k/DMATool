/*
** FTD3XX_NET.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for .NET wrapper of the Windows FTD3XX.dll API calls.
** Main module
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
using System.IO;
#if !NETSTANDARD2_0
using System.Windows.Forms;
#endif

namespace FTD3XXWU_NET
{
    /// <summary>
    /// D3XX .NET Class Library (FTD3XX_NET.DLL) for D3XX Library (FTD3XX.DLL)
    /// </summary>
    public partial class FTDI
    {
        #region CONSTRUCTOR_DESTRUCTOR

        // constructor
        /// <summary>
        /// Constructor for the FTDI class.
        /// </summary>
        public FTDI()
        {
            // If FTD3XX.DLL is NOT loaded already, load it
            if (hFTD3XXDLL == IntPtr.Zero)
            {
                // Load our FTD3XX.DLL library
				hFTD3XXDLL = LoadLibrary("FTD3XXWU.DLL");
                if (hFTD3XXDLL == IntPtr.Zero)
                {
					// Failed to load our FTD3XX.DLL library from System32 or the application directory
					// Try the same directory that this FTD3XX_NET DLL is in
					//MessageBox.Show("Attempting to load FTD3XX.DLL from:\n" + Path.GetDirectoryName(GetType().Assembly.Location));
                    string path = @Path.GetDirectoryName(GetType().Assembly.Location) + "\\" + "FTD3XXWU.DLL";
					hFTD3XXDLL = LoadLibrary(path);
                }
            }

            // If we have succesfully loaded the library, get the function pointers set up
            if (hFTD3XXDLL != IntPtr.Zero)
            {
				// Set up our function pointers for use through our exported methods
                pFT_CreateDeviceInfoList = GetProcAddress(hFTD3XXDLL, "FT_CreateDeviceInfoList");
                pFT_GetDeviceInfoList = GetProcAddress(hFTD3XXDLL, "FT_GetDeviceInfoList");
                pFT_ListDevices = GetProcAddress(hFTD3XXDLL, "FT_ListDevices");
                pFT_Create = GetProcAddress(hFTD3XXDLL, "FT_Create");
                pFT_Close = GetProcAddress(hFTD3XXDLL, "FT_Close");
                pFT_GetFirmwareVersion = GetProcAddress(hFTD3XXDLL, "FT_GetFirmwareVersion");
                pFT_GetDriverVersion = GetProcAddress(hFTD3XXDLL, "FT_GetDriverVersion");
                pFT_GetLibraryVersion = GetProcAddress(hFTD3XXDLL, "FT_GetLibraryVersion");
                pFT_GetDeviceDescriptor = GetProcAddress(hFTD3XXDLL, "FT_GetDeviceDescriptor");
                pFT_GetConfigurationDescriptor = GetProcAddress(hFTD3XXDLL, "FT_GetConfigurationDescriptor");
                pFT_GetInterfaceDescriptor = GetProcAddress(hFTD3XXDLL, "FT_GetInterfaceDescriptor");
                pFT_GetPipeInformation = GetProcAddress(hFTD3XXDLL, "FT_GetPipeInformation");
                pFT_GetStringDescriptor = GetProcAddress(hFTD3XXDLL, "FT_GetStringDescriptor");
                pFT_GetVIDPID = GetProcAddress(hFTD3XXDLL, "FT_GetVIDPID");
                pFT_GetChipConfiguration = GetProcAddress(hFTD3XXDLL, "FT_GetChipConfiguration");
                pFT_SetChipConfiguration = GetProcAddress(hFTD3XXDLL, "FT_SetChipConfiguration");
                pFT_ReadPipe = GetProcAddress(hFTD3XXDLL, "FT_ReadPipe");
                pFT_WritePipe = GetProcAddress(hFTD3XXDLL, "FT_WritePipe");
                pFT_ReadPipeEx = GetProcAddress(hFTD3XXDLL, "FT_ReadPipeEx");
                pFT_WritePipeEx = GetProcAddress(hFTD3XXDLL, "FT_WritePipeEx");
                pFT_AbortPipe = GetProcAddress(hFTD3XXDLL, "FT_AbortPipe");
                pFT_FlushPipe = GetProcAddress(hFTD3XXDLL, "FT_FlushPipe");
                pFT_SetStreamPipe = GetProcAddress(hFTD3XXDLL, "FT_SetStreamPipe");
                pFT_ClearStreamPipe = GetProcAddress(hFTD3XXDLL, "FT_ClearStreamPipe");
                pFT_GetOverlappedResult = GetProcAddress(hFTD3XXDLL, "FT_GetOverlappedResult");
                pFT_CycleDevicePort = GetProcAddress(hFTD3XXDLL, "FT_CycleDevicePort");
                pFT_ResetDevicePort = GetProcAddress(hFTD3XXDLL, "FT_ResetDevicePort");
                pFT_EnableGPIO = GetProcAddress(hFTD3XXDLL, "FT_EnableGPIO");
                pFT_WriteGPIO = GetProcAddress(hFTD3XXDLL, "FT_WriteGPIO");
                pFT_ReadGPIO = GetProcAddress(hFTD3XXDLL, "FT_ReadGPIO");
                pFT_SetGPIOPull = GetProcAddress(hFTD3XXDLL, "FT_SetGPIOPull");
                pFT_SetNotificationCallback = GetProcAddress(hFTD3XXDLL, "FT_SetNotificationCallback");
                pFT_ClearNotificationCallback = GetProcAddress(hFTD3XXDLL, "FT_ClearNotificationCallback");
                pFT_IsDevicePath = GetProcAddress(hFTD3XXDLL, "FT_IsDevicePath");
                pFT_SetPipeTimeout = GetProcAddress(hFTD3XXDLL, "FT_SetPipeTimeout");
                pFT_GetPipeTimeout = GetProcAddress(hFTD3XXDLL, "FT_GetPipeTimeout");
                pFT_SetSuspendTimeout = GetProcAddress(hFTD3XXDLL, "FT_SetSuspendTimeout");
                pFT_GetSuspendTimeout = GetProcAddress(hFTD3XXDLL, "FT_GetSuspendTimeout");
                pFT_ControlTransfer = GetProcAddress(hFTD3XXDLL, "FT_ControlTransfer");
            }
            else
            {
				// Failed to load our DLL - alert the user
                UInt32 ulGLE = GetLastError();
#if !NETSTANDARD2_0
                MessageBox.Show("FTD3XX_NET.dll failed to load FTD3XXWU.DLL.", "Error Message");
#else
    throw new DllNotFoundException("FTD3XXWU.DLL failed to load");
#endif
                throw new Exception("FTD3XX_NET.dll failed to load FTD3XXWU.DLL.");
            }
        }

        /// <summary>
        /// Destructor for the FTDI class.
        /// </summary>
        ~FTDI()
        {
			// FreeLibrary here - we should only do this if we are completely finished
			FreeLibrary(hFTD3XXDLL);
            hFTD3XXDLL = IntPtr.Zero;
        }

        #endregion

		#region LOAD_LIBRARY

		/// <summary>
		/// Built-in Windows API functions to allow us to dynamically load our own DLL.
		/// Will allow us to use old versions of the DLL that do not have all of these functions available.
		/// </summary>
		[DllImport("kernel32.dll", CallingConvention=CallingConvention.StdCall)]
		private static extern IntPtr LoadLibrary(string dllToLoad);
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.StdCall)]
		private static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.StdCall)]
		private static extern bool FreeLibrary(IntPtr hModule);
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern UInt32 GetLastError();

        /// <summary>
        /// Handle to our DLL - used with GetProcAddress to load all of our functions
        /// Declare pointers to each of the functions we are going to use in FTD3XX.DLL
        /// These are assigned in our constructor and freed in our destructor.
        /// </summary>
        IntPtr hFTD3XXDLL = IntPtr.Zero;
        IntPtr pFT_CreateDeviceInfoList = IntPtr.Zero;
        IntPtr pFT_GetDeviceInfoList = IntPtr.Zero;
        IntPtr pFT_ListDevices = IntPtr.Zero;
        IntPtr pFT_Create = IntPtr.Zero;
        IntPtr pFT_Close = IntPtr.Zero;
        IntPtr pFT_GetFirmwareVersion = IntPtr.Zero;
        IntPtr pFT_GetDriverVersion = IntPtr.Zero;
        IntPtr pFT_GetLibraryVersion = IntPtr.Zero;
        IntPtr pFT_GetDeviceDescriptor = IntPtr.Zero;
        IntPtr pFT_GetConfigurationDescriptor = IntPtr.Zero;
        IntPtr pFT_GetInterfaceDescriptor = IntPtr.Zero;
        IntPtr pFT_GetPipeInformation = IntPtr.Zero;
        IntPtr pFT_GetStringDescriptor = IntPtr.Zero;
        IntPtr pFT_GetVIDPID = IntPtr.Zero;
        IntPtr pFT_GetChipConfiguration = IntPtr.Zero;
        IntPtr pFT_SetChipConfiguration = IntPtr.Zero;
        IntPtr pFT_WritePipe = IntPtr.Zero;
        IntPtr pFT_ReadPipe = IntPtr.Zero;
        IntPtr pFT_WritePipeEx = IntPtr.Zero;
        IntPtr pFT_ReadPipeEx = IntPtr.Zero;
        IntPtr pFT_AbortPipe = IntPtr.Zero;
        IntPtr pFT_FlushPipe = IntPtr.Zero;
        IntPtr pFT_SetStreamPipe = IntPtr.Zero;
        IntPtr pFT_ClearStreamPipe = IntPtr.Zero;
        IntPtr pFT_GetOverlappedResult = IntPtr.Zero;
        IntPtr pFT_CycleDevicePort = IntPtr.Zero;
        IntPtr pFT_ResetDevicePort = IntPtr.Zero;
        IntPtr pFT_EnableGPIO = IntPtr.Zero;
        IntPtr pFT_WriteGPIO = IntPtr.Zero;
        IntPtr pFT_ReadGPIO = IntPtr.Zero;
        IntPtr pFT_SetGPIOPull = IntPtr.Zero;
        IntPtr pFT_SetNotificationCallback = IntPtr.Zero;
        IntPtr pFT_ClearNotificationCallback = IntPtr.Zero;
        IntPtr pFT_IsDevicePath = IntPtr.Zero;
        IntPtr pFT_SetPipeTimeout = IntPtr.Zero;
        IntPtr pFT_GetPipeTimeout = IntPtr.Zero;
        IntPtr pFT_SetSuspendTimeout = IntPtr.Zero;
        IntPtr pFT_GetSuspendTimeout = IntPtr.Zero;
        IntPtr pFT_ControlTransfer = IntPtr.Zero;

        #endregion


        #region CONSTANTS_PRIVATE

        /// <summary>
        /// Open by options supported by FTD3XX DLL.
        /// </summary>
        private enum FT_OPEN
        {
            /// <summary>
            /// Open by serial number
            /// </summary>
            BY_SERIAL_NUMBER = 0x00000001,
            /// <summary>
            /// Open by description
            /// </summary>
            BY_DESCRIPTION = 0x00000002,
            /// <summary>
            /// Open by index
            /// </summary>
            BY_INDEX = 0x00000010
        }

        /// <summary>
        /// Maximum length of strings in the chip configuration
        /// </summary>
        private const byte FT_MANUFACTURER_MAXLEN = 15;
        private const byte FT_DESCRIPTION_MAXLEN = 31;
        private const byte FT_SERIALNUMBER_MAXLEN = 15;

        #endregion

        #region CLASSES_PRIVATE

        /// <summary>
        /// FT60X chip configuration
        /// </summary>
        [StructLayout(LayoutKind.Explicit, Size = 152, Pack = 1)]
        private class FT_60XCONFIGURATION_INTERNAL
        {
            // Device Descriptor
            [MarshalAs(UnmanagedType.U2)]
            [FieldOffset(0)]
            public UInt16 VendorID;

            [MarshalAs(UnmanagedType.U2)]
            [FieldOffset(2)]
            public UInt16 ProductID;


            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(4)]
            public UInt32 res1;

            // String Descriptors
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 124)]
            [FieldOffset(8)]
            public byte[] res2;

            // Configuration Descriptor
            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(132)]
            public byte Reserved;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(133)]
            public byte PowerAttributes;

            [MarshalAs(UnmanagedType.U2)]
            [FieldOffset(134)]
            public UInt16 PowerConsumption;

            // Data Transfer Configuration
            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(136)]
            public byte Reserved2;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(137)]
            public byte FIFOClock;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(138)]
            public byte FIFOMode;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(139)]
            public byte ChannelConfig;

            // Optional Feature Support
            [MarshalAs(UnmanagedType.U2)]
            [FieldOffset(140)]
            public UInt16 OptionalFeatureSupport;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(142)]
            public byte BatteryChargingGPIOConfig;

            [MarshalAs(UnmanagedType.U1)]
            [FieldOffset(143)]
            public byte FlashEEPROMDetection;      // Read-only

            // MSIO and GPIO Configuration
            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(144)]
            public UInt32 MSIO_Control;

            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(148)]
            public UInt32 GPIO_Control;
        };

        #endregion

        #region DELEGATES_PRIVATE

        /// <summary>
        /// Delegates declaration for the FTD3XX.DLL functions
        /// </summary>
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_CreateDeviceInfoList(ref UInt32 pulNumDevices);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetDeviceInfoList(byte[] pDevices, ref UInt32 pulNumDevices);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ListDevices(ref UInt32 pulNumDevices, IntPtr pArg, UInt32 ulFlags);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_Create(UInt32 ulIndex, FT_OPEN ulFlags, ref IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_CreateEx(string szString, FT_OPEN ulFlags, ref IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_Close(IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetFirmwareVersion(IntPtr ftHandle, ref UInt32 pulVersion);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetDriverVersion(IntPtr ftHandle, ref UInt32 pulVersion);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetLibraryVersion(ref UInt32 pulVersion);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetDeviceDescriptor(IntPtr ftHandle, byte[] pDescriptor);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetConfigurationDescriptor(IntPtr ftHandle, byte[] pDescriptor);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetInterfaceDescriptor(IntPtr ftHandle, byte bInterfaceIndex, byte[] pDescriptor);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetPipeInformation(IntPtr ftHandle, byte bInterfaceIndex, byte bPipeIndex, byte[] pDescriptor);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetStringDescriptor(IntPtr ftHandle, byte bStringIndex, byte[] pDescriptor);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetVIDPID(IntPtr ftHandle, ref UInt16 puwVID, ref UInt16 puwPID);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetChipConfiguration(IntPtr ftHandle, byte[] pConfiguration);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetChipConfiguration(IntPtr ftHandle, byte[] pConfiguration);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ResetChipConfiguration(IntPtr ftHandle, IntPtr pConfiguration);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_WritePipe(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ReadPipe(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_WritePipeAsync(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ReadPipeAsync(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_WritePipeEx(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_WritePipeExI(IntPtr ftHandle, byte bPipe, IntPtr pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ReadPipeEx(IntPtr ftHandle, byte bPipe, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, ref NativeOverlapped pOverlapped);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ReadPipeExI(IntPtr ftHandle, byte bPipe, IntPtr pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred, IntPtr pOverlapped);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetOverlappedResult(IntPtr ftHandle, ref NativeOverlapped pOverlapped, ref UInt32 pulBytesTransferred, bool bWait);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetOverlappedResultI(IntPtr ftHandle, IntPtr pOverlapped, ref UInt32 pulBytesTransferred, bool bWait);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_AbortPipe(IntPtr ftHandle, byte bPipe);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_FlushPipe(IntPtr ftHandle, byte bPipe);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetStreamPipe(IntPtr ftHandle, bool bAllWritePipes, bool bAllReadPipes, byte bPipe, UInt32 ulStreamSize);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ClearStreamPipe(IntPtr ftHandle, bool bAllWritePipes, bool bAllReadPipes, byte bPipe);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_CycleDevicePort(IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ResetDevicePort(IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_EnableGPIO(IntPtr ftHandle, UInt32 ulMask, UInt32 ulDirection);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_WriteGPIO(IntPtr ftHandle, UInt32 ulMask, UInt32 ulData);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ReadGPIO(IntPtr ftHandle, ref UInt32 pulData);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetGPIOPull(IntPtr ftHandle, UInt32 ulMask, UInt32 ulPull);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetNotificationCallback(IntPtr ftHandle, FT_NOTIFICATION_CALLBACK_DATA pCallback, IntPtr pvContext);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ClearNotificationCallback(IntPtr ftHandle);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_IsDevicePath(IntPtr ftHandle, byte[] pDevicePath);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetPipeTimeout(IntPtr ftHandle, byte bPipe, UInt32 ulTimeoutInMs);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetPipeTimeout(IntPtr ftHandle, byte bPipe, ref UInt32 pulTimeoutInMs);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_SetSuspendTimeout(IntPtr ftHandle, UInt32 ulTimeout);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_GetSuspendTimeout(IntPtr ftHandle, ref UInt32 pulTimeout);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate FT_STATUS tFT_ControlTransfer(IntPtr ftHandle, UInt64 pSetupPacket, byte[] pBuffer, UInt32 ulBytesToTransfer, ref UInt32 pulBytesTransferred);

        #endregion

        #region VARIABLES_PRIVATE

        /// <summary>
        /// Handle to the device
        /// </summary>
        private IntPtr ftHandle = IntPtr.Zero;

        /// <summary>
        /// Version numbers
        /// </summary>
        private UInt32 ulDriverVersion = 0;
        private UInt32 ulLibraryVersion = 0;
        private UInt32 ulFirmwareVersion = 0;

        /// <summary>
        /// Strings in USB String Descriptors
        /// </summary>
        private string szManufacturer = "";
        private string szProductDescription = "";
        private string szSerialNumber = "";

        /// <summary>
        /// USB Descriptors
        /// </summary>
        private bool bIsUSB3 = false;
        private UInt16 uwVID = 0;
        private UInt16 uwPID = 0;
        private FT_DEVICE_DESCRIPTOR oDeviceDescriptor = null;
        private FT_CONFIGURATION_DESCRIPTOR oConfigurationDescriptor = null;
        private List<FT_INTERFACE_DESCRIPTOR> oInterfaceDescriptors = null;
        private List<FT_PIPE_INFORMATION> oReservedPipeInformations = null;
        private List<FT_PIPE_INFORMATION> oDataPipeInformations = null;

        #endregion


        #region CONSTANTS_PUBLIC

        /// <summary>
        /// Status values for FTDI devices.
        /// </summary>
        public enum FT_STATUS
        {
            /// <summary>
            /// Status OK
            /// </summary>
            FT_OK = 0,
            /// <summary>
            /// The device handle is invalid
            /// </summary>
            FT_INVALID_HANDLE,
            /// <summary>
            /// Device not found
            /// </summary>
            FT_DEVICE_NOT_FOUND,
            /// <summary>
            /// Device is not open
            /// </summary>
            FT_DEVICE_NOT_OPENED,
            /// <summary>
            /// IO error
            /// </summary>
            FT_IO_ERROR,
            /// <summary>
            /// Insufficient resources
            /// </summary>
            FT_INSUFFICIENT_RESOURCES,
            /// <summary>
            /// A parameter was invalid
            /// </summary>
            FT_INVALID_PARAMETER,
            /// <summary>
            /// The requested baud rate is invalid
            /// </summary>
            FT_INVALID_BAUD_RATE,
            /// <summary>
            /// Device not opened for erase
            /// </summary>
            FT_DEVICE_NOT_OPENED_FOR_ERASE,
            /// <summary>
            /// Device not poened for write
            /// </summary>
            FT_DEVICE_NOT_OPENED_FOR_WRITE,
            /// <summary>
            /// Failed to write to device
            /// </summary>
            FT_FAILED_TO_WRITE_DEVICE,
            /// <summary>
            /// Failed to read the device EEPROM
            /// </summary>
            FT_EEPROM_READ_FAILED,
            /// <summary>
            /// Failed to write the device EEPROM
            /// </summary>
            FT_EEPROM_WRITE_FAILED,
            /// <summary>
            /// Failed to erase the device EEPROM
            /// </summary>
            FT_EEPROM_ERASE_FAILED,
            /// <summary>
            /// An EEPROM is not fitted to the device
            /// </summary>
            FT_EEPROM_NOT_PRESENT,
            /// <summary>
            /// Device EEPROM is blank
            /// </summary>
            FT_EEPROM_NOT_PROGRAMMED,
            /// <summary>
            /// Invalid arguments
            /// </summary>
            FT_INVALID_ARGS,

            /// <summary>
            /// Functionality not supported
            /// </summary>
            FT_NOT_SUPPORTED,
            /// <summary>
            /// No more items
            /// </summary>
            FT_NO_MORE_ITEMS,
            /// <summary>
            /// Timed out
            /// </summary>
            FT_TIMEOUT,
            /// <summary>
            /// Operation aborted
            /// </summary>
            FT_OPERATION_ABORTED,
            /// <summary>
            /// Reserved pipe
            /// </summary>
            FT_RESERVED_PIPE,
            /// <summary>
            /// Invalid control request direction
            /// </summary>
            FT_INVALID_CONTROL_REQUEST_DIRECTION,
            /// <summary>
            /// Invalid control request type
            /// </summary>
            FT_INVALID_CONTROL_REQUEST_TYPE,
            /// <summary>
            /// IO pending
            /// </summary>
            FT_IO_PENDING,
            /// <summary>
            /// IO incomplete
            /// </summary>
            FT_IO_INCOMPLETE,
            /// <summary>
            /// End of file
            /// </summary>
            FT_HANDLE_EOF,
            /// <summary>
            /// Busy
            /// </summary>
            FT_BUSY,
            /// <summary>
            /// No system resources
            /// </summary>
            FT_NO_SYSTEM_RESOURCES,
            /// <summary>
            /// Device not ready
            /// </summary>
            FT_DEVICE_LIST_NOT_READY,
            /// <summary>
            /// Device not connected
            /// </summary>
            FT_DEVICE_NOT_CONNECTED,
            /// <summary>
            /// Incorrect device path
            /// </summary>
            FT_INCORRECT_DEVICE_PATH,

            /// <summary>
            /// An other error has occurred
            /// </summary>
            FT_OTHER_ERROR
        };

        /// <summary>
        /// Device type identifiers for FT_DEVICE_INFO
        /// </summary>
        public enum FT_DEVICE
        {
            /// <summary>
            /// Unknown device
            /// </summary>
            FT_DEVICE_UNKNOWN = 3,
            /// <summary>
            /// FT600 device
            /// </summary>
            FT_DEVICE_600 = 600,
            /// <summary>
            /// FT601 device
            /// </summary>
            FT_DEVICE_601 = 601,
        };

        /// <summary>
        /// USB Pipe types
        /// </summary>
        public enum FT_PIPE_TYPE
        {
            /// <summary>
            /// USB control pipe
            /// </summary>
            CONTROL,
            /// <summary>
            /// USB isochronous pipe
            /// </summary>
            ISOCHRONOUS,
            /// <summary>
            /// USB bulk pipe
            /// </summary>
            BULK,
            /// <summary>
            /// USB interrupt pipe
            /// </summary>
            INTERRUPT
        };

        /// <summary>
        /// USB Descriptor types
        /// </summary>
        public enum FT_DESCRIPTOR_TYPE
        {
            /// <summary>
            /// USB device descriptor
            /// </summary>
            DEVICE_DESCRIPTOR = 1,
            /// <summary>
            /// USB configuration descriptor
            /// </summary>
            CONFIGURATION_DESCRIPTOR,
            /// <summary>
            /// USB string descriptor
            /// </summary>
            STRING_DESCRIPTOR,
            /// <summary>
            /// USB interface descriptor
            /// </summary>
            INTERFACE_DESCRIPTOR,
            /// <summary>
            /// USB endpoint descriptor
            /// </summary>
            ENDPOINT_DESCRIPTOR,
        };

        /// <summary>
        /// Notification types
        /// </summary>
        public enum FT_NOTIFICATION_TYPE
        {
            /// <summary>
            /// Data notification
            /// </summary>
            DATA,
            /// <summary>
            /// GPIO notification
            /// </summary>
            GPIO
        };

        /// <summary>
        /// GPIO lines
        /// </summary>
        public enum FT_GPIO
        {
            /// <summary>
            /// GPIO 0
            /// </summary>
            GPIO_0,
            /// <summary>
            /// GPIO 1
            /// </summary>
            GPIO_1,
            /// <summary>
            /// GPIO 2
            /// </summary>
            GPIO_2,
            /// <summary>
            /// GPIO 3
            /// </summary>
            GPIO_3,
            /// <summary>
            /// GPIO 4
            /// </summary>
            GPIO_4,
            /// <summary>
            /// GPIO 5
            /// </summary>
            GPIO_5,
            /// <summary>
            /// GPIO 6
            /// </summary>
            GPIO_6,
            /// <summary>
            /// GPIO 7
            /// </summary>
            GPIO_7,
        }

        /// <summary>
        /// GPIO mask
        /// </summary>
        public enum FT_GPIO_MASK
        {
            /// <summary>
            /// GPIO 0
            /// </summary>
            GPIO_0 = 1,
            /// <summary>
            /// GPIO 1
            /// </summary>
            GPIO_1 = 2,
            /// <summary>
            /// GPIO ALL
            /// </summary>
            GPIO_ALL = 3,
        }

        /// <summary>
        /// GPIO directions
        /// </summary>
        public enum FT_GPIO_DIRECTION
        {
            /// <summary>
            /// Direction IN
            /// </summary>
            IN,
            /// <summary>
            /// Direction OUT
            /// </summary>
            OUT
        }

        /// <summary>
        /// GPIO values
        /// </summary>
        public enum FT_GPIO_VALUE
        {
            /// <summary>
            /// Value LOW
            /// </summary>
            LOW,
            /// <summary>
            /// Value HIGH
            /// </summary>
            HIGH
        }

        /// <summary>
        /// Battery Charging power source types
        /// </summary>
        public enum FT_BATTERY_CHARGING_TYPE
        {
            /// <summary>
            /// Battery Charging Not Detected/OFF
            /// </summary>
            OFF,
            /// <summary>
            /// Standard Downstream Port
            /// </summary>
            SDP,
            /// <summary>
            /// Charging Downstream Port
            /// </summary>
            CDP,
            /// <summary>
            /// Dedicated Charging Port
            /// </summary>
            DCP,
        };

        /// <summary>
        /// FIFO clock speeds for FT60X chip configuration
        /// </summary>
        public enum FT_60XCONFIGURATION_FIFO_CLK
        {
            /// <summary>
            /// 100 MHz
            /// </summary>
            CLK_100_MHZ,
            /// <summary>
            /// 66 MHz
            /// </summary>
            CLK_66_MHZ,
            /// <summary>
            /// Number of options
            /// </summary>
            CLK_COUNT
        };

        /// <summary>
        /// FIFO modes for FT60X chip configuration
        /// </summary>
        public enum FT_60XCONFIGURATION_FIFO_MODE
        {
            /// <summary>
            /// 245 mode - has maximum of 1 OUT and 1 IN
            /// </summary>
            MODE_245,
            /// <summary>
            /// 600 mode - can have different channel configuration
            /// </summary>
            MODE_600,
            /// <summary>
            /// Number of options
            /// </summary>
            MODE_COUNT
        };

        /// <summary>
        /// Channel configurations for FT60X chip configuration
        /// </summary>
        public enum FT_60XCONFIGURATION_CHANNEL_CONFIG
        {
            /// <summary>
            /// 4 OUT and 4 IN pipes
            /// </summary>
            FOUR,
            /// <summary>
            /// 2 OUT and 2 IN pipes
            /// </summary>
            TWO,
            /// <summary>
            /// 1 OUT and 1 IN pipes
            /// </summary>
            ONE,
            /// <summary>
            /// 1 OUT pipe only
            /// </summary>
            ONE_OUTPIPE,
            /// <summary>
            /// 1 IN pipe only
            /// </summary>
            ONE_INPIPE,
            /// <summary>
            /// number of channel configurations
            /// </summary>
            COUNT,
        };

        /// <summary>
        /// Optional feature support for FT60X chip configuration
        /// </summary>
        public enum FT_60XCONFIGURATION_OPTIONAL_FEATURE
        {
            /// <summary>
            /// Disable all
            /// </summary>
            DISABLEALL = 0,
            /// <summary>
            /// Enable battery charging
            /// </summary>
            ENABLEBATTERYCHARGING = (0x1 << 0),
            /// <summary>
            /// Disable cancel session underrun
            /// </summary>
            DISABLECANCELSESSIONUNDERRUN = (0x1 << 1),
            /// <summary>
            /// Enable notification on channel 1 IN pipe
            /// </summary>
            ENABLENOTIFICATIONMESSAGE_INCH1 = (0x1 << 2),
            /// <summary>
            /// Enable notification on channel 2 IN pipe
            /// </summary>
            ENABLENOTIFICATIONMESSAGE_INCH2 = (0x1 << 3),
            /// <summary>
            /// Enable notification on channel 3 IN pipe
            /// </summary>
            ENABLENOTIFICATIONMESSAGE_INCH3 = (0x1 << 4),
            /// <summary>
            /// Enable notification on channel 4 IN pipe
            /// </summary>
            ENABLENOTIFICATIONMESSAGE_INCH4 = (0x1 << 5),
            /// <summary>
            /// Enable notification on all channel IN pipes
            /// </summary>
            ENABLENOTIFICATIONMESSAGE_INCHALL = (0xF << 2),
            /// <summary>
            /// Disable underrun on channel 1 IN pipe
            /// </summary>
            DISABLEUNDERRUN_INCH1 = (0x1 << 6),
            /// <summary>
            /// Disable underrun on channel 2 IN pipe
            /// </summary>
            DISABLEUNDERRUN_INCH2 = (0x1 << 7),
            /// <summary>
            /// Disable underrun on channel 3 IN pipe
            /// </summary>
            DISABLEUNDERRUN_INCH3 = (0x1 << 8),
            /// <summary>
            /// Disable underrun on channel 4 IN pipe
            /// </summary>
            DISABLEUNDERRUN_INCH4 = (0x1 << 9),
            /// <summary>
            /// Disable underrun on all channel IN pipes
            /// </summary>
            DISABLEUNDERRUN_INCHALL = (0xF << 6),
            /// <summary>
            /// Enable FIFO in suspend
            /// </summary>
            ENABLEFIFOINSUSPEND = (1 << 10),
            /// <summary>
            /// Disable chip powerdown
            /// </summary>
            DISABLECHIPPOWERDOWN = (1 << 11),
            /// <summary>
            /// Enable bitbang mode
            ENABLEBITBANGMODE = (1 << 12),
            /// <summary>
            /// Enable all
            /// </summary>
            ENABLEALL = 0xFFFF,
        };

        /// <summary>
        /// Bit flags for FlashEEPROMDetection read-only field of FT60X chip configuration
        /// </summary>
        public enum FT_60XCONFIGURATION_FLASH_ROM_BIT
        {
            /// <summary>
            /// ROM or Flash memory
            /// </summary>
            ROM,
            /// <summary>
            /// ROM or Flash memory exists
            /// </summary>
            MEMORY_NOTEXIST,
            /// <summary>
            /// Custom configuration is invalid
            /// </summary>
            CUSTOMDATA_INVALID,
            /// <summary>
            /// Custom configuration checksum is invalid
            /// </summary>
            CUSTOMDATACHKSUM_INVALID,
            /// <summary>
            /// Custom or default configuration values
            /// </summary>
            CUSTOM,
            /// <summary>
            /// GPIO is used as input values
            /// </summary>
            GPIO_INPUT,
            /// <summary>
            /// GPIO 0 is low or high
            /// </summary>
            GPIO_0,
            /// <summary>
            /// GPIO 1 is low or high
            /// </summary>
            GPIO_1,
        };

        /// <summary>
        /// Bit offsets for BatteryChargingGPIOConfig field of FT60X chip configuration
        /// </summary>
        public const UInt32 FT_60XCONFIGURATION_BATCHG_BIT_OFFSET_DEF = 0; // Bit 0 and Bit 1
        public const UInt32 FT_60XCONFIGURATION_BATCHG_BIT_OFFSET_SDP = 2; // Bit 2 and Bit 3
        public const UInt32 FT_60XCONFIGURATION_BATCHG_BIT_OFFSET_CDP = 4; // Bit 4 and Bit 5
        public const UInt32 FT_60XCONFIGURATION_BATCHG_BIT_OFFSET_DCP = 6; // Bit 6 and Bit 7
        public const UInt32 FT_60XCONFIGURATION_BATCHG_BIT_MASK = 3;       // 2 bits

        /// <summary>
        /// Default values for all fields of FT60X chip configuration
        /// </summary>
        public const UInt16 FT_60XCONFIGURATION_DEFAULT_VENDORID = 0x0403;
        public const UInt16 FT_60XCONFIGURATION_DEFAULT_PRODUCTID_600 = 0x601E;
        public const UInt16 FT_60XCONFIGURATION_DEFAULT_PRODUCTID_601 = 0x601F;
        public const string FT_60XCONFIGURATION_DEFAULT_MANUFACTURER = "FTDI";
        public const string FT_60XCONFIGURATION_DEFAULT_PRODUCTDESCRIPTION = "FTDI SuperSpeed-FIFO Bridge";
        public const string FT_60XCONFIGURATION_DEFAULT_SERIALNUMBER = "000000000001";
        public const byte FT_60XCONFIGURATION_DEFAULT_POWERATTRIBUTES = 0xE0;
        public const byte FT_60XCONFIGURATION_DEFAULT_POWERCONSUMPTION = 0x60;
        public const byte FT_60XCONFIGURATION_DEFAULT_FIFOCLOCK = (byte)FT_60XCONFIGURATION_FIFO_CLK.CLK_100_MHZ;
        public const byte FT_60XCONFIGURATION_DEFAULT_FIFOMODE = (byte)FT_60XCONFIGURATION_FIFO_MODE.MODE_600;
        public const byte FT_60XCONFIGURATION_DEFAULT_CHANNELCONFIG = (byte)FT_60XCONFIGURATION_CHANNEL_CONFIG.FOUR;
        public const UInt16 FT_60XCONFIGURATION_DEFAULT_OPTIONALFEATURE = (UInt16)FT_60XCONFIGURATION_OPTIONAL_FEATURE.DISABLEALL;
        public const byte FT_60XCONFIGURATION_DEFAULT_BATTERYCHARGING = 0xE4;
        public const byte FT_60XCONFIGURATION_DEFAULT_BATTERYCHARGING_TYPE_DCP = 0x3;
        public const byte FT_60XCONFIGURATION_DEFAULT_BATTERYCHARGING_TYPE_CDP = 0x2;
        public const byte FT_60XCONFIGURATION_DEFAULT_BATTERYCHARGING_TYPE_SDP = 0x1;
        public const byte FT_60XCONFIGURATION_DEFAULT_BATTERYCHARGING_TYPE_OFF = 0x0;
        public const byte FT_60XCONFIGURATION_DEFAULT_FLASHDETECTION = 0x0;
        public const UInt32 FT_60XCONFIGURATION_DEFAULT_MSIOCONTROL = 0x10800;
        public const UInt32 FT_60XCONFIGURATION_DEFAULT_GPIOCONTROL = 0x0;
        public const UInt32 FT_60XCONFIGURATION_SIZE = 152;

        public const byte FT_BMREQUEST_HOST_TO_DEVICE = 0;
        public const byte FT_BMREQUEST_DEVICE_TO_HOST = 1;
        public const byte FT_BMREQUEST_STANDARD = 0;
        public const byte FT_BMREQUEST_CLASS = 1;
        public const byte FT_BMREQUEST_VENDOR = 2;
        public const byte FT_BMREQUEST_TO_DEVICE = 0;
        public const byte FT_BMREQUEST_TO_INTERFACE = 1;
        public const byte FT_BMREQUEST_TO_ENDPOINT = 2;
        public const byte FT_BMREQUEST_TO_OTHER = 3;

        #endregion

        #region CLASSES_PUBLIC

        //**************************************************************************
        // DEVICE INFO
        //**************************************************************************

        /// <summary>
        /// Device information to contain information regarding D3XX devices attached to the system
        /// </summary>
        [StructLayout(LayoutKind.Explicit, Size = 68, Pack = 1, CharSet = CharSet.Ansi)]
        public class FT_DEVICE_INFO
        {
            /// <summary>
            /// Bit flags for USB 3 or USB 2 connection, etc
            /// </summary>
            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(0)]
            public UInt32 Flags;

            /// <summary>
            /// Device type as indicated by FT_DEVICE
            /// </summary>
            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(4)]
            public FT_DEVICE Type;

            /// <summary>
            /// Concatenated VID and PID
            /// </summary>
            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(8)]
            public UInt32 ID;

            /// <summary>
            /// Location identifier
            /// </summary>
            [MarshalAs(UnmanagedType.U4)]
            [FieldOffset(12)]
            public UInt32 LocId;

            /// <summary>
            /// Serial number as indicated in the USB device descriptor
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            [FieldOffset(16)]
            public byte[] SerialNumber;

            /// <summary>
            /// Product description as indicated in the USB device descriptor
            /// </summary>
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            [FieldOffset(32)]
            public byte[] Description;

            /// <summary>
            /// Handle of the device if opened
            /// </summary>
            /// [MarshalAs(UnmanagedType.AsAny)]
            [FieldOffset(64)]
            public IntPtr ftHandle;
        };


        //**************************************************************************
        // DESCRIPTORS
        //**************************************************************************

        /// <summary>
        /// USB descriptor
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public abstract class FT_DESCRIPTOR
        {
            /// <summary>
            /// Descriptor length
            /// </summary>
            public byte bLength;
            /// <summary>
            /// Descriptor type
            /// </summary>
            public byte bDescriptorType;
        }

        /// <summary>
        /// USB device descriptor
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public class FT_DEVICE_DESCRIPTOR : FT_DESCRIPTOR
        {
            /// <summary>
            /// USB Specification Number which device complies too
            /// </summary>
            public UInt16 bcdUSB;
            /// <summary>
            /// Class Code (Assigned by USB Org)
            /// </summary>
            public byte bDeviceClass;
            /// <summary>
            /// Subclass Code (Assigned by USB Org)
            /// </summary>
            public byte bDeviceSubClass;
            /// <summary>
            /// Protocol Code (Assigned by USB Org)
            /// </summary>
            public byte bDeviceProtocol;
            /// <summary>
            /// Maximum Packet Size for Zero Endpoint
            /// </summary>
            public byte bMaxPacketSize0;
            /// <summary>
            /// Vendor ID (Assigned by USB Org)
            /// </summary>
            public UInt16 idVendor;
            /// <summary>
            /// Product ID (Assigned by Manufacturer)
            /// </summary>
            public UInt16 idProduct;
            /// <summary>
            /// Device Release Number
            /// </summary>
            public UInt16 bcdDevice;
            /// <summary>
            /// Index of Manufacturer String Descriptor
            /// </summary>
            public byte iManufacturer;
            /// <summary>
            /// Index of Product String Descriptor
            /// </summary>
            public byte iProduct;
            /// <summary>
            /// Index of Serial Number String Descriptor
            /// </summary>
            public byte iSerialNumber;
            /// <summary>
            /// Number of Possible Configurations
            /// </summary>
            public byte bNumConfigurations;
        };

        /// <summary>
        /// USB configuration descriptor
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public class FT_CONFIGURATION_DESCRIPTOR : FT_DESCRIPTOR
        {
            /// <summary>
            /// Total length in bytes of data returned
            /// </summary>
            public UInt16 wTotalLength;
            /// <summary>
            /// Number of Interfaces
            /// </summary>
            public byte bNumInterfaces;
            /// <summary>
            /// Value to use as an argument to select this configuration
            /// </summary>
            public byte bConfigurationValue;
            /// <summary>
            /// Index of String Descriptor describing this configuration
            /// </summary>
            public byte iConfiguration;
            /// <summary>
            /// Self Powered, Remote Wakeup
            /// </summary>
            public byte bmAttributes;
            /// <summary>
            /// Maximum Power Consumption
            /// </summary>
            public byte MaxPower;
        };

        /// <summary>
        /// USB interface descriptor
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public class FT_INTERFACE_DESCRIPTOR : FT_DESCRIPTOR
        {
            /// <summary>
            /// Number of Interface
            /// </summary>
            public byte bInterfaceNumber;
            /// <summary>
            /// Value used to select alternative setting
            /// </summary>
            public byte bAlternateSetting;
            /// <summary>
            /// Number of Endpoints used for this interface
            /// </summary>
            public byte bNumEndpoints;
            /// <summary>
            /// Class Code (Assigned by USB Org)
            /// </summary>
            public byte bInterfaceClass;
            /// <summary>
            /// Subclass Code (Assigned by USB Org)
            /// </summary>
            public byte bInterfaceSubClass;
            /// <summary>
            /// Protocol Code (Assigned by USB Org)
            /// </summary>
            public byte bInterfaceProtocol;
            /// <summary>
            /// Index of String Descriptor Describing this interface
            /// </summary>
            public byte iInterface;
        };

        /// <summary>
        /// USB string descriptor
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Size = 516, Pack = 1, CharSet = CharSet.Unicode)]
        public class FT_STRING_DESCRIPTOR : FT_DESCRIPTOR
        {
            /// <summary>
            /// Unicode Encoded String
            /// </summary>
            [MarshalAs(UnmanagedType.LPWStr, SizeConst = 512)]
            public string szString;
        };

        /// <summary>
        /// Pipe information
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public class FT_PIPE_INFORMATION
        {
            /// <summary>
            /// Pipe type
            /// </summary>
            public FT_PIPE_TYPE PipeType;
            /// <summary>
            /// Pipe ID
            /// </summary>
            public byte PipeId;
            /// <summary>
            /// Max transfer size
            /// </summary>
            public UInt16 MaximumPacketSize;
            /// <summary>
            /// Interval for polling, for interrupt pipe only
            /// </summary>
            public byte Interval;
        };


        //**************************************************************************
        // NOTIFICATIONS
        //**************************************************************************

        /// <summary>
        /// Notification callback info for polymorphism
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public abstract class FT_NOTIFICATION_INFO
        {
        };

        /// <summary>
        /// Notification callback info for DATA
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Size = 5, Pack = 1)]
        public class FT_NOTIFICATION_INFO_DATA : FT_NOTIFICATION_INFO
        {
            /// <summary>
            /// Data length
            /// </summary>
            public UInt32 ulDataLength;
            /// <summary>
            /// Endpoint number
            /// </summary>
            public byte ucEndpointNo;
        };

        /// <summary>
        /// Notification callback info for GPIO
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Size = 8, Pack = 1)]
        public class FT_NOTIFICATION_INFO_GPIO : FT_NOTIFICATION_INFO
        {
            /// <summary>
            /// GPIO 0 status
            /// </summary>
            public UInt32 bGPIO0;
            /// <summary>
            /// GPIO 1 status
            /// </summary>
            public UInt32 bGPIO1;
        };


        //**************************************************************************
        // CONFIGURATION
        //**************************************************************************

        /// <summary>
        /// FTXXX chip configuration
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public abstract class FT_CONFIGURATION
        {
            /// <summary>
            /// Vendor ID as specified in USB device descriptor
            /// </summary>
            public UInt16 VendorID;
            /// <summary>
            /// Product ID as specified in USB device descriptor
            /// </summary>
            public UInt16 ProductID;
            /// <summary>
            /// Manufacturer referenced to in USB device descriptor
            /// </summary>
            public string Manufacturer;
            /// <summary>
            /// Product description as referenced in USB device descriptor
            /// </summary>
            public string Description;
            /// <summary>
            /// Serial number as referenced in USB device descriptor
            /// </summary>
            public string SerialNumber;
        }

        /// <summary>
        /// FT60X chip configuration
        /// </summary>
        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public class FT_60XCONFIGURATION : FT_CONFIGURATION
        {
            /// <summary>
            /// Reserved byte
            /// </summary>
            public byte Reserved;
            /// <summary>
            /// Power attribute as described in USB device descriptor
            /// </summary>
            public byte PowerAttributes;
            /// <summary>
            /// Power consumption as described in USB device descriptor
            /// </summary>
            public UInt16 PowerConsumption;
            /// <summary>
            /// Reserved byte
            /// </summary>
            public byte Reserved2;
            /// <summary>
            /// FIFO clock
            /// </summary>
            public byte FIFOClock;
            /// <summary>
            /// FIFO mode
            /// </summary>
            public byte FIFOMode;
            /// <summary>
            /// Channel configuration
            /// </summary>
            public byte ChannelConfig;
            /// <summary>
            /// Optional features
            /// </summary>
            public UInt16 OptionalFeatureSupport;
            /// <summary>
            /// Battery charing GPIO config
            /// </summary>
            public byte BatteryChargingGPIOConfig;
            /// <summary>
            /// Read-only memory detection
            /// </summary>
            public byte FlashEEPROMDetection;
            /// <summary>
            /// MSIO control
            /// </summary>
            public UInt32 MSIO_Control;
            /// <summary>
            /// GPIO control
            /// </summary>
            public UInt32 GPIO_Control;
        };

        /// <summary>
        /// FT60X control setup packet
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public class FT_SETUP_PACKET
        {
            /// <summary>
            /// Request type
            /// </summary>
            public byte RequestType;
            /// <summary>
            /// Request
            /// </summary>
            public byte Request;
            /// <summary>
            /// Value
            /// </summary>
            public UInt16 Value;
            /// <summary>
            /// Index
            /// </summary>
            public UInt16 Index;
            /// <summary>
            /// Length
            /// </summary>
            public UInt16 Length;
        };


        #endregion

        #region DELEGATES_PUBLIC

        /// <summary>
        /// Delegates declaration for the FTD3XX.DLL functions
        /// </summary>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void FT_NOTIFICATION_CALLBACK_DATA(IntPtr pvContext, FT_NOTIFICATION_TYPE eType, FT_NOTIFICATION_INFO_DATA pvInfo);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void FT_NOTIFICATION_CALLBACK_GPIO(IntPtr pvContext, FT_NOTIFICATION_TYPE eType, FT_NOTIFICATION_INFO_GPIO pvInfo);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void FT_NOTIFICATION_CALLBACK(IntPtr pvContext, FT_NOTIFICATION_TYPE eType, FT_NOTIFICATION_INFO pvInfo);

        #endregion

        #region VARIABLES_PUBLIC

        /// <summary>
        /// GUID of D3XX devices
        /// </summary>
        public static readonly Guid FT_GUID = new Guid("d1e8fe6a-ab75-4d9e-97d2-06fa22c7736c");

        #endregion

    }
}
