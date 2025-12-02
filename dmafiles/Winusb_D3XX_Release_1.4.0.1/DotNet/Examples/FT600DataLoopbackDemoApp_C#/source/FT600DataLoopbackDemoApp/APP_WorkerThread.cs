/*
** APP_WorkerThread.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for Demo Application.
**
** Author: FTDI
** Project: C# Data Loopback Demo Application
** Module: Worker thread implementation
**
** History:
**  1.0.0.0	- Initial version
**
*/

using System;
using System.Windows;
using System.Windows.Threading;
using System.Threading;
using System.IO;
using System.Collections.Generic;
using DemoUtility;
using FTD3XXWU_NET;



namespace DemoApp
{
#region Worker Thread Class
    /// <summary>
    /// Worker thread class implementation
    /// </summary>
    public abstract class WorkerThread
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public WorkerThread(
            FTDI _d3xxDevice,
            ref TransferParams _oParams
            )
        {
            d3xxDevice = _d3xxDevice;
            oParams.bChannelIndex = _oParams.bChannelIndex;
            oParams.bPipe = _oParams.bPipe;
            oParams.bAsync = _oParams.bAsync;
            oParams.bStress = _oParams.bStress;
            oParams.ulPacketSize = _oParams.ulPacketSize;
            oParams.ulQueueSize = _oParams.ulQueueSize;
            oParams.fxnCallbackCompleted = _oParams.fxnCallbackCompleted;
            oParams.fxnCallbackDebug = _oParams.fxnCallbackDebug;
            oParams.fxnCallbackRate = _oParams.fxnCallbackRate;
            oParams.fileDirectory = _oParams.fileDirectory;
            oParams.fileName = _oParams.fileName;
            oParams.bTestMode = _oParams.bTestMode;
        }

        /// <summary>
        /// Thread entry function
        /// </summary>
        public void Run()
        {
            FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OTHER_ERROR;
            byte[] bBuffer = new byte[oParams.ulPacketSize];
            bool bResult = true;
            UInt32 ulBytesTransferred = 0;
            threadCompleted = new ManualResetEvent(false);


            LogFile.Log("Worker[0x{0:X2}] begin...[{1}]", oParams.bPipe, oParams.ulPacketSize);

            if (oParams.IsWritePipe())
            {
                Loopback.GenerateData(bBuffer);
            }

            if ((oParams.bPipe & 0x80) != 0)
            {
                // Use asynchronous for Read transfer

                NativeOverlapped oOverlapped = new NativeOverlapped();
                UInt32 ulTotalBytesTransferred = 0;
                do
                {
                    ftStatus = TransferAsync(oParams.bPipe, bBuffer, oParams.ulPacketSize, ref ulBytesTransferred, ref oOverlapped);
                    if (ftStatus == FTDI.FT_STATUS.FT_IO_PENDING)
                    {
                        LogFile.Log("Worker[0x{0:X2}] TransferAsync FT_IO_PENDING", oParams.bPipe);
                        ftStatus = WaitAsync(ref oOverlapped, ref ulBytesTransferred);
                        if (ftStatus != FTDI.FT_STATUS.FT_OK)
                        {
                            break;
                        }
                        LogToFile(bBuffer, ulBytesTransferred);
                        ulTotalBytesTransferred += ulBytesTransferred;
                    }
                } while (ulTotalBytesTransferred < oParams.ulPacketSize);
                ulBytesTransferred = ulTotalBytesTransferred;
                if (ftStatus != FTDI.FT_STATUS.FT_OK)
                {
                    d3xxDevice.AbortPipe(oParams.bPipe);
                    LogFile.Log("Worker[0x{0:X2}] TransferAsync/WaitAsync failed! ftStatus={1}", oParams.bPipe, ftStatus);
                    bResult = false;
                }
            }
            else
            {
                // Use synchronous for Write transfer

                ftStatus = TransferSync(oParams.bPipe, bBuffer, oParams.ulPacketSize, ref ulBytesTransferred);
                if (ftStatus == FTDI.FT_STATUS.FT_OK)
                {
                    LogToFile(bBuffer, ulBytesTransferred);
                }
                else
                {
                    d3xxDevice.AbortPipe(oParams.bPipe);
                    LogFile.Log("Worker[0x{0:X2}] TransferSync failed! ftStatus={1}", oParams.bPipe, ftStatus);
                    bResult = false;
                }
            }

            if (oParams.ulPacketSize != ulBytesTransferred)
            {
                LogFile.Log("Worker[0x{0:X2}] ulBytesTransferred={1}", oParams.bPipe, ulBytesTransferred);
                if (ulBytesTransferred == 0)
                {
                    bResult = false;
                }
            }

            threadCompleted.Set();
            bThreadRunning = false;

            LogToUI(bResult, ulBytesTransferred);

            LogFile.Log("Worker[0x{0:X2}] end...[{1}]", oParams.bPipe, ulBytesTransferred);
        }

        /// <summary>
        /// Wait for the completion
        /// </summary>
        private FTDI.FT_STATUS WaitAsync(ref NativeOverlapped oOverlapped, ref UInt32 ulBytesTransferred)
        {
            FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OTHER_ERROR;
            var timeStart = DateTime.Now;
            const double TIMEOUT_IN_MSEC = 5000;

            ulBytesTransferred = 0;
            do
            {
                ftStatus = d3xxDevice.WaitAsync(ref oOverlapped, ref ulBytesTransferred, true);
                if (ftStatus == FTDI.FT_STATUS.FT_OK)
                {
                    if (ulBytesTransferred == 0)
                    {
                        TimeSpan timeDiff = DateTime.Now - timeStart;
                        if (timeDiff.TotalMilliseconds > TIMEOUT_IN_MSEC)
                        {
                            LogFile.Log("Worker[0x{0:X2}] timeout {1}", oParams.bPipe, timeDiff.TotalMilliseconds);
                            break;
                        }

                        Thread.Sleep(16);
                        continue;
                    }
                }

                break;
            }
            while (Application.Current != null && !IsStopped());

            return ftStatus;
        }

        /// <summary>
        /// Stop the thread
        /// </summary>
        public void Stop()
        {
            LogFile.Log("Worker[0x{0:X2}] stop ...", oParams.bPipe);

            if (bThreadRunning)
            {
                d3xxDevice.AbortPipe(oParams.bPipe);
                SetStop();
                threadCompleted.WaitOne();
            }

            LogFile.Log("Worker[0x{0:X2}] stop done...", oParams.bPipe);
        }

        /// <summary>
        /// Check if the thread is still running
        /// </summary>
        public bool IsRunning()
        {
            return bThreadRunning;
        }

        /// <summary>
        /// Abstract function to be override by Writer and Reader threads
        /// </summary>
        protected abstract 
            FTDI.FT_STATUS TransferSync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred);

        /// <summary>
        /// Abstract function to be override by Writer and Reader threads
        /// </summary>
        protected abstract
            FTDI.FT_STATUS TransferAsync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred, ref NativeOverlapped oOverlapped);

        /// <summary>
        /// Internal function to signal stopping of thread
        /// </summary>
        private void SetStop()
        {
            lock (lockStop)
            {
                bStop = true;
            }
        }

        /// <summary>
        /// Internal function to signal stopping of thread
        /// </summary>
        private bool IsStopped()
        {
            lock (lockStop)
            {
                return bStop;
            }
        }

        /// <summary>
        /// Internal function to call back the thread manager
        /// </summary>
        private void LogToUI(bool bResult, UInt32 ulBytesTransferred)
        {
            if (oParams.fxnCallbackCompleted != null)
            {
				LogRate(ulBytesTransferred);

                Task task = new Task();
            	task.SetResult(bResult, oParams.bChannelIndex, oParams.bPipe, ulBytesTransferred);

                if (Application.Current != null)
                {
                    System.Threading.Tasks.Task t = System.Threading.Tasks.Task.Run(delegate
                    {
                    	if (!oParams.IsWritePipe())
                    	{
                        	Thread.Sleep(32);
                    	}
                        oParams.fxnCallbackCompleted(task.Result);
                    });
                }
            }
        }

        /// <summary>
        /// Internal function to update the UI with the result
        /// </summary>
        private void LogRate(UInt32 ulRate)
        {
            if (oParams.fxnCallbackRate != null)
            {
                if (Application.Current != null)
                {
                    Application.Current.Dispatcher.BeginInvoke(
                        DispatcherPriority.Send,
                        new DelegateCallbackTextBoxRate(oParams.fxnCallbackRate),
                        oParams.bPipe,
                        ulRate
                        );
                }
            }
        }

        /// <summary>
        /// Internal function to log data to a file for loopback verification
        /// </summary>
        private void LogToFile(byte[] bBuffer, UInt32 lSize)
        {
            if (lSize == 0)
            {
                return;
            }

            if (lSize != bBuffer.Length)
            {
                Array.Resize(ref bBuffer, (Int32)lSize);
            }

            LogFile.Log(bBuffer, oParams.fileDirectory, oParams.fileName, true);
        }

        /// <summary>
        /// Transfer param structure submitted by task manager thread
        /// </summary>
        public struct TransferParams
        {
            public byte bChannelIndex;
            public byte bPipe;
            public bool bAsync;
            public bool bStress;
            public bool bTestMode;
            public UInt32 ulPacketSize;
            public UInt32 ulQueueSize;
            public DelegateCallbackTask fxnCallbackCompleted;
            public DelegateCallbackTextBoxDebug fxnCallbackDebug;
            public DelegateCallbackTextBoxRate fxnCallbackRate;
            public string fileDirectory;
            public string fileName;

            public bool IsWritePipe()
            {
                if (bPipe < 0x80)
                {
                    return true;
                }
                return false;
            }
        }

        private TransferParams oParams;
        protected FTDI d3xxDevice;
        private bool bStop = false;
        private Object lockStop = new Object();
        private ManualResetEvent threadCompleted;
        private bool bThreadRunning = true;
    }
#endregion


    /// <summary>
    /// Writer thread class implementation
    /// </summary>
    public class WriterThread : WorkerThread
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public WriterThread(FTDI _d3xxDevice, ref TransferParams _oParams) 
            : base(_d3xxDevice, ref _oParams)
        {
        }

        /// <summary>
        /// Override function for the abstract function in the base class
        /// </summary>
        protected override 
            FTDI.FT_STATUS TransferSync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred)
        {
            return d3xxDevice.WritePipe(bPipe, bBuffer, ulPacketSize, ref ulBytesTransferred);
        }

        /// <summary>
        /// Override function for the abstract function in the base class
        /// </summary>
        protected override
            FTDI.FT_STATUS TransferAsync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred, ref NativeOverlapped oOverlapped)
        {
            return d3xxDevice.WritePipeAsync(bPipe, bBuffer, ulPacketSize, ref ulBytesTransferred, ref oOverlapped);
        }
    }


    /// <summary>
    /// Reader thread class implementation
    /// </summary>
    public class ReaderThread : WorkerThread
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ReaderThread(FTDI _d3xxDevice, ref TransferParams _oParams)
            : base(_d3xxDevice, ref _oParams)
        {
        }

        /// <summary>
        /// Override function for the abstract function in the base class
        /// </summary>
        protected override 
            FTDI.FT_STATUS TransferSync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred)
        {
            return d3xxDevice.ReadPipe(bPipe, bBuffer, ulPacketSize, ref ulBytesTransferred);
        }

        /// <summary>
        /// Override function for the abstract function in the base class
        /// </summary>
        protected override
            FTDI.FT_STATUS TransferAsync
            (byte bPipe, byte[] bBuffer, UInt32 ulPacketSize, ref UInt32 ulBytesTransferred, ref NativeOverlapped oOverlapped)
        {
            return d3xxDevice.ReadPipeAsync(bPipe, bBuffer, ulPacketSize, ref ulBytesTransferred, ref oOverlapped);
        }
    }
}
