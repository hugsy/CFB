using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GUI.Models;
using Microsoft.Data.Sqlite;
using Windows.Storage;


namespace GUI.ViewModels
{
    public class SaveLoadIrpsViewModel : BindableBase
    {
        private string _filename = "temporary_db.cfb";

        public SaveLoadIrpsViewModel()
        {
            //
            // make sure the file exists
            //
            Task.Run(() =>
            {
               ApplicationData.Current.LocalFolder.CreateFileAsync(_filename, CreationCollisionOption.ReplaceExisting);
            });
        }

        

        private bool _isLoading = false;

        public bool IsLoading
        {
            get => _isLoading;
            set => Set(ref _isLoading, value);
        }


        private string _status;
        public string Status
        {
            get => _status;
            set => Set(ref _status, value);
        }


        private string DbPath
        {
            get => Path.Combine(ApplicationData.Current.LocalFolder.Path, _filename);
        }


        public int CountIrpEntries(string dbpath = null)
        {
            int nb_entries = 0;
            dbpath = dbpath ?? DbPath;

            using (var db = new SqliteConnection($"Filename={dbpath}"))
            {
                db.Open();

                var cmd = new SqliteCommand("SELECT COUNT(*) from Irps", db);
                var query = cmd.ExecuteReader();

                nb_entries = query.GetInt32(0);
                db.Close();
            }

            return nb_entries;
        }


        public async Task<StorageFile> DumpIrpsToFile()
        {
            var file = await ApplicationData.Current.LocalFolder.CreateFileAsync(_filename, CreationCollisionOption.OpenIfExists);

            using (var db = new SqliteConnection($"Filename={DbPath}"))
            {
                db.Open();

                //
                // Create the table
                //
                {
                    var req = @"
CREATE TABLE IF NOT EXISTS Irps (
    TimeStamp integer,
    IrqLevel integer,
    Type integer,
    IoctlCode integer,
    Status integer,
    ProcessId integer NOT NULL,
    ThreadId integer NOT NULL,
    InputBufferLength integer,
    OutputBufferLength integer,
    DriverName text NOT NULL,
    DeviceName text NOT NULL,
    ProcessName text NOT NULL,
    InputBuffer blob,
    OutputBuffer blob
)
";
                    var cmd = new SqliteCommand(req, db);
                    cmd.ExecuteReader();
                }


                //
                // Populate it
                //
                var tx = db.BeginTransaction();
                try
                {
                    foreach (var irp in await App.Irps.GetAsync())
                    {
                        var query = @"
                            INSERT INTO Irps VALUES(
                            @TimeStamp, 
                            @IrqLevel, 
                            @Type, 
                            @IoctlCode, 
                            @Status, 
                            @ProcessId, 
                            @ThreadId, 
                            @InputBufferLength, 
                            @OutputBufferLength, 
                            @DriverName, 
                            @DeviceName, 
                            @ProcessName,
                            @InputBuffer,
                            @OutputBuffer
                            )
                            ";
                        SqliteCommand cmd = new SqliteCommand(query, db, tx);
                        cmd.Parameters.AddWithValue("@TimeStamp", irp.header.TimeStamp);
                        cmd.Parameters.AddWithValue("@IrqLevel", irp.header.IrqLevel);
                        cmd.Parameters.AddWithValue("@Type", irp.header.Type);
                        cmd.Parameters.AddWithValue("@IoctlCode", irp.header.IoctlCode);
                        cmd.Parameters.AddWithValue("@Status", irp.header.Status);
                        cmd.Parameters.AddWithValue("@ProcessId", irp.header.ProcessId);
                        cmd.Parameters.AddWithValue("@ThreadId", irp.header.ThreadId);
                        cmd.Parameters.AddWithValue("@InputBufferLength", irp.header.InputBufferLength);
                        cmd.Parameters.AddWithValue("@OutputBufferLength", irp.header.OutputBufferLength);
                        cmd.Parameters.AddWithValue("@DriverName", irp.header.DriverName);
                        cmd.Parameters.AddWithValue("@DeviceName", irp.header.DeviceName);
                        cmd.Parameters.AddWithValue("@ProcessName", irp.header.ProcessName);
                        cmd.Parameters.AddWithValue("@InputBuffer", irp.body.InputBuffer);
                        cmd.Parameters.AddWithValue("@OutputBuffer", irp.body.OutputBuffer);
                        cmd.ExecuteReader();
                    }
                    tx.Commit();
                }
                catch (Exception)
                {
                    tx.Rollback();
                }
                db.Close();
            }

            return file;
        }


        public async Task<bool> LoadIrpsFromFile(StorageFile file)
        {
            var f = await file.CopyAsync(ApplicationData.Current.TemporaryFolder, file.Name, NameCollisionOption.ReplaceExisting);

            using (var db = new SqliteConnection($"Filename={f.Path}"))
            {
                db.Open();

                var cmd = new SqliteCommand(@"
                    SELECT 
                    TimeStamp, 
                    IrqLevel,
                    Type,
                    IoctlCode,
                    Status,
                    ProcessId,
                    ThreadId,
                    InputBufferLength,
                    OutputBufferLength,
                    DriverName,
                    DeviceName,
                    ProcessName,
                    InputBuffer,
                    OutputBuffer  
                    FROM Irps", db);
                var query = cmd.ExecuteReader();

                while (query.Read())
                {
                    Irp irp = new Irp();
                    irp.header.TimeStamp = (ulong)query.GetInt64(0);
                    irp.header.IrqLevel = (uint)query.GetInt32(1);
                    irp.header.Type = (uint)query.GetInt32(2);
                    irp.header.IoctlCode = (uint)query.GetInt32(3);
                    irp.header.Status = (uint)query.GetInt32(4);
                    irp.header.ProcessId = (uint)query.GetInt32(5);
                    irp.header.ThreadId = (uint)query.GetInt32(6);
                    irp.header.InputBufferLength = (uint)query.GetInt32(7);
                    irp.header.OutputBufferLength = (uint)query.GetInt32(8);
                    irp.header.DriverName = query.GetString(9);
                    irp.header.DeviceName = query.GetString(10);
                    irp.header.ProcessName = query.GetString(11);

                    irp.body.InputBuffer = ((MemoryStream)query.GetStream(12)).ToArray();
                    irp.body.OutputBuffer = ((MemoryStream)query.GetStream(13)).ToArray();

                    await App.Irps.Insert(irp);
                }

                App.ViewModel.UpdateUi();

                db.Close();
            }

            await f.DeleteAsync();

            return true;
        }
    }
}
