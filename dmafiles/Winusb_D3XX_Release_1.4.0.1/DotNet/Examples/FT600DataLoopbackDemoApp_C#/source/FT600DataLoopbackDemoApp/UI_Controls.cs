/*
** UI_Controls.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for Demo Application.
**
** Author: FTDI
** Project: C# Data Loopback Demo Application
** Module: UI Controls
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



namespace DemoApp
{
    public class Controls
    {
        /// <summary>
        /// Class for Open and Close button controls
        /// </summary>
        public class OpenClose
        {
            public OpenClose(Button _btnOpen, Button _btnClose)
            {
                btnOpen = _btnOpen;
                btnClose = _btnClose;
                state = State.Close;
            }

            public void SetState(State _state)
            {
                state = _state;

                if (state == State.Open)
                {
                    btnOpen.IsEnabled = false;
                    btnClose.IsEnabled = true;
                }
                else
                {
                    btnOpen.IsEnabled = true;
                    btnClose.IsEnabled = false;
                }
            }

            public void SetDisable()
            {
                btnOpen.IsEnabled = false;
                btnClose.IsEnabled = false;
            }

            public bool IsOpen()
            {
                if (state == State.Open)
                {
                    return true;
                }

                return false;
            }

            private Button btnOpen;
            private Button btnClose;
            private State state;
        }

        /// <summary>
        /// Class for OpenBy options controls
        /// </summary>
        public class OpenBy
        {
            public OpenBy(List<Tuple<RadioButton, TextBox>> _ListOpenBy)
            {
                ListOpenBy = _ListOpenBy;
            }

            public void Select(RadioButton radioBtn)
            {
                var OpenBy = ListOpenBy.Find(x => x.Item1 == radioBtn);

                OpenBy.Item2.IsEnabled = (bool)OpenBy.Item1.IsChecked;
                if (OpenBy.Item2.IsEnabled == false)
                {
                    OpenBy.Item2.Foreground = System.Windows.Media.Brushes.Black;
                }
                else
                {
                    OpenBy.Item2.Foreground = System.Windows.Media.Brushes.White;
                }
            }

            public void SelectDefault()
            {
                if (ListOpenBy[(byte)Type.Index].Item1.IsChecked == false)
                {
                    foreach (var OpenBy in ListOpenBy)
                    {
                        if (OpenBy.Item1.IsChecked == true)
                        {
                            OpenBy.Item1.IsChecked = false;
                            OpenBy.Item2.IsEnabled = false;
                            OpenBy.Item2.Foreground = System.Windows.Media.Brushes.Black;
                        }
                    }

                    ListOpenBy[(byte)Type.Index].Item1.IsChecked = true;
                    ListOpenBy[(byte)Type.Index].Item2.IsEnabled = true;
                    ListOpenBy[(byte)Type.Index].Item2.Foreground = System.Windows.Media.Brushes.White;
                }
            }

            public void GetSelectedOption(ref byte bOption, ref string strOption)
            {
                byte i = 0;

                foreach (var OpenBy in ListOpenBy)
                {
                    if (OpenBy.Item1.IsChecked == true)
                    {
                        strOption = OpenBy.Item2.Text;
                        bOption = i++;
                        break;
                    }

                    i++;
                }
            }

            public void SetOptionValue(byte bOption, string strOption)
            {
                if (bOption < ListOpenBy.Count)
                {
                    ListOpenBy[bOption].Item2.Text = strOption;
                }
            }

            public void SetState(State state)
            {
                bool bEnable = (state == State.Open ? false : true);

                foreach (var OpenBy in ListOpenBy)
                {
                    OpenBy.Item2.IsEnabled = (bool)OpenBy.Item1.IsChecked;
                    OpenBy.Item1.IsEnabled = bEnable;

                    if (OpenBy.Item2.IsEnabled == false)
                    {
                        OpenBy.Item2.Foreground = System.Windows.Media.Brushes.Black;
                    }
                    else
                    {
                        OpenBy.Item2.Foreground = System.Windows.Media.Brushes.White;
                    }
                }
            }

            public enum Type
            {
                Description,
                SerialNumber,
                Index,
                Count,
            };

            private List<Tuple<RadioButton, TextBox>> ListOpenBy;
        }

        /// <summary>
        /// Class for Pipe transfer controls
        /// </summary>
        public class PipeTransfer
        {
            public PipeTransfer(
                Button _btnStartAll,
                Button _btnStopAll,
                List<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>> _ListPipes,
                List<Tuple<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>,
                           Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>>> _ListChannels
                )
            {
                btnStartAll = _btnStartAll;
                btnStopAll = _btnStopAll;
                ListPipes = _ListPipes;
                ListChannels = _ListChannels;
                foreach (var Pipe in ListPipes)
                {
                    ListStressRunning.Add(false);
                }
            }

            public delegate void DelegateExecute(Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe);

            public void ExecuteAllPipes(DelegateExecute fxn, Int32 lDelay)
            {
                foreach (var Pipe in ListPipes)
                {
                    fxn(Pipe);
                    if (lDelay > 0)
                    {
                        Thread.Sleep(lDelay);
                    }
                }
            }

            public void ExecuteAllWritePipes(DelegateExecute fxn, Int32 lDelay)
            {
                foreach (var Pipe in ListPipes)
                {
                    if (GetPipeID(Pipe) < 0x80)
                    {
                        fxn(Pipe);
                        if (lDelay > 0)
                        {
                            Thread.Sleep(lDelay);
                        }
                    }
                }
            }

            public void ExecuteAllReadPipes(DelegateExecute fxn, Int32 lDelay)
            {
                foreach (var Pipe in ListPipes)
                {
                    if (GetPipeID(Pipe) > 0x80)
                    {
                        fxn(Pipe);
                        if (lDelay > 0)
                        {
                            Thread.Sleep(lDelay);
                        }
                    }
                }
            }

            public void SetStressAllPipes(bool bStart)
            {
                for (int i = 0; i<ListStressRunning.Count; i++)
                {
                    ListStressRunning[i] = bStart;
                }
            }

            public void SetStress(byte bPipe, bool bStart)
            {
                int i = 0;
                foreach (var Pipe in ListPipes)
                {
                    if (GetPipeID(Pipe) == bPipe)
                    {
                        ListStressRunning[i] = bStart;
                        if (bPipe < 0x80)
                        {
                            ListStressRunning[i + 1] = bStart;
                        }
                        else
                        {
                            ListStressRunning[i - 1] = bStart;
                        }
                        break;
                    }
                    i++;
                }                
            }

            public bool IsSetStress(byte bPipe)
            {
                int i = 0;
                foreach (var Pipe in ListPipes)
                {
                    if (GetPipeID(Pipe) == bPipe)
                    {
                        if (bPipe < 0x80)
                        {
                            if (!ListStressRunning[i] || !ListStressRunning[i + 1])
                            {
                                return false;
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else
                        {
                            if (!ListStressRunning[i] || !ListStressRunning[i - 1])
                            {
                                return false;
                            }
                            else
                            {
                                return true;
                            }
                        }
                    }
                    i++;
                }

                return false;
            }

            public void SetState(State state, byte bNumWritePipes = 0, byte bNumReadPipes = 0)
            {
                bool bEnable = (state == State.Open ? true : false);
                btnStartAll.IsEnabled = bEnable;
                btnStopAll.IsEnabled = bEnable;

                if (bEnable)
                {
                    byte bNumWritePipeCount = 0;
                    byte bNumReadPipeCount = 0;

                    foreach (var Pipe in ListPipes)
                    {
                        if (GetPipeID(Pipe) < 0x80)
                        {
                            if (bNumWritePipeCount < bNumWritePipes)
                            {
                                bNumWritePipeCount++;
                                SetCheck(Pipe, true);
                                SetEnable(Pipe, true);
                                SaveSize(4096);
                                SetSize(Pipe);
                            }
                            else
                            {
                                SetCheck(Pipe, false);
                                SetEnable(Pipe, false);
                            }
                        }
                        else
                        {
                            if (bNumReadPipeCount < bNumReadPipes)
                            {
                                bNumReadPipeCount++;
                                SetCheck(Pipe, true);
                                SetEnable(Pipe, true);
                                SaveSize(4096);
                                SetSize(Pipe);
                            }
                            else
                            {
                                SetCheck(Pipe, false);
                                SetEnable(Pipe, false);
                            }
                        }

                        SetPipeState(PipeState.CallbackOpen, Pipe, IsChecked(Pipe));
                        SetRate(Pipe, 0);
                    }
                }
                else
                {
                    foreach (var Pipe in ListPipes)
                    {
                        SetPipeState(PipeState.CallbackClose, Pipe, false);
                    }
                }
            }

            public void SetPipeState(
                PipeState state, 
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe, 
                bool bEnable)
            {
                switch (state)
                {
                    case PipeState.CallbackOpen: // fall-through
                    case PipeState.Select:
                        {
                            Pipe.Item3.IsEnabled = bEnable;
                            Pipe.Item4.IsEnabled = bEnable;
                            Pipe.Item5.IsEnabled = bEnable;
                            Pipe.Item6.IsEnabled = bEnable;
                            break;
                        }
                    case PipeState.Start:
                        {
                            Pipe.Item2.IsEnabled = false;
                            Pipe.Item3.IsEnabled = false;
                            Pipe.Item4.IsEnabled = false;
                            Pipe.Item5.IsEnabled = true;
                            Pipe.Item6.IsEnabled = true;
                            break;
                        }
                    case PipeState.Stop:
                        {
                            Pipe.Item6.IsEnabled = false;
                            break;
                        }
                    case PipeState.CallbackStart: // fall-through
                    case PipeState.CallbackStop:  // fall-through
                    case PipeState.CallbackClose:
                        {
                            Pipe.Item2.IsEnabled = bEnable;
                            Pipe.Item3.IsEnabled = bEnable;
                            Pipe.Item4.IsEnabled = bEnable;
                            Pipe.Item5.IsEnabled = bEnable;
                            Pipe.Item6.IsEnabled = bEnable;
                            break;
                        }
                }
            }

            public Tuple<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>,
                         Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>>
                GetChannel(byte bPipe)
            {
                return ListChannels.Find(x => GetPipeID(x.Item1) == bPipe || GetPipeID(x.Item2) == bPipe);
            }

            public Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>
                FindPipePartner(byte bPipe)
            {
                var Channel = ListChannels.Find(x => GetPipeID(x.Item1) == bPipe || GetPipeID(x.Item2) == bPipe);
                if (Channel != null)
                {
                    if (GetPipeID(Channel.Item1) == bPipe)
                    {
                        return Channel.Item2;
                    }
                    else
                    {
                        return Channel.Item1;
                    }
                }

                return null;
            }

            public Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>
                FindPipe(byte pipe)
            {
                return ListPipes.Find(x => GetPipeID(x) == pipe);
            }

            public Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>
                FindPipe(Button btn)
            {
                return ListPipes.Find(x => GetStart(x) == btn || GetStop(x) == btn);
            }

            public Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>
                FindPipe(CheckBox cb)
            {
                return ListPipes.Find(x => GetEnabled(x) == cb);
            }

            public byte GetPipeID(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item1;
            }

            public CheckBox GetEnabled(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item2;
            }

            public void SetEnable(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe, bool bEnable)
            {
                Pipe.Item2.IsEnabled = bEnable;
            }

            public bool IsEnabled(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item2.IsEnabled;
            }

            public void SetCheck(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe, bool bEnable)
            {
                Pipe.Item2.IsChecked = bEnable;
            }

            public bool IsChecked(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return (bool)Pipe.Item2.IsChecked;
            }

            public UInt32 GetSize(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return UInt32.Parse(Pipe.Item3.Text);
            }

            public void SetSize(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                if (Pipe.Item2.IsChecked == true)
                {
                    Pipe.Item3.Text = ulSaveSize.ToString();
                }
            }

            public void SaveSize(UInt32 ulSize)
            {
                ulSaveSize = ulSize;
            }

            public Button GetStart(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item4;
            }

            public Button GetStop(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item5;
            }

            public UInt32 GetRate(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return UInt32.Parse(Pipe.Item6.Text);
            }

            public void SetRate(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe, UInt32 ulRate)
            {
                Pipe.Item6.Text = ulRate.ToString();
            }

            public byte GetChannelID(
                Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte> Pipe)
            {
                return Pipe.Item7;
            }

            private List<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>> ListPipes;
            private List<Tuple<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>,
                               Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>>> ListChannels;
            private Button btnStartAll;
            private Button btnStopAll;
            private List<bool> ListStressRunning = new List<bool>();
            private UInt32 ulSaveSize = 0;
        }

        /// <summary>
        /// Class for Stress testing controls
        /// </summary>
        public class StressTest
        {
            public StressTest(CheckBox _cbEnable, CheckBox _cbRandom, TextBox _tbSize)
            {
                cbEnable = _cbEnable;
                cbRandom = _cbRandom;
                tbSize = _tbSize;
            }

            public void SetState(State state)
            {
                bool bEnable = (state == State.Open ? true : false);
                cbEnable.IsEnabled = bEnable;
                cbRandom.IsEnabled = bEnable;
                tbSize.IsEnabled = bEnable;
            }

            public void Enable(bool bEnable)
            {
                cbEnable.IsChecked = bEnable;
            }

            public bool IsEnabled()
            {
                return (bool)cbEnable.IsChecked;
            }

            public void Random(bool bEnable)
            {
                cbRandom.IsChecked = bEnable;
            }

            public bool IsRandom()
            {
                return (bool)cbRandom.IsChecked;
            }

            public UInt32 GetSize()
            {
                return UInt32.Parse(tbSize.Text);
            }

            public void SetSize(UInt32 ulSize)
            {
                tbSize.Text = ulSize.ToString();
            }

            private CheckBox cbEnable;
            private CheckBox cbRandom;
            private TextBox tbSize;
        }


        public void SetOpenClose(Button _btnOpen, Button _btnClose)
        {
            oOpenClose = new OpenClose(_btnOpen, _btnClose);
        }

        public void SetOpenBy(List<Tuple<RadioButton, TextBox>> _ListOpenBy)
        {
            oOpenBy = new OpenBy(_ListOpenBy);
        }

        public void SetPipeTransfer(
            Button _btnStartAll,
            Button _btnStopAll,
            List<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>> _ListPipes,
            List<Tuple<Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>,
                        Tuple<byte, CheckBox, TextBox, Button, Button, TextBox, byte>>> _ListChannels
            )
        {
            oPipeTransfer = new PipeTransfer(_btnStartAll, _btnStopAll, _ListPipes, _ListChannels);
        }

        public void SetStressTest(CheckBox _cbEnable, CheckBox _cbRandom, TextBox _tbSize)
        {
            oStressTest = new StressTest(_cbEnable, _cbRandom, _tbSize);
        }


        public enum State
        {
            Open,
            Close
        };

        public enum PipeState
        {
            Select,
            Start,
            Stop,
            CallbackOpen,
            CallbackStart,
            CallbackStop,
            CallbackClose,
        };

        public OpenClose oOpenClose;
        public OpenBy oOpenBy;
        public PipeTransfer oPipeTransfer;
        public StressTest oStressTest;
    }
}

