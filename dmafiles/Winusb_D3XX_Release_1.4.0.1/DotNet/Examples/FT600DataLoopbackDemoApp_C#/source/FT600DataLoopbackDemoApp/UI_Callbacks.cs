/*
** UI_Callbacks.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for Demo Application.
**
** Author: FTDI
** Project: C# Data Loopback Demo Application
** Module: Callback implementation
**
** History:
**  1.0.0.0	- Initial version
**
*/

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Threading;
using DemoUtility;



namespace DemoApp
{
    public delegate void DelegateCallbackTask(Task.TaskResult oResult);
    public delegate void DelegateCallbackTextBoxDebug(string szDebug);
    public delegate void DelegateCallbackTextBoxRate(byte bPipe, UInt32 ulRateMBps);
    public delegate void DelegeteCallbackHotPlug();

    public partial class UI_MainWindow : Window
    {
        /// <summary>
        /// Callback function called by the thread manager
        /// </summary>
        public void Callback(Task.TaskResult oResult)
        {
            switch (oResult.eType)
            {
                case Task.TaskType.Detect:
                    {
                        CallbackDetect(oResult.bResult, oResult.ulNumDevices);
                        break;
                    }

                case Task.TaskType.Open:
                    {
                        CallbackOpen(oResult.bResult, oResult.bNumWritePipes, oResult.bNumReadPipes, oResult.bTransferResult);
                        break;
                    }
                case Task.TaskType.Close:
                    {
                        CallbackClose(oResult.bResult);
                        break;
                    }
                case Task.TaskType.Start:
                    {
                        CallbackTransfer(oResult.bPipe, oResult.bResult,
                            oResult.bLoopbackCompleted, oResult.bTransferResult, oResult.ulBytesTransferred);
                        break;
                    }
                case Task.TaskType.Stop:
                    {
                        CallbackAbortTransfer(oResult.bPipe, oResult.bResult);
                        break;
                    }
            }
        }

        /// <summary>
        /// Internal helper function called by Callback()
        /// </summary>
        private void CallbackDetect(bool bResult, UInt32 ulNumDevices)
        {
            if (bResult && ulNumDevices > 0)
            {
                LogFile.Log("CallbackDetect success={0}\r\n", bResult);

                // Automatically open the device

                string szOption = "";
                byte bOption = 0;

                oControl.oOpenBy.SelectDefault();
                oControl.oOpenBy.GetSelectedOption(ref bOption, ref szOption);
                if (UInt32.Parse(szOption) >= ulNumDevices)
                {
                    szOption = "0";
                    oControl.oOpenBy.SetOptionValue(bOption, szOption);
                }

                TaskAddOpen(bOption, szOption);
            }
            else
            {
                TextBoxOutputReset();
                TextBoxOutput("No device is connected!\n");
                LogFile.Log("CallbackDetect success={0}\r\n", bResult);
            }
        }

        /// <summary>
        /// Internal helper function called by Callback()
        /// </summary>
        private void CallbackOpen(bool bResult, byte bNumWritePipes, byte bNumReadPipes, bool bTransferResult)
        {
            if (bResult)
            {
                oControl.oOpenClose.SetState(Controls.State.Open);
                oControl.oOpenBy.SetState(Controls.State.Open);
                oControl.oPipeTransfer.SetState(Controls.State.Open, bNumWritePipes, bNumReadPipes);
                oControl.oStressTest.SetState(Controls.State.Open);
                TextBoxOutputColor(true);
            }
            else
            {
                if (bNumWritePipes == 0 && bNumReadPipes == 0)
                {
                    TextBoxOutput("No device is connected!\n");
                }
                else if (bNumWritePipes == 0 || bNumReadPipes == 0)
                {
                    TextBoxOutput("\r\n");
                    TextBoxOutput("ERROR: Device can be opened but invalid channel configuration detected!\r\n");
                    TextBoxOutput("\r\n");
                    TextBoxOutput("Loopback requires atleast 2 pipes (1OUT/1IN) enabled!\n");
                    TextBoxOutput("If using default configuration, please add/remove GPIO pin jumpers!\n");
                    TextBoxOutput("Otherwise if using custom configuration, change configuration settings to use at least 1 channel (1 OUT and 1 IN pipes)\n");
                    TextBoxOutput("\r\n\r\n\r\n\r\n");
                }
                else if (!bTransferResult)
                {
                    TextBoxOutput("\r\n");
                    TextBoxOutput("ERROR: Device can be opened but basic loopback failed!\r\n");
                    TextBoxOutput("\r\n");
                    TextBoxOutput("Possible causes:\r\n");
                    TextBoxOutput("1. FPGA is for 245 mode but chip was not configured to 245 mode.\r\n");
                    TextBoxOutput("2. FPGA is for 600 mode but chip was configured to 245 mode.\r\n");
                    TextBoxOutput("3. No FPGA is connected to the PCB board.\r\n");
                    TextBoxOutput("4. FPGA image is not correct.\n   (Ex: Data Streamer FPGA is used instead of Data Loopback FPGA)\r\n");
                    TextBoxOutput("5. FPGA image is not sending data. (Ex: FPGA has bugs. Try to reset FPGA.)\r\n");
                    TextBoxOutput("\r\n\r\n");
                }
            }

            LogFile.Log("CallbackOpen success={0}\r\n", bResult);
        }

        /// <summary>
        /// Internal helper function called by Callback()
        /// </summary>
        private void CallbackClose(bool bResult)
        {
            oControl.oOpenClose.SetState(Controls.State.Close);
            oControl.oOpenBy.SetState(Controls.State.Close);
            oControl.oPipeTransfer.SetState(Controls.State.Close);
            oControl.oStressTest.SetState(Controls.State.Close);
            TextBoxOutputColor(false);

            LogFile.Log("CallbackClose success={0}\r\n", bResult);
        }

        /// <summary>
        /// Internal helper function called by Callback()
        /// </summary>
        private void CallbackTransfer(byte bPipe, bool bResult, bool bLoopbackCompleted, bool bLoobackResult, UInt32 ulBytesTransferred)
        {
            var Pipe = oControl.oPipeTransfer.FindPipe(bPipe);
            oControl.oPipeTransfer.SetPipeState(Controls.PipeState.CallbackStart, Pipe, true);

            if (bPipe < 0x80)
            {
                TextBoxOutput(string.Format("[EP{0:X2}] Written {1} bytes to device!\n", bPipe, ulBytesTransferred));
                LogFile.Log(string.Format("[EP{0:X2}] Written {1} bytes to device!\n", bPipe, ulBytesTransferred));
            }
            else
            {
                TextBoxOutput(string.Format("[EP{0:X2}] Read {1} bytes from device!\n", bPipe, ulBytesTransferred));
                LogFile.Log(string.Format("[EP{0:X2}] Read {1} bytes from device!\n", bPipe, ulBytesTransferred));
            }

            if (bLoopbackCompleted)
            {
                var Channel = oControl.oPipeTransfer.GetChannel(bPipe); 
                if (Channel == null)
                {
                    return;
                }

                if (!bLoobackResult)
                {
                    TextBoxOutput(String.Format("RESULT: Verified files does NOT match! FAILED! 0x{0:X2} 0x{1:X2}\n", 
                        oControl.oPipeTransfer.GetPipeID(Channel.Item1), oControl.oPipeTransfer.GetPipeID(Channel.Item2)));
                    LogFile.Log("RESULT: Verified files does NOT match! FAILED 0x{0:X2} 0x{1:X2}!\n", 
                        oControl.oPipeTransfer.GetPipeID(Channel.Item1), oControl.oPipeTransfer.GetPipeID(Channel.Item2));
                    oControl.oStressTest.Random(false);

                    oControl.oPipeTransfer.SetRate(Channel.Item1, 0);
                    oControl.oPipeTransfer.SetRate(Channel.Item2, 0);
                }
                else
                {
                    TextBoxOutput(String.Format("RESULT: Verified files do match! PASSED! [0x{0:X2} 0x{1:X2}]\n",
                        oControl.oPipeTransfer.GetPipeID(Channel.Item1), oControl.oPipeTransfer.GetPipeID(Channel.Item2)));
                    LogFile.Log("RESULT: Verified files do match! PASSED [0x{0:X2} 0x{1:X2}]!\n",
                        oControl.oPipeTransfer.GetPipeID(Channel.Item1), oControl.oPipeTransfer.GetPipeID(Channel.Item2));

                    oControl.oPipeTransfer.SetRate(Channel.Item1, 0);
                    oControl.oPipeTransfer.SetRate(Channel.Item2, 0);

                    // If stress test mode, do transfer again
                    if (oControl.oPipeTransfer.IsSetStress(bPipe))
                    {
                        //if (bPipe == 0x82 || bPipe == 0x02)
                        {
                            Thread.Sleep(250);
                            if (Application.Current == null)
                            {
                                return;
                            }
                            if (!oControl.oPipeTransfer.IsSetStress(bPipe))
                            {
                                return;
                            }
                        }

                        if (oControl.oStressTest.IsRandom())
                        {
                            Random randomizer = new Random();
                            UInt32 ulRandom = (UInt32)randomizer.Next((Int32)oControl.oStressTest.GetSize());
                            if (ulRandom == 0)
                            {
                                ulRandom = ulDefaultStressSize;
                            }

                            oControl.oPipeTransfer.SaveSize(ulRandom);
                            oControl.oPipeTransfer.SetSize(Channel.Item1);
                            oControl.oPipeTransfer.SetSize(Channel.Item2);
                        }

                        StartPipe(Channel.Item1);
                        Thread.Sleep(16);
                        StartPipe(Channel.Item2);
                        LogFile.Log("CallbackTransfer 0x{0:X2}", bPipe);
                        return;
                    }
                }
            }

            LogFile.Log("CallbackTransfer success={0} pipe=0x{1:X2}\r\n", bResult, bPipe);
        }

        /// <summary>
        /// Internal helper function called by Callback()
        /// </summary>
        private void CallbackAbortTransfer(byte bPipe, bool bResult)
        {
            var Pipe = oControl.oPipeTransfer.FindPipe(bPipe);
            oControl.oPipeTransfer.SetPipeState(Controls.PipeState.CallbackStop, Pipe, true);

            LogFile.Log("CallbackAbortTransfer success={0} pipe=0x{1:X2}", bResult, bPipe);
        }

        /// <summary>
        /// Callback function called by the thread manager
        /// </summary>
        public void CallbackDebug(string str)
        {
            TextBoxOutput(str);

            LogFile.Log(str);
        }

        /// <summary>
        /// Callback function called by the thread worker
        /// </summary>
        public void CallbackRate(byte bPipe, UInt32 ulRateMBps)
        {
            var Pipe = oControl.oPipeTransfer.FindPipe(bPipe);
            oControl.oPipeTransfer.SetRate(Pipe, ulRateMBps + oControl.oPipeTransfer.GetRate(Pipe));
        }

        /// <summary>
        /// Callback function for hot plugging called by the system
        /// </summary>
        public void CallbackDeviceInserted()
        {
            TextBoxOutput("Device is plugged!\r\n\r\n");
            LogFile.Log("Device is plugged!");

            oControl.oOpenBy.SelectDefault();
            TaskAddDetect();

            LogFile.Log("CallbackDeviceInserted");
        }

        /// <summary>
        /// Callback function for hot plugging called by the system
        /// </summary>
        public void CallbackDeviceRemoved()
        {
            TextBoxOutput("Plug-in device! Application will detect it automatically!\r\n\r\n");
            LogFile.Log("Device is unplugged!");

            if (oControl.oStressTest.IsEnabled())
            {
                oControl.oStressTest.Enable(false);
                oControl.oPipeTransfer.SetStressAllPipes(false);
            }

            TaskAddClose();

            LogFile.Log("CallbackDeviceRemoved");
        }
    }
}
