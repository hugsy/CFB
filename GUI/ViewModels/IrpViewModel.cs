using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GUI.Helpers;
using GUI.Models;
using GUI.Native;
using Windows.ApplicationModel;
using Windows.Security.Cryptography;
using Windows.Storage;
using Windows.Storage.Provider;
using Windows.Storage.Streams;

namespace GUI.ViewModels
{
    public class IrpViewModel : BindableBase
    {

        public IrpViewModel(Irp irp = null)
            => _model = irp ?? new Irp();


        private Irp _model;
        
        public Irp Model
        {
            get => _model;
        }


        private bool _isLoading = false;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }



        public DateTime TimeStamp       { get => DateTime.FromFileTime((long)Model.header.TimeStamp); }
        public string TimeStampString   { get => TimeStamp.ToString("yyyy/MM/dd HH:mm:ss.fffffff"); }
        public uint ProcessId           { get => Model.header.ProcessId; }
        public uint ThreadId            { get => Model.header.ThreadId; }
        public string IrqLevel          { get => Model.IrqlAsString(); }
        public string Type              { get => Model.TypeAsString(); }
        public uint IoctlCode           { get => Model.header.IoctlCode; }
        public uint Status              { get => Model.header.Status; }
        public uint InputBufferLength   { get => Model.header.InputBufferLength; }
        public uint OutputBufferLength  { get => Model.header.OutputBufferLength; }
        public string DriverName        { get => Model.header.DriverName; }
        public string DeviceName        { get => Model.header.DeviceName; }
        public string ProcessName       { get => Model.header.ProcessName; }
        public byte[] InputBuffer       { get => Model.body.InputBuffer; }
        public byte[] OutputBuffer      { get => Model.body.OutputBuffer; }

        public string IoctlCodeString    { get => 
                Model.header.Type == (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL || Model.header.Type == (uint)IrpMajorType.IRP_MJ_INTERNAL_DEVICE_CONTROL 
                ? $"0x{IoctlCode.ToString("x8")}"
                : "N/A"; }
        public string StatusString       { get => $"0x{Status.ToString("x8")}"; }

        public string StatusFullString   { get => Utils.FormatMessage(Status); }


        private string ShowStringMax(string s, int nb)
        {
            if (s == null || s.Length == 0)
                return "";

            int len = Math.Min(s.Length, nb);

            var t = s.Substring(0, len);
            if (len == nb)
                t += "[...]";
            return t;
        }


        public string InputBufferString  { get => InputBuffer != null ? ShowStringMax(BitConverter.ToString(InputBuffer), 10 * 3): ""; }
        public string OutputBufferString { get => OutputBuffer != null ? ShowStringMax(BitConverter.ToString(OutputBuffer), 10 * 3): ""; }


        /// <summary>
        /// Export the IRP as a standalone script whose type is given as parameter.
        /// </summary>
        /// <param name="_type">File type of the script</param>
        /// <returns>True on success, Exception() upon failure</returns>
        public async Task<bool> ExportAs(string _type)
        {
            if (Model.header.Type != (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL)
                throw new Exception("Only IRP_MJ_DEVICE_CONTROL IRP can be replayed...");


            List<string> ValidTypes = new List<string>() {
                "Raw",
                "Powershell",
                "Python",
                "C"
            };

            if (!ValidTypes.Contains(_type))
            {
                throw new Exception("Invalid export type provided");
            }

            string template_filepath = null;

            if (_type != "Raw")
            {
                template_filepath = Package.Current.InstalledLocation.Path + $"\\ScriptTemplates\\{_type:s}Template.txt";
                if (!File.Exists(template_filepath))
                {
                    throw new Exception($"SaveAs{_type}Script(): missing template");
                }
            }

            return await GenerateBodyScript(_type, template_filepath);
        }

        /// <summary>
        /// Generate the script body 
        /// </summary>
        /// <param name="TypeStr"></param>
        /// <param name="template_file"></param>
        /// <returns></returns>
        private async Task<bool> GenerateBodyScript(string TypeStr, string template_file)
        {

            IBuffer output;

            if (TypeStr == "Raw")
            {
                output = CryptographicBuffer.CreateFromByteArray(this.InputBuffer);
            }
            else
            {
                var DeviceName = this.DeviceName.Replace(@"\Device", @"\\.");
                var IrpDataInStr = "";
                var IrpDataOutStr = "\"\"";

                foreach (byte c in this.Model.body.InputBuffer)
                    IrpDataInStr += $"{c:X2}";

                if (this.OutputBufferLength > 0)
                    IrpDataOutStr = $"b'\\x00'*{this.OutputBufferLength:d}";

                var fmt = File.ReadAllText(template_file);
                output = CryptographicBuffer.ConvertStringToBinary(
                        String.Format(fmt,
                        this.IoctlCode,
                        DeviceName,
                        this.DriverName,
                        $"\"{IrpDataInStr}\"",
                        this.OutputBufferLength
                    ),
                    BinaryStringEncoding.Utf8
                );
            }


            //
            // write it to disk
            //
            var savePicker = new Windows.Storage.Pickers.FileSavePicker();
            savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
            savePicker.SuggestedFileName = $"Irp-{this.IoctlCodeString}-Session-{DateTime.Now.ToString("yyyyMMddTHH:mm:ssZ")}";

            switch (TypeStr)
            {
                case "Powershell":
                    savePicker.FileTypeChoices.Add("PowerShell", new List<string>() { ".ps1" });
                    break;

                case "Python":
                    savePicker.FileTypeChoices.Add("Python", new List<string>() { ".py" });
                    break;

                case "C":
                    savePicker.FileTypeChoices.Add("C", new List<string>() { ".c" });
                    break;
            }

            StorageFile file = await savePicker.PickSaveFileAsync();
            if (file != null)
            {
                CachedFileManager.DeferUpdates(file);
                await FileIO.WriteBufferAsync(file, output);
                FileUpdateStatus status = await CachedFileManager.CompleteUpdatesAsync(file);
                if (status == FileUpdateStatus.Complete)
                    return true;
            }

            throw new Exception($"Couldn't save IRP as a {TypeStr} script...");
        }
    }
}
