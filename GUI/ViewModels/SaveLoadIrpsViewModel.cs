using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Data.Sqlite;
using Windows.Storage;


namespace GUI.ViewModels
{
    public class SaveLoadIrpsViewModel : BindableBase
    {

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



        public async Task<StorageFile> DumpIrpsToFile(string filename)
        {
            var dirname = ApplicationData.Current.LocalFolder.Path;

            var file = await ApplicationData.Current.LocalFolder.CreateFileAsync(filename, CreationCollisionOption.OpenIfExists);
            string dbpath = Path.Combine(dirname, filename);

            using (var db = new SqliteConnection($"Filename={dbpath}"))
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
                        var query = @"INSERT INTO Irps VALUES(
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

    }
}
