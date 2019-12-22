using GUI.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.ApplicationModel.Background;
using Windows.Storage;
using Windows.System.Threading;


namespace GUI.Tasks
{
    public sealed class IrpDumperBackgroundTask : IBackgroundTask
    {
        BackgroundTaskCancellationReason _cancelReason = BackgroundTaskCancellationReason.Abort;
        volatile bool _cancelRequested = false;
        string _cancelReasonExtra = "";
        BackgroundTaskDeferral _deferral = null;
        ThreadPoolTimer _periodicTimer = null;
        //ulong _progress = 0;
        IBackgroundTaskInstance _taskInstance = null;


        //
        // This is where the task entrypoint lands
        //
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            Debug.WriteLine($"Starting background instance '{taskInstance.Task.Name}'...");

            var cost = BackgroundWorkCost.CurrentBackgroundWorkCost;
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["BackgroundWorkCost"] = cost.ToString();

            var delay = (double)ApplicationData.Current.LocalSettings.Values["BackgroundTaskPollDelay"];
            taskInstance.Canceled += new BackgroundTaskCanceledEventHandler(OnCanceled);

            _deferral = taskInstance.GetDeferral();
            _taskInstance = taskInstance;
            _periodicTimer = ThreadPoolTimer.CreatePeriodicTimer(
                new TimerElapsedHandler(PeriodicTimerCallback), 
                TimeSpan.FromSeconds(delay)
            );
        }


        //
        // Handles background task cancellation.
        //
        private void OnCanceled(IBackgroundTaskInstance sender, BackgroundTaskCancellationReason reason)
        {
            _cancelRequested = true;
            _cancelReason = reason;
            Debug.WriteLine($"Canceled background instance '{sender.Task.Name}'");
        }


        //
        // Periodic callback for the task: this is where we actually do the job of fetching new IRPs
        //
        private void PeriodicTimerCallback(ThreadPoolTimer timer)
        {
            // is there a pending cancellation request
            if(_cancelRequested)
            {
                _periodicTimer.Cancel();
                var msg = $"Cancelling background task {_taskInstance.Task.Name }, reason: {_cancelReason.ToString()}";
                if (_cancelReasonExtra.Length > 0)
                    msg += _cancelReasonExtra;
                Debug.WriteLine(msg);
                _deferral.Complete();
                return;
            }

            try 
            {
                //
                // collect the irps from the broker
                //
                List<Irp> NewIrps = FetchIrps();
                _taskInstance.Progress += (uint)NewIrps.Count;

                Debug.WriteLine($"Received {NewIrps.Count:d} new irps");

                //
                // push them to the db
                //
            }
            catch (Exception e)
            {
                _cancelRequested = true;
                _cancelReason = BackgroundTaskCancellationReason.ConditionLoss;
                _cancelReasonExtra = e.Message;
            }
        }


        //
        // Fetch new IRPs
        //
        private List<Irp> FetchIrps()
        {
            var msg = Task.Run( () => App.BrokerSession.GetInterceptedIrps()).Result;

            if (msg.header.is_success)
                return msg.body.irps;

            throw new Exception($"GetInterceptedIrps() request returned FALSE, GLE=0x{msg.header.gle}");
        }
    }
}
