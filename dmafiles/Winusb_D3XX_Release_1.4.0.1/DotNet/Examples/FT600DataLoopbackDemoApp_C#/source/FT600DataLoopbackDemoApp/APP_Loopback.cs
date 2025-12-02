/*
** APP_Loopback.cs
**
** Copyright © 2016 Future Technology Devices International Limited
**
** C# Source file for Data Loopback Demo Application.
**
** Author: FTDI
** Project: C# Data Loopback Demo Application
** Module: Loopback implementation
**
** History:
**  1.0.0.0	- Initial version
**
*/

using System;
using System.IO;
using System.Security.Cryptography;
using DemoUtility;



namespace DemoApp
{
    /// <summary>
    /// Loopback implementation to be used by the thread manager
    /// </summary>
    public class Loopback
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public Loopback(byte _bNumChannels)
        {
            bNumChannels = _bNumChannels;
            arrayParams = new Params[bNumChannels];

            for (byte bChannelIndex = 0; bChannelIndex < bNumChannels; bChannelIndex++)
            {
                arrayParams[bChannelIndex].bStarted = false;
                arrayParams[bChannelIndex].ulNumBytesRead = 0;
                arrayParams[bChannelIndex].ulNumBytesWritten = 0;
            }

            fileDirectory = Environment.CurrentDirectory + "\\" + szDirectoryName;
        }

        /// <summary>
        /// Begin loopback on the channel
        /// </summary>
        public void Begin(byte bChannelIndex, bool bDeleteFilesAfterVerification)
        {
            if (bChannelIndex < bNumChannels)
            {
                if (!arrayParams[bChannelIndex].bStarted)
                {
                    arrayParams[bChannelIndex].bStarted = true;

                    var timeNow = DateTime.Now;

                    arrayParams[bChannelIndex].fileNameWrite = string.Format(
                        "{0}_{1}_Payload_Channel{2}_{3}.txt",
                        timeNow.ToString("yyyyMMdd"),
                        timeNow.ToString("hhmmssff"),
                        bChannelIndex + 1,
                        "Write"
                        );

                    arrayParams[bChannelIndex].fileNameRead = string.Format(
                        "{0}_{1}_Payload_Channel{2}_{3}.txt",
                        timeNow.ToString("yyyyMMdd"),
                        timeNow.ToString("hhmmssff"),
                        bChannelIndex + 1,
                        "Read"
                        );

                    bDeleteFiles = bDeleteFilesAfterVerification;
                }
            }
        }

        /// <summary>
        /// End loopback on the channel
        /// </summary>
        public void End(byte bChannelIndex, bool bPreserveFiles)
        {
            if (bChannelIndex < bNumChannels)
            {
                if (arrayParams[bChannelIndex].ulNumBytesWritten == arrayParams[bChannelIndex].ulNumBytesRead)
                {
                    ClearBytesTransferred(bChannelIndex);
                    arrayParams[bChannelIndex].bStarted = false;

                    if (bDeleteFiles && !bPreserveFiles)
                    {
                        DeleteFiles(bChannelIndex);
                    }
                }
            }
        }

        /// <summary>
        /// Get filename of the loopback log
        /// </summary>
        public string GetFileName(byte bChannelIndex, bool bWrite)
        {
            if (bWrite)
            {
                return arrayParams[bChannelIndex].fileNameWrite;
            }

            return arrayParams[bChannelIndex].fileNameRead;
        }

        /// <summary>
        /// Get directory of the loopback log
        /// </summary>
        public string GetFileDirectory()
        {
            return fileDirectory;
        }

        /// <summary>
        /// Verify the loopback count of data written and read
        /// </summary>
        public bool IsLoopbackComplete(byte bChannelIndex, byte bPipe)
        {
            if (bChannelIndex < bNumChannels)
            {
                if (arrayParams[bChannelIndex].ulNumBytesWritten == 
                    arrayParams[bChannelIndex].ulNumBytesRead)
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Add bytes to the loopback counters
        /// </summary>
        public void AddBytesTransferred(byte bChannelIndex, bool bWrite, UInt32 ulBytesTransferred)
        {
            if (bChannelIndex < bNumChannels)
            {
                if (bWrite)
                {
                    arrayParams[bChannelIndex].ulNumBytesWritten += ulBytesTransferred;
                }
                else
                {
                    arrayParams[bChannelIndex].ulNumBytesRead += ulBytesTransferred;
                }
            }
        }

        /// <summary>
        /// Reset the loopback counters
        /// </summary>
        public void ClearBytesTransferred(byte bChannelIndex)
        {
            if (bChannelIndex < bNumChannels)
            {
                arrayParams[bChannelIndex].ulNumBytesWritten = 0;
                arrayParams[bChannelIndex].ulNumBytesRead = 0;
            }
        }

        /// <summary>
        /// Generate data for the loopback transfer to be called by the Writer threads
        /// </summary>
        public static void GenerateData(byte[] bBuffer)
        {
            Random randomizer = new Random();
            int type = randomizer.Next(3);

            switch (type)
            {
                case 0: // Incremental data
                {
                    byte b = 0;
                    for (int i = 0; i < bBuffer.Length; i++)
                    {
                        bBuffer[i] = b++;
                    }

                    break;
                }
                case 1: // Fixed data
                {
                    byte b = (byte)randomizer.Next(256);
                    for (int i = 0; i < bBuffer.Length; i++)
                    {
                        bBuffer[i] = b;
                    }

                    break;
                }
                default: // Random data
                {
                    randomizer.NextBytes(bBuffer);

                    break;
                }
            }
        }

        /// <summary>
        /// Verify loopback files of the writer thread and reader thread
        /// </summary>
        public bool VerifyFiles(byte bChannelIndex)
        {
            try
            {
                var filePathWrite = fileDirectory + "\\" + arrayParams[bChannelIndex].fileNameWrite;
                var filePathRead = fileDirectory + "\\" + arrayParams[bChannelIndex].fileNameRead;

                if (!File.Exists(filePathWrite) || !File.Exists(filePathRead))
                {
                    return false;
                }

                var fileInfoWrite = new FileInfo(filePathWrite);
                var fileInfoRead = new FileInfo(filePathRead);

                if (fileInfoWrite == null || fileInfoRead == null)
                {
                    return false;
                }

                var streamWrite = fileInfoWrite.OpenRead();
                if (streamWrite == null)
                {
                    return false;
                }

                var streamRead = fileInfoRead.OpenRead();
                if (streamRead == null)
                {
                    streamWrite.Close();
                    return false;
                }

                byte[] hashWrite = MD5.Create().ComputeHash(streamWrite);
                byte[] hashRead = MD5.Create().ComputeHash(streamRead);

                if (!CompareHash(hashWrite, hashRead))
                {
                    if (!arrayParams[bChannelIndex].bMismatch)
                    {
                        arrayParams[bChannelIndex].bMismatch = true;
                        streamWrite.Close();
                        streamRead.Close();
                        return true;
                    }
                    else
                    {
                        streamWrite.Close();
                        streamRead.Close();
                        return false;
                    }
                }

                arrayParams[bChannelIndex].bMismatch = false;
                streamWrite.Close();
                streamRead.Close();
                return true;
            }
            catch (IOException ex)
            {
                LogFile.Log(ex.ToString());
                return false;
            }
        }

        /// <summary>
        /// Compare the hashes of the write and read pipes
        /// </summary>
        private bool CompareHash(byte[] hashWrite, byte[] hashRead)
        {
            if (hashWrite == null || hashRead == null)
            {
                return false;
            }

            for (int i = 0; i < hashWrite.Length; i++)
            {
                if (hashWrite[i] != hashRead[i])
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Delete loopback files of the writer thread and reader thread
        /// </summary>
        private void DeleteFiles(byte bChannelIndex)
        {
            try
            {
                var filePathWrite = fileDirectory + "\\" + arrayParams[bChannelIndex].fileNameWrite;
                var filePathRead = fileDirectory + "\\" + arrayParams[bChannelIndex].fileNameRead;

                if (File.Exists(filePathWrite))
                {
                    File.Delete(filePathWrite);
                }

                if (File.Exists(filePathRead))
                {
                    File.Delete(filePathRead);
                }
            }
            catch (IOException ex)
            {
                LogFile.Log(ex.ToString());
            }
        }

        private struct Params
        {
            public UInt32 ulNumBytesWritten;
            public UInt32 ulNumBytesRead;
            public string fileNameWrite;
            public string fileNameRead;
            public bool bStarted;
            public bool bMismatch;
        };

        private Params[] arrayParams;
        private byte bNumChannels;
        private string fileDirectory;
        private const string szDirectoryName = "FT600DataLoopback_Output";
        private bool bDeleteFiles;
    }
}
