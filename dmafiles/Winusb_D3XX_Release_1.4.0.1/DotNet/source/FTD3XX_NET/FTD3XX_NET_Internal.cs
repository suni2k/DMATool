/*
** FTD3XX_NET_Internal.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for .NET wrapper of the Windows FTD3XX.dll API calls.
** Internal functions
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
using System.Text;
using System.Runtime.InteropServices;



namespace FTD3XXWU_NET
{
    public partial class FTDI
    {
        //**************************************************************************
        // InitializeProperties
        //**************************************************************************
        /// <summary>
        /// Initializes the class properties
        /// </summary>
        /// <returns>None</returns>
        private void InitializeProperties()
        {
            GetVersions();
            GetDescriptors();
        }


        //**************************************************************************
        // UninitializeProperties
        //**************************************************************************
        /// <summary>
        /// Uninitializes the class properties
        /// </summary>
        /// <returns>None</returns>
        private void UninitializeProperties()
        {
            ulLibraryVersion = 0;
            ulDriverVersion = 0;
            ulFirmwareVersion = 0;
            uwPID = 0;
            uwVID = 0;
            szManufacturer = "";
            szProductDescription = "";
            szSerialNumber = "";
            bIsUSB3 = false;

            oDeviceDescriptor = null;
            oConfigurationDescriptor = null;
            if (oInterfaceDescriptors != null)
            {
                oInterfaceDescriptors.Clear();
                oInterfaceDescriptors = null;
            }
            if (oReservedPipeInformations != null)
            {
                oReservedPipeInformations.Clear();
                oReservedPipeInformations = null;
            }
            if (oDataPipeInformations != null)
            {
                oDataPipeInformations.Clear();
                oDataPipeInformations = null;
            }
        }


        //**************************************************************************
        // GetVersions
        //**************************************************************************
        /// <summary>
        /// Retrieves the version-related properties
        /// </summary>
        /// <returns>None</returns>
        private void GetVersions()
        {
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            ftStatus = GetLibraryVersion(ref ulLibraryVersion);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }

            ftStatus = GetDriverVersion(ref ulDriverVersion);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }

            ftStatus = GetFirmwareVersion(ref ulFirmwareVersion);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }
        }


        //**************************************************************************
        // GetDescriptors
        //**************************************************************************
        /// <summary>
        /// Retrieves the descriptor-related properties
        /// </summary>
        /// <returns>None</returns>
        private void GetDescriptors()
        {
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;
            FTDI.FT_STRING_DESCRIPTOR StringDescriptor = null;


            ftStatus = GetDeviceDescriptor(out oDeviceDescriptor);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }

            uwVID = oDeviceDescriptor.idVendor;
            uwPID = oDeviceDescriptor.idProduct;
            bIsUSB3 = (oDeviceDescriptor.bcdUSB >= 0x0300) ? true : false;

            ftStatus = GetStringDescriptor(1, out StringDescriptor);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }
            szManufacturer = StringDescriptor.szString;

            ftStatus = GetStringDescriptor(2, out StringDescriptor);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }
            szProductDescription = StringDescriptor.szString;

            ftStatus = GetStringDescriptor(3, out StringDescriptor);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }
            szSerialNumber = StringDescriptor.szString;

            ftStatus = GetConfigurationDescriptor(out oConfigurationDescriptor);
            if (ftStatus != FT_STATUS.FT_OK)
            {
                return;
            }

            oInterfaceDescriptors = new List<FT_INTERFACE_DESCRIPTOR>();
            oReservedPipeInformations = new List<FT_PIPE_INFORMATION>();
            oDataPipeInformations = new List<FT_PIPE_INFORMATION>();

            for (byte i = 0; i < oConfigurationDescriptor.bNumInterfaces; i++)
            {
                FT_INTERFACE_DESCRIPTOR InterfaceDescriptor;
                ftStatus = GetInterfaceDescriptor(i, out InterfaceDescriptor);
                if (ftStatus != FT_STATUS.FT_OK)
                {
                    return;
                }

                oInterfaceDescriptors.Add(InterfaceDescriptor);

                for (byte j = 0; j < InterfaceDescriptor.bNumEndpoints; j++)
                {
                    FT_PIPE_INFORMATION PipeInformation;
                    ftStatus = GetPipeInformation(i, j, out PipeInformation);
                    if (ftStatus != FT_STATUS.FT_OK)
                    {
                        return;
                    }

                    if (i == 0)
                    {
                        oReservedPipeInformations.Add(PipeInformation);
                    }
                    else
                    {
                        oDataPipeInformations.Add(PipeInformation);
                    }
                }
            }
        }


        //**************************************************************************
        // GetFirmwareVersion
        //**************************************************************************
        /// <summary>
        /// Retrieves the firmware version of the device.  
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetFirmwareVersion in FTD3XX.DLL</returns>
        /// <param name="pulVersion">A reference to an unsigned int that will contain the verion of the firmware.</param>
        private FT_STATUS GetFirmwareVersion(ref UInt32 pulVersion)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetFirmwareVersion != IntPtr.Zero)
            {
                var FT_GetFirmwareVersion = (tFT_GetFirmwareVersion)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetFirmwareVersion, typeof(tFT_GetFirmwareVersion));

                // Call FT_GetFirmwareVersion
                ftStatus = FT_GetFirmwareVersion(ftHandle, ref pulVersion);
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetDriverVersion
        //**************************************************************************
        /// <summary>
        /// Retrieves the driver version.  
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetDriverVersion in FTD3XX.DLL</returns>
        /// <param name="pulVersion">A reference to an unsigned int that will contain the verion of the kernel driver.</param>
        private FT_STATUS GetDriverVersion(ref UInt32 pulVersion)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetDriverVersion != IntPtr.Zero)
            {
                var FT_GetDriverVersion = (tFT_GetDriverVersion)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetDriverVersion, typeof(tFT_GetDriverVersion));

                // Call FT_GetDriverVersion
                ftStatus = FT_GetDriverVersion(ftHandle, ref pulVersion);
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetLibraryVersion
        //**************************************************************************
        /// <summary>
        /// Retrieves the library version.  
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetLibraryVersion in FTD3XX.DLL</returns>
        /// <param name="pulVersion">A reference to an unsigned int that will contain the verion of the user library.</param>
        private FT_STATUS GetLibraryVersion(ref UInt32 pulVersion)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetLibraryVersion != IntPtr.Zero)
            {
                var FT_GetLibraryVersion = (tFT_GetLibraryVersion)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetLibraryVersion, typeof(tFT_GetLibraryVersion));

                // Call FT_GetLibraryVersion
                ftStatus = FT_GetLibraryVersion(ref pulVersion);
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetDeviceDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the device descriptor
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetDeviceDescriptor in FTD3XX.DLL</returns>
        /// <param name="pDescriptor">A reference to a FT_DEVICE_DESCRIPTOR class that will contain the USB device descriptor.</param>
        private FT_STATUS GetDeviceDescriptor(out FT_DEVICE_DESCRIPTOR pDescriptor)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            pDescriptor = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetDeviceDescriptor != IntPtr.Zero)
            {
                var FT_GetDeviceDescriptor = (tFT_GetDeviceDescriptor)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetDeviceDescriptor, typeof(tFT_GetDeviceDescriptor));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] ByteArray = new byte[Marshal.SizeOf(typeof(FT_DEVICE_DESCRIPTOR))];

                    // Call FT_GetDeviceDescriptor
                    ftStatus = FT_GetDeviceDescriptor(ftHandle, ByteArray);

                    if (ftStatus == FT_STATUS.FT_OK)
                    {
                        pDescriptor = Deserialize<FT_DEVICE_DESCRIPTOR>(ByteArray);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetConfigurationDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the configuration descriptor
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetConfigurationDescriptor in FTD3XX.DLL</returns>
        /// <param name="pDescriptor">A reference to a FT_CONFIGURATION_DESCRIPTOR class that will contain the USB configuration descriptor.</param>
        private FT_STATUS GetConfigurationDescriptor(out FT_CONFIGURATION_DESCRIPTOR pDescriptor)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            pDescriptor = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetConfigurationDescriptor != IntPtr.Zero)
            {
                var FT_GetConfigurationDescriptor = (tFT_GetConfigurationDescriptor)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetConfigurationDescriptor, typeof(tFT_GetConfigurationDescriptor));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] ByteArray = new byte[Marshal.SizeOf(typeof(FT_CONFIGURATION_DESCRIPTOR))];

                    // Call FT_GetConfigurationDescriptor
                    ftStatus = FT_GetConfigurationDescriptor(ftHandle, ByteArray);

                    if (ftStatus == FT_STATUS.FT_OK)
                    {
                        pDescriptor = Deserialize<FT_CONFIGURATION_DESCRIPTOR>(ByteArray);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetInterfaceDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the interface descriptor
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetInterfaceDescriptor in FTD3XX.DLL</returns>
        /// <param name="bInterfaceIndex">Index of the interface descriptor to be queried.</param>
        /// <param name="pDescriptor">A reference to a FT_INTERFACE_DESCRIPTOR class that will contain the USB interface descriptor.</param>
        private FT_STATUS GetInterfaceDescriptor(byte bInterfaceIndex, out FT_INTERFACE_DESCRIPTOR pDescriptor)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            pDescriptor = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetInterfaceDescriptor != IntPtr.Zero)
            {
                var FT_GetInterfaceDescriptor = (tFT_GetInterfaceDescriptor)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetInterfaceDescriptor, typeof(tFT_GetInterfaceDescriptor));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] ByteArray = new byte[Marshal.SizeOf(typeof(FT_INTERFACE_DESCRIPTOR))];

                    // Call FT_GetInterfaceDescriptor
                    ftStatus = FT_GetInterfaceDescriptor(ftHandle, bInterfaceIndex, ByteArray);

                    if (ftStatus == FT_STATUS.FT_OK)
                    {
                        pDescriptor = Deserialize<FT_INTERFACE_DESCRIPTOR>(ByteArray);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetStringDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the string descriptor
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetStringDescriptor in FTD3XX.DLL</returns>
        /// <param name="bInterfaceIndex">Index of the string descriptor to be queried.</param>
        /// <param name="pDescriptor">A reference to a FT_STRING_DESCRIPTOR class that will contain the USB string descriptor.</param>
        private FT_STATUS GetStringDescriptor(byte bStringIndex, out FT_STRING_DESCRIPTOR pDescriptor)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            pDescriptor = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetStringDescriptor != IntPtr.Zero)
            {
                var FT_GetStringDescriptor = (tFT_GetStringDescriptor)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetStringDescriptor, typeof(tFT_GetStringDescriptor));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] ByteArray = new byte[Marshal.SizeOf(typeof(FT_STRING_DESCRIPTOR))];

                    // Call FT_GetStringDescriptor
                    ftStatus = FT_GetStringDescriptor(ftHandle, bStringIndex, ByteArray);

                    if (ftStatus == FT_STATUS.FT_OK)
                    {
                        pDescriptor = new FT_STRING_DESCRIPTOR();
                        pDescriptor.bLength = ByteArray[0];
                        pDescriptor.bDescriptorType = ByteArray[1];

                        byte[] szString = new byte[pDescriptor.bLength - 2];
                        Array.Copy(ByteArray, 2, szString, 0, pDescriptor.bLength - 2);
                        pDescriptor.szString = System.Text.Encoding.Unicode.GetString(szString);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // GetPipeInformation
        //**************************************************************************
        /// <summary>
        /// Gets the pipe information
        /// </summary>
        /// <returns>FT_STATUS value from FT_GetPipeInformation in FTD3XX.DLL</returns>
        /// <param name="bInterfaceIndex">Index of the interface to be queried.</param>
        /// <param name="bPipeIndex">Index of the pipe to be queried.</param>
        /// <param name="pDescriptor">A reference to a FT_PIPE_INFORMATION class that will contain the pipe information.</param>
        private FT_STATUS GetPipeInformation(byte bInterfaceIndex, byte bPipeIndex, out FT_PIPE_INFORMATION pDescriptor)
        {
            // Initialise ftStatus to something other than FT_OK
            FT_STATUS ftStatus = FT_STATUS.FT_OTHER_ERROR;


            pDescriptor = null;

            // If the DLL hasn't been loaded, just return here
            if (hFTD3XXDLL == IntPtr.Zero)
                return ftStatus;

            // Check for our required function pointers being set up
            if (pFT_GetPipeInformation != IntPtr.Zero)
            {
                var FT_GetPipeInformation = (tFT_GetPipeInformation)Marshal.GetDelegateForFunctionPointer(
                    pFT_GetPipeInformation, typeof(tFT_GetPipeInformation));

                if (ftHandle != IntPtr.Zero)
                {
                    byte[] ByteArray = new byte[Marshal.SizeOf(typeof(FT_PIPE_INFORMATION))];

                    // Call FT_GetPipeInformation
                    ftStatus = FT_GetPipeInformation(ftHandle, bInterfaceIndex, bPipeIndex, ByteArray);

                    if (ftStatus == FT_STATUS.FT_OK)
                    {
                        pDescriptor = Deserialize<FT_PIPE_INFORMATION>(ByteArray);
                    }
                }
            }

            return ftStatus;
        }


        //**************************************************************************
        // Serialize
        //**************************************************************************
        /// <summary>
        /// Method to convert object structure to byte array
        /// </summary>
        /// <returns>An array of byte.</returns>
        /// <param name="obj">The structure/class object to be converted to an array of bytes.</param>
        private byte[] Serialize(object obj)
        {
            int size = Marshal.SizeOf(obj);
            byte[] arr = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(obj, ptr, true);
            Marshal.Copy(ptr, arr, 0, size);
            Marshal.FreeHGlobal(ptr);

            return arr;
        }


        //**************************************************************************
        // Deserialize
        //**************************************************************************
        /// <summary>
        /// Method to convert byte array to object structure
        /// </summary>
        /// <returns>An generic structure/class object</returns>
        /// <param name="obj">The array of bytes to be converted to a structure/class object.</param>
        private T Deserialize<T>(byte[] arr)
        {
            T obj = default(T);
            int size = Marshal.SizeOf(typeof(T));
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(arr, 0, ptr, size);
            obj = (T)Marshal.PtrToStructure(ptr, typeof(T));
            Marshal.FreeHGlobal(ptr);

            return obj;
        }
    }
}
