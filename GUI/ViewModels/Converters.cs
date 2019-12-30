using GUI.Models;
using Windows.UI.Xaml;

namespace GUI.ViewModels
{
    /// <summary>
    /// Bunch of View converter helpers to improve the views
    /// </summary>
    public static class Converters
    {
        /// <summary>
        /// Returns the reverse of the provided value.
        /// </summary>
        public static bool Not(bool value) => !value;

        /// <summary>
        /// Returns true if the specified value is not null; otherwise, returns false.
        /// </summary>
        public static bool IsNotNull(object value) => value != null;

        /// <summary>
        /// Returns Visibility.Collapsed if the specified value is true; otherwise, returns Visibility.Visible.
        /// </summary>
        public static Visibility BooleanToVisibility(bool value) =>
            value ? Visibility.Visible : Visibility.Collapsed;

        public static bool OnlyForDeviceIoControlIrp(IrpViewModel irp) =>
            irp != null && irp.Model.header.Type == (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL;

        public static Visibility VisibleOnlyForDeviceIoControlIrp(IrpViewModel irp) =>
            irp != null && irp.Model.header.Type == (uint)IrpMajorType.IRP_MJ_DEVICE_CONTROL ? Visibility.Visible : Visibility.Collapsed;

        /// <summary>
        /// Returns Visibility.Collapsed if the specified value is true; otherwise, returns Visibility.Visible.
        /// </summary>
        public static Visibility CollapsedIf(bool value) =>
            value ? Visibility.Collapsed : Visibility.Visible;

        /// <summary>
        /// Returns Visibility.Collapsed if the specified value is true; otherwise, returns Visibility.Visible.
        /// </summary>
        public static Visibility VisibleIfGreaterThanZero(uint value) =>
            value > 0 ? Visibility.Visible : Visibility.Collapsed ;

        /// <summary>
        /// Returns Visibility.Collapsed if the specified value is null; otherwise, returns Visibility.Visible.
        /// </summary>
        public static Visibility CollapsedIfNull(object value) =>
            value == null ? Visibility.Collapsed : Visibility.Visible;

        /// <summary>
        /// Returns Visibility.Collapsed if the specified string is null or empty; otherwise, returns Visibility.Visible.
        /// </summary>
        public static Visibility CollapsedIfNullOrEmpty(string value) =>
            string.IsNullOrEmpty(value) ? Visibility.Collapsed : Visibility.Visible;

        /// <summary>
        /// Display an address as hexa string
        /// </summary>
        /// <param name="Address"></param>
        /// <returns></returns>
        public static string FormatAddressAsHex(ulong Address) =>
            $"0x{Address.ToString("X")}";

    }
}
