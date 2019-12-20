using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.Core;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;


namespace GUI.Models
{
    /// <summary>
    /// This class operates in background and manages the IRP fetching operations.
    /// Upon receiving new IRPs it'll push them to the IrpRepository.
    /// </summary>
    public class IrpDumper
    {
        private double _probe_new_data_delay = 0.5; // seconds
        private bool _enabled = false;
        private ApplicationTrigger _trigger = null;
        private BackgroundTaskRegistration _task;

        private const string IrpDumperTaskName = "IrpDumperBackgroundTask";
        private const string IrpDumperTaskClass = "Tasks.IrpDumperBackgroundClass";


        public bool Enabled
        {
            get => _enabled;
            set
            {
                _enabled = value;
                if (_enabled)
                    StartFetcher();
                else
                    StopFetcher();
            }
        }


        private void StartFetcher()
        {
            _task = RegisterBackgroundTask(IrpDumperTaskClass, IrpDumperTaskName, _trigger, null);
            _task.Progress += new BackgroundTaskProgressEventHandler(OnProgress);
            _task.Completed += new BackgroundTaskCompletedEventHandler(OnCompleted);
            ApplicationData.Current.LocalSettings.Values.Add("BackgroundTaskPollDelay", _probe_new_data_delay.ToString());
        }


        private void StopFetcher()
        {
            UnregisterBackgroundTask();
        }



        private void OnProgress(IBackgroundTaskRegistration task, BackgroundTaskProgressEventArgs args)
        {
            //var ignored = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            //{
            //    var progress = "Progress: " + args.Progress + "%";
            //    BackgroundTaskSample.ServicingCompleteTaskProgress = progress;
            //});
        }


        private void OnCompleted(IBackgroundTaskRegistration task, BackgroundTaskCompletedEventArgs args)
        {
        }


        public IrpDumper()
        {
            _trigger = new ApplicationTrigger();
        }


        private static bool TaskRequiresBackgroundAccess(String name)
            => true;


        private static BackgroundTaskRegistration RegisterBackgroundTask(String irpDumperTaskclass, string irpDumperTaskName, IBackgroundTrigger trigger, IBackgroundCondition condition)
        {
            var requestTask = BackgroundExecutionManager.RequestAccessAsync();

            var builder = new BackgroundTaskBuilder();
            builder.Name = IrpDumperTaskName;
            builder.TaskEntryPoint = irpDumperTaskclass;
            builder.SetTrigger(trigger);

            if (condition != null)
            {
                builder.AddCondition(condition);
                builder.CancelOnConditionLoss = true;
            }

            BackgroundTaskRegistration task = builder.Register();
            ApplicationData.Current.LocalSettings.Values.Remove(IrpDumperTaskName);
            return task;
        }


        private static bool UnregisterBackgroundTask()
        {
            foreach (var cur in BackgroundTaskRegistration.AllTasks)
            {
                if (cur.Value.Name == IrpDumperTaskName)
                {
                    cur.Value.Unregister(true);
                    return true;
                }
            }
            return false;
        }
    }
}
