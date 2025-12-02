/*
** APP_TaskManager.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for Demo Application.
**
** Author: FTDI
** Project: C# Data Loopback Demo Application
** Module: Task Manager thread implementation
**
** History:
**  1.0.0.0	- Initial version
**
*/

using System;
using System.Linq;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Threading;
using System.Threading;
using DemoUtility;
using FTD3XXWU_NET;



namespace DemoApp
{
    public class TaskManager
    {
        /// <summary>
        /// Entry point of the task manager thread
        /// </summary>
        public void Run()
        {
            bool bResult = false;


            LogFile.Log("TaskManager begin...");

            objectWriter = new WriterThread[ulMaxChannels];
            threadWriter = new Thread[ulMaxChannels];
            objectReader = new ReaderThread[ulMaxChannels];
            threadReader = new Thread[ulMaxChannels];
            
            while (Application.Current != null)
            {
                if (queueTasks.Count == 0)
                {
                    Thread.Sleep(32);
                    continue;
                }

                // Dequeue a task
                Task task = RemoveTask();
                if (task == null)
                {
                    continue;
                }

                // Process a task
                switch (task.Param.eType)
                {
                    case Task.TaskType.Detect:
                        {
                            UInt32 ulNumDevicesConnected;

                            bResult = ProcessDetect(task.Param.oTransferParams.fxnCallbackDebug, out ulNumDevicesConnected);

                            task.SetResult(bResult, ulNumDevicesConnected);
                            CallUICallback(task, bResult);
                            break;
                        }

                    case Task.TaskType.Open:
                        {
                            bResult = ProcessOpen(task.Param.oTransferParams.fxnCallbackDebug, 
                                task.Param.bOpenBy, task.Param.OpenByString);

                            if (bResult)
                            {
                                byte bNumWritePipes = 0, bNumReadPipes = 0;
                                bool bTransferResult = false;

                                GetNumPipes(ref bNumWritePipes, ref bNumReadPipes);
								
								// Loopback requires atleast 1 write and 1 read pipe enabled
                                if (bNumWritePipes == 0 || bNumReadPipes == 0)
                                {
                                    ProcessClose();
                                    bResult = false;
                                }
                                else
                                {
                                    bTransferResult = DoSimpleTransfer();
                                    if (!bTransferResult)
                                    {
                                        ProcessClose();
                                        bResult = false;
                                    }
                                }

                                task.SetResult(bResult, bNumWritePipes, bNumReadPipes, bTransferResult);
                            }

                            CallUICallback(task, bResult);
                            break;
                        }

                    case Task.TaskType.Close:
                        {
                            bResult = ProcessClose();

                            CallUICallback(task, bResult);
                            break;
                        }

                    case Task.TaskType.Start:
                        {
                            bResult = ProcessTransfer(task.Param.oTransferParams);

                            // UI Callback will be called on the completion routine
                            break;
                        }

                    case Task.TaskType.Stop:
                        {
                            bResult = ProcessAbortTransfer(task.Param.oTransferParams);

                            CallUICallback(task, bResult);
                            break;
                        }

                    case Task.TaskType.TestMode:
                        {
                            //bResult = ProcessTestMode(task.Param.oTransferParams,
                            //    task.Param.bOpenBy, task.Param.OpenByString);

                            CallUICallback(task, bResult);
                            break;
                        }
                }
            }

            LogFile.Log("TaskManager end...\r\n");
        }

        /// <summary>
        /// Add task to the queue of the task manager
        /// </summary>
        public void AddTask(Task newTask)
        {
            LogFile.Log("AddTask...{0} TaskCount={1}", newTask.Type, queueTasks.Count);

            if (newTask.Type == Task.TaskType.Close)
            {
                while (queueTasks.Count > 0)
                {
                    queueTasks.Dequeue();
                }

                LogFile.Log("TaskCount={0}", queueTasks.Count);
            }
            queueTasks.Enqueue(newTask);
        }

        /// <summary>
        /// Internal function to remove task from the queue of the task manager
        /// </summary>
        private Task RemoveTask()
        {
            Task taskItem = queueTasks.Dequeue();

            LogFile.Log("RemoveTask...{0} QueueCount{1}", taskItem.Type, queueTasks.Count);

            return taskItem;
        }

        /// <summary>
        /// Internal function for task Detect 
        /// </summary>
        private bool ProcessDetect(DelegateCallbackTextBoxDebug fxnCallbackDebug, out UInt32 ulNumDevicesConnected)
        {
            FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OTHER_ERROR;


            LogFile.Log("ProcessDetect...");

            ftStatus = d3xxDevice.CreateDeviceInfoList(out ulNumDevicesConnected);
            if (ftStatus != FTDI.FT_STATUS.FT_OK)
            {
                return false;
            }

            if (ulNumDevicesConnected == 0)
            {
                return false;
            }

            List<FTDI.FT_DEVICE_INFO> DeviceInfoList;
            ftStatus = d3xxDevice.GetDeviceInfoList(out DeviceInfoList);
            if (ftStatus != FTDI.FT_STATUS.FT_OK)
            {
                return false;
            }

            LogInfo(fxnCallbackDebug, "List of Connected Devices!\n\n");

            int i = 0;
            foreach (var DeviceInfo in DeviceInfoList)
            {
                string Description = d3xxDevice.GetDescription(DeviceInfo);
                string SerialNumber = d3xxDevice.GetSerialNumber(DeviceInfo);

                LogInfo(fxnCallbackDebug, String.Format("Device[{0:d}]\n", i++));
                LogInfo(fxnCallbackDebug, String.Format(
                    "\tFlags: 0x{0:X} [{1}] | Type: {2:d} | ID: 0x{3:X8} | ftHandle: 0x{4:X}\n",
                    DeviceInfo.Flags, DeviceInfo.Flags == 0x4 ? "USB3" : "USB2", 
                    DeviceInfo.Type, DeviceInfo.ID, DeviceInfo.ftHandle));
                LogInfo(fxnCallbackDebug, String.Format("\tDescription: {0}\n", Description));
                LogInfo(fxnCallbackDebug, String.Format("\tSerialNumber: {0}\n", SerialNumber));
            }

            return true;
        }

        /// <summary>
        /// Internal function for task Open
        /// </summary>
        private bool ProcessOpen(DelegateCallbackTextBoxDebug fxnCallbackDebug, byte bOpenBy, string OpenByString)
        {
            FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OTHER_ERROR;
            bool bResult = false;
            string szMsg;


            LogFile.Log("ProcessOpen...");

            if (d3xxDevice.IsOpen)
            {
                return false;
            }

            switch ((Controls.OpenBy.Type)bOpenBy)
            {
                case Controls.OpenBy.Type.Description:
                    {
                        szMsg = String.Format("\nDevice with Description [" + OpenByString + "] opened ");
                        ftStatus = d3xxDevice.OpenByDescription(OpenByString);
                        if (ftStatus != FTDI.FT_STATUS.FT_OK)
                        {
                            Thread.Sleep(1000);
                            ftStatus = d3xxDevice.OpenByDescription(OpenByString);
                        }
                        break;
                    }
                case Controls.OpenBy.Type.SerialNumber:
                    {
                        szMsg = String.Format("\nDevice with Serial Number [" + OpenByString + "] opened ");
                        ftStatus = d3xxDevice.OpenBySerialNumber(OpenByString);
                        if (ftStatus != FTDI.FT_STATUS.FT_OK)
                        {
                            Thread.Sleep(1000);
                            ftStatus = d3xxDevice.OpenBySerialNumber(OpenByString);
                        }
                        break;
                    }
                case Controls.OpenBy.Type.Index: // fall-through
                default:
                    {
                        szMsg = String.Format("\nDevice at Index [" + OpenByString + "] opened ");
                        ftStatus = d3xxDevice.OpenByIndex(UInt32.Parse(OpenByString));
                        if (ftStatus != FTDI.FT_STATUS.FT_OK)
                        {
                            Thread.Sleep(1000);
                            ftStatus = d3xxDevice.OpenByIndex(UInt32.Parse(OpenByString));
                        }
                        break;
                    }
            }

            if (ftStatus == FTDI.FT_STATUS.FT_OK)
            {
                bResult = true;
                LogInfo(fxnCallbackDebug, String.Format(szMsg + "successfully!\n"));

                // Disable suspend
                d3xxDevice.SetSuspendTimeout(0);
                UInt32 ulSuspendTimeout = 0;
                d3xxDevice.GetSuspendTimeout(ref ulSuspendTimeout);

                //// Disable read pipe timeout
                //foreach (var Pipe in d3xxDevice.DataPipeInformation)
                //{
                //    if (Pipe.PipeId > 0x80)
                //    {
                //        d3xxDevice.SetReadPipeTimeout(Pipe.PipeId, 0);
                //        UInt32 ulTimeout = 0;
                //        d3xxDevice.GetReadPipeTimeout(Pipe.PipeId, ref ulTimeout);
                //    }
                //}

                // Check if USB 2
                if (!d3xxDevice.IsUSB3)
                {
                    LogInfo(fxnCallbackDebug, 
                        "Warning: Device is connected using USB 2 cable and/or through a USB 2 host controller!\n\n");
                }

                // Display version numbers
                LogInfo(fxnCallbackDebug, String.Format(
                    "Device Firmware Version: {0:X4}!\n", d3xxDevice.FirmwareVersion));
                LogInfo(fxnCallbackDebug, String.Format(
                    "D3XX Driver Version: {0:X8} | D3XX Library Version: {1:X8}!\n\n", 
                    d3xxDevice.DriverVersion, d3xxDevice.LibraryVersion));

                // Display descriptors and chip configuration
                LogDeviceInformation();

                // Get number of channels
                byte bNumChannels = (byte)((d3xxDevice.DataPipeInformation.Count + 1) / 2);
                objectLoopback = new Loopback(bNumChannels);
            }

            return bResult;
        }

        /// <summary>
        /// Internal function for logging chip configuration and device descriptor
        /// </summary>
        private void LogDeviceInformation()
        {
            var desc = d3xxDevice.DeviceDescriptor;
            LogFile.Log("\tDEVICE DESCRIPTOR");
            LogFile.Log("\tbLength                  : 0x{0:X2}   ({1:d})", desc.bLength, desc.bLength);
            LogFile.Log("\tbDescriptorType          : 0x{0:X2}", desc.bDescriptorType);
            LogFile.Log("\tbcdUSB                   : 0x{0:X4}   ({1})", desc.bcdUSB, desc.bcdUSB >= 0x0300 ? "USB 3" : "USB 2");
            LogFile.Log("\tbDeviceClass             : 0x{0:X2}", desc.bDeviceClass);
            LogFile.Log("\tbDeviceSubClass          : 0x{0:X2}", desc.bDeviceSubClass);
            LogFile.Log("\tbDeviceProtocol          : 0x{0:X2}", desc.bDeviceProtocol);
            LogFile.Log("\tbMaxPacketSize0          : 0x{0:X2}   ({1:d})", desc.bMaxPacketSize0, desc.bMaxPacketSize0);
            LogFile.Log("\tidVendor                 : 0x{0:X4}", desc.idVendor);
            LogFile.Log("\tidProduct                : 0x{0:X4}", desc.idProduct);
            LogFile.Log("\tbcdDevice                : 0x{0:X4}", desc.bcdDevice);
            LogFile.Log("\tiManufacturer            : 0x{0:X2}   ({1})", desc.iManufacturer, d3xxDevice.Manufacturer);
            LogFile.Log("\tiProduct                 : 0x{0:X2}   ({1})", desc.iProduct, d3xxDevice.ProductDescription);
            LogFile.Log("\tiSerialNumber            : 0x{0:X2}   ({1})", desc.iSerialNumber, d3xxDevice.SerialNumber);
            LogFile.Log("\tbNumConfigurations       : 0x{0:X2}", desc.bNumConfigurations);

            var conf = new FTDI.FT_60XCONFIGURATION();
            if (d3xxDevice.GetChipConfiguration(conf) != FTDI.FT_STATUS.FT_OK)
            {
                return;
            }

            LogFile.Log("\tCHIP CONFIGURATION");
            LogFile.Log("\tVendorID                 : 0x{0:X4}", conf.VendorID);
            LogFile.Log("\tProductID                : 0x{0:X4}", conf.ProductID);
            LogFile.Log("\tManufacturer             : " + conf.Manufacturer);
            LogFile.Log("\tDescription              : " + conf.Description);
            LogFile.Log("\tSerialNumber             : " + conf.SerialNumber);
            LogFile.Log("\tPowerAttributes          : 0x{0:X2}", conf.PowerAttributes);
            LogFile.Log("\tPowerConsumption         : 0x{0:X4}", conf.PowerConsumption);
            LogFile.Log("\tFIFOMode                 : 0x{0:X2}", conf.FIFOMode);
            LogFile.Log("\tChannelConfig            : 0x{0:X2}", conf.ChannelConfig);
            LogFile.Log("\tOptionalFeatureSupport   : 0x{0:X4}", conf.OptionalFeatureSupport);
            LogFile.Log("\tFlashEEPROMDetection     : 0x{0:X2}", conf.FlashEEPROMDetection);
        }

        /// <summary>
        /// Internal function for test if transfer is working
        /// </summary>
        private bool DoSimpleTransfer()
        {
            FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OK;
            UInt32 transferBytes = 4096;
            byte[] writeBytes = new byte[transferBytes];
            byte[] readBytes = new byte[transferBytes];
            bool bLoopbackResult = false;


            LogFile.Log("DoSimpleTransfer begin...");

            var Event = new AutoResetEvent(false);

            System.Threading.Tasks.Task.Run(delegate
            {
                for (UInt32 j = 0; j < writeBytes.Length; j++)
                {
                    writeBytes[j] = 0xAA;
                }

                Array.Clear(readBytes, 0, readBytes.Length);

                UInt32 bytesWritten = 0;
                ftStatus = d3xxDevice.WritePipe(0x02, writeBytes, (UInt32)writeBytes.Length, ref bytesWritten);
                if (ftStatus != FTDI.FT_STATUS.FT_OK)
                {
                    LogFile.Log("DoSimpleTransfer WritePipe failed! ftStatus={0}", ftStatus);
                    Event.Set();
                    return;
                }
                if (bytesWritten != (UInt32)writeBytes.Length)
                {
                    LogFile.Log("DoSimpleTransfer bytesWritten = {0}", bytesWritten);
                    Event.Set();
                    return;
                }

                UInt32 bytesRead = 0;
                ftStatus = d3xxDevice.ReadPipe(0x82, readBytes, (UInt32)readBytes.Length, ref bytesRead);
                if (ftStatus != FTDI.FT_STATUS.FT_OK)
                {
                    LogFile.Log("DoSimpleTransfer ReadPipe failed! ftStatus={0}", ftStatus);
                    Event.Set();
                    return;
                }
                if (bytesRead != (UInt32)readBytes.Length)
                {
                    LogFile.Log("DoSimpleTransfer bytesRead = {0}", bytesRead);
                    Event.Set();
                    return;
                }

                bool same = writeBytes.SequenceEqual(readBytes);
                if (same == false)
                {
                    LogFile.Log("DoSimpleTransfer SequenceEqual failed!");
                    Event.Set();
                    return;
                }

                bLoopbackResult = true;
                Event.Set();
            });

            bool bRet = Event.WaitOne(3000);
            if (!bRet)
            {
                LogFile.Log("DoSimpleTransfer WaitOne timedout!");
                d3xxDevice.AbortPipe(0x02);
                d3xxDevice.AbortPipe(0x82);
            }

            LogFile.Log("DoSimpleTransfer end... bLoopbackResult = {0}", bLoopbackResult);

            return bRet;
        }

        /// <summary>
        /// Internal function for task Close
        /// </summary>
        private bool ProcessClose()
        {
            LogFile.Log("ProcessClose...");

            if (d3xxDevice.IsOpen)
            {
                d3xxDevice.Close();
            }

            //if (queueTasks != null)
            //{
            //    if (queueTasks.Count > 0)
            //    {
            //        queueTasks.Clear();
            //    }
            //}

            if (objectLoopback != null)
            {
                objectLoopback = null;
            }

            return true;
        }

        /// <summary>
        /// Internal function for task Transfer
        /// </summary>
        private bool ProcessTransfer(WorkerThread.TransferParams oParams)
        {
            LogFile.Log("ProcessTransfer[0x{0:X2}]...", oParams.bPipe);

            if (!d3xxDevice.IsOpen)
            {
                return false;
            }

            if (objectLoopback == null)
            {
                return false;
            }

            objectLoopback.Begin(oParams.bChannelIndex, oParams.bStress);
            oParams.fileDirectory = objectLoopback.GetFileDirectory();
            oParams.fileName = objectLoopback.GetFileName(oParams.bChannelIndex, oParams.IsWritePipe());
            oParams.fxnCallbackCompleted = ProcessTransferCompletion;

            if (oParams.IsWritePipe())
            {
                objectWriter[oParams.bChannelIndex] = new WriterThread(d3xxDevice, ref oParams);
                threadWriter[oParams.bChannelIndex] = new Thread(new ThreadStart(objectWriter[oParams.bChannelIndex].Run));
                threadWriter[oParams.bChannelIndex].Start();
            }
            else
            {
                objectReader[oParams.bChannelIndex] = new ReaderThread(d3xxDevice, ref oParams);
                threadReader[oParams.bChannelIndex] = new Thread(new ThreadStart(objectReader[oParams.bChannelIndex].Run));
                threadReader[oParams.bChannelIndex].Start();
            }

            return true;
        }

        /// <summary>
        /// Internal function for task Transfer completion routine
        /// </summary>
        private void ProcessTransferCompletion(Task.TaskResult oResult)
        {
            LogFile.Log("ProcessTransferCompletion[0x{0:X2}]...", oResult.bPipe);

            if (objectLoopback == null)
            {
				return;
			}
			
            if (oResult.bResult)
            {
                objectLoopback.AddBytesTransferred(oResult.bChannelIndex, oResult.IsWritePipe(), oResult.ulBytesTransferred);

                if (objectLoopback.IsLoopbackComplete(oResult.bChannelIndex, oResult.bPipe))
                {
                    oResult.bLoopbackCompleted = true;

                    if (objectLoopback.VerifyFiles(oResult.bChannelIndex))
                    {
                        oResult.bTransferResult = true;
                        objectLoopback.End(oResult.bChannelIndex, false);
                    }
                    else
                    {
                        oResult.bTransferResult = false;
                        objectLoopback.End(oResult.bChannelIndex, true);
                    }
                }
                else
                {
                    oResult.bLoopbackCompleted = false;
                }
            }

            if (Application.Current != null)
            {
                Application.Current.Dispatcher.Invoke(
                    DispatcherPriority.ApplicationIdle,
                    new DelegateCallbackTask(UICallback),
                    oResult
                    );
            }
        }

        /// <summary>
        /// Internal function for task abort Transfer
        /// </summary>
        private bool ProcessAbortTransfer(WorkerThread.TransferParams oParams)
        {
            LogFile.Log("ProcessAbortTransfer[0x{0:X2}]...", oParams.bPipe);

            if (oParams.IsWritePipe())
            {
                if (objectWriter[oParams.bChannelIndex] != null)
                {
                    objectWriter[oParams.bChannelIndex].Stop();
                }
            }
            else
            {
                if (objectReader[oParams.bChannelIndex] != null)
                {
                    objectReader[oParams.bChannelIndex].Stop();
                }
            }

            return true;
        }

        /// <summary>
        /// Internal function for retreiving the number of pipes
        /// </summary>
        private void GetNumPipes(ref byte bNumWritePipes, ref byte bNumReadPipes)
        {
            if (!d3xxDevice.IsOpen)
            {
                return;
            }

            foreach (var Pipe in d3xxDevice.DataPipeInformation)
            {
                if (Pipe.PipeId < 0x80)
                {
                    bNumWritePipes++;
                }
                else
                {
                    bNumReadPipes++;
                }
            }
        }

        /// <summary>
        /// Internal function for updating of the textbox debugging in UI
        /// </summary>
        private void LogInfo(DelegateCallbackTextBoxDebug fxnCallback, string strLog)
        {
            if (Application.Current != null)
            {
                Application.Current.Dispatcher.Invoke(
                    DispatcherPriority.Send,
                    new DelegateCallbackTextBoxDebug(fxnCallback),
                    strLog
                    );
            }
        }

        /// <summary>
        /// Internal function for updating of UI
        /// </summary>
        private void CallUICallback(Task task, bool bResult)
        {
            task.SetResult(bResult);

            if (Application.Current != null)
            {
                Application.Current.Dispatcher.Invoke(
                    DispatcherPriority.ApplicationIdle,
                    new DelegateCallbackTask(UICallback),
                    task.Result
                    );
            }
        }

        /// <summary>
        /// Set the UI callback
        /// </summary>
        public void SetUICallback(DelegateCallbackTask fxnCallback)
        {
            UICallback = fxnCallback;
        }

        /// <summary>
        /// Check if the path is the same for hotplugging
        /// Used for HotPlug
        /// </summary>
        public bool IsDevicePath(string szDevicePath)
        {
            if (!szDevicePath.Contains(d3xxDevice.SerialNumber))
            {
                return false;
            }

            return true;
        }

        /// <summary>
        /// Check if the path is the same for hotplugging
        /// Used for HotPlug2
        /// </summary>
        public bool IsDevicePathEx(string szDevicePath)
        {
            if (!d3xxDevice.IsOpen)
            {
                return false;
            }

            var ftStatus = d3xxDevice.IsDevicePath(szDevicePath);
            if (ftStatus != FTDI.FT_STATUS.FT_OK)
            {
                return false;
            }

            return true;
        }

        /// <summary>
        /// Get D3xx Guid
        /// </summary>
        public static Guid GetGuid()
        {
            return FTDI.FT_GUID;
        }

        private const UInt32 ulMaxChannels = 4;
        private WriterThread[] objectWriter;
        private Thread[] threadWriter;
        private ReaderThread[] objectReader;
        private Thread[] threadReader;
        private Queue<Task> queueTasks = new Queue<Task>();
        private FTDI d3xxDevice = new FTDI();
        private DelegateCallbackTask UICallback;
        private Loopback objectLoopback;
    }
}
