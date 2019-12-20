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
        private const string IrpDumperTaskClass = "Tasks.IrpDumperBackgroundTask";

        private const string IrpDumperPollDelayKey = "BackgroundTaskPollDelay";


        public bool Enabled
        {
            get => _enabled;
            set
            {
                bool success = false;
                if (value)
                    success = StartFetcher();
                else
                    success = StopFetcher();

                if (success)
                    _enabled = value;
            }
        }


        private bool StartFetcher()
        {
            if(ApplicationData.Current.LocalSettings.Values[IrpDumperPollDelayKey] == null)
                ApplicationData.Current.LocalSettings.Values[IrpDumperPollDelayKey] = _probe_new_data_delay.ToString();

            _task = RegisterBackgroundTask(IrpDumperTaskClass, IrpDumperTaskName, _trigger, null);
            _task.Progress += new BackgroundTaskProgressEventHandler(OnProgress);
            _task.Completed += new BackgroundTaskCompletedEventHandler(OnCompleted);

            return true;
        }


        private bool StopFetcher()
        {
            UnregisterBackgroundTask();
            return true;
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


        private static BackgroundTaskRegistration RegisterBackgroundTask(String clsname, string taskname, IBackgroundTrigger trigger, IBackgroundCondition condition)
        {
            var requestTask = BackgroundExecutionManager.RequestAccessAsync();

            var builder = new BackgroundTaskBuilder();
            builder.Name = taskname;
            builder.TaskEntryPoint = clsname;
            builder.SetTrigger(trigger);

            if (condition != null)
            {
                builder.AddCondition(condition);
                builder.CancelOnConditionLoss = true;
            }

            BackgroundTaskRegistration task = builder.Register();
            ApplicationData.Current.LocalSettings.Values.Remove(taskname);
            ApplicationData.Current.LocalSettings.Values.Remove(IrpDumperPollDelayKey);
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
