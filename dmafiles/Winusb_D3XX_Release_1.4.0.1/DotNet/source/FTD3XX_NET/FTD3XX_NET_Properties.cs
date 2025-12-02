/*
** FTD3XX_NET_Properties.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for .NET wrapper of the Windows FTD3XX.dll API calls.
** Public properties
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



namespace FTD3XXWU_NET
{
    public partial class FTDI
    {
        //**************************************************************************
        // IsOpen
        //**************************************************************************
        /// <summary>
        /// Gets the open status of the device.
        /// </summary>
        public bool IsOpen
        {
            get
            {
                if (ftHandle == IntPtr.Zero)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }


        //**************************************************************************
        // IsUSB3
        //**************************************************************************
        /// <summary>
        /// Gets the USB3 status of the device.
        /// </summary>
        public bool IsUSB3
        {
            get
            {
                return bIsUSB3;
            }
        }


        //**************************************************************************
        // VendorID
        //**************************************************************************
        /// <summary>
        /// Gets the VendorID (VID).
        /// </summary>
        public UInt16 VendorID
        {
            get 
            { 
                return uwVID; 
            }
        }


        //**************************************************************************
        // ProductID
        //**************************************************************************
        /// <summary>
        /// Gets the ProductID (PID).
        /// </summary>
        public UInt16 ProductID
        {
            get 
            { 
                return uwPID; 
            }
        }


        //**************************************************************************
        // Manufacturer
        //**************************************************************************
        /// <summary>
        /// Gets the Manufacturer string in the USB Device Descriptor.
        /// </summary>
        public string Manufacturer
        {
            get 
            { 
                return szManufacturer; 
            }
        }


        //**************************************************************************
        // ProductDescription
        //**************************************************************************
        /// <summary>
        /// Gets the ProductDescription string in the USB Device Descriptor.
        /// </summary>
        public string ProductDescription
        {
            get 
            { 
                return szProductDescription; 
            }
        }


        //**************************************************************************
        // SerialNumber
        //**************************************************************************
        /// <summary>
        /// Gets the SerialNumber string in the USB Device Descriptor.
        /// </summary>
        public string SerialNumber
        {
            get 
            { 
                return szSerialNumber; 
            }
        }


        //**************************************************************************
        // DriverVersion
        //**************************************************************************
        /// <summary>
        /// Gets the version of the kernel driver.
        /// </summary>
        public UInt32 DriverVersion
        {
            get 
            { 
                return ulDriverVersion; 
            }
        }


        //**************************************************************************
        // LibraryVersion
        //**************************************************************************
        /// <summary>
        /// Gets the version of the user library.
        /// </summary>
        public UInt32 LibraryVersion
        {
            get 
            { 
                return ulLibraryVersion; 
            }
        }


        //**************************************************************************
        // FirmwareVersion
        //**************************************************************************
        /// <summary>
        /// Gets the version of the device firmware.
        /// </summary>
        public UInt32 FirmwareVersion
        {
            get
            {
                return ulFirmwareVersion;
            }
        }


        //**************************************************************************
        // DeviceDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the USB Device Descriptor.
        /// </summary>
        public FT_DEVICE_DESCRIPTOR DeviceDescriptor
        {
            get 
            { 
                return oDeviceDescriptor; 
            }
        }


        //**************************************************************************
        // ConfigurationDescriptor
        //**************************************************************************
        /// <summary>
        /// Gets the USB Configuration Descriptor.
        /// </summary>
        public FT_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
        {
            get 
            { 
                return oConfigurationDescriptor; 
            }
        }


        //**************************************************************************
        // InterfaceDescriptors
        //**************************************************************************
        /// <summary>
        /// Gets the list of USB Interface Descriptor.
        /// </summary>
        public List<FT_INTERFACE_DESCRIPTOR> InterfaceDescriptors
        {
            get 
            { 
                return oInterfaceDescriptors; 
            }
        }


        //**************************************************************************
        // ReservedPipeInformation
        //**************************************************************************
        /// <summary>
        /// Gets the list of Pipe Information for all the reserved pipes in Interface 0.
        /// </summary>
        public List<FT_PIPE_INFORMATION> ReservedPipeInformation
        {
            get 
            { 
                return oReservedPipeInformations; 
            }
        }


        //**************************************************************************
        // DataPipeInformation
        //**************************************************************************
        /// <summary>
        /// Gets the list of Pipe Information for all data pipes in Interface 1.
        /// </summary>
        public List<FT_PIPE_INFORMATION> DataPipeInformation
        {
            get 
            { 
                return oDataPipeInformations; 
            }
        }
    }
}
