/*
***************************************************************************
**
** Module: Win32API.cs
**
** Description:
**  This will contain various Win32 apis that we need to directly access
**
** Revision History:
** ------------------------------------------------------------------------
** Date        Name         Reason
** 7/29/2005    HGI          Initial Creation
**
**
**
*/
using System;
using System.Drawing;
using System.Runtime.InteropServices;


namespace Win32API
{
    /*
    ***************************************************************************
    **
    ** Class: RTC
    **
    */
    ///<summary>
    /// Rich Text Control Additions
    ///</summary>
    ///

    /*
    ***************************************************************************
    **
    ** Class: Keyboard
    **
    */
    ///<summary>
    /// This is the Keyboard stuff
    ///</summary>
    ///
    public class Keyboard
    {
        ///<summary>Usefull virtual keys to use</summary>
        public enum VirtualKeys
        {
            LSHIFT          = 0xA0,
            RSHIFT          = 0xA1,
            LCONTROL        = 0xA2,
            RCONTROL        = 0xA3,
            LMENU           = 0xA4,
            RMENU           = 0xA5,
            F1              = 0x70,
            F2              = 0x71,
            F3              = 0x72,
            F4              = 0x73,
            F5              = 0x74,
            F6              = 0x75,
            F7              = 0x76,
            F8              = 0x77,
            F9              = 0x78,
            F10             = 0x79,
            F11             = 0x7A,
            F12             = 0x7B,
            KEY_F           = 'F',

        }


        /*
        ***************************************************************************
        **
        ** Function: GetKeyState
        */
        ///<summary>
        ///Get the state of a particular key (pressed etc..)
        ///</summary>
        ///<param name="keyCode" type="int">Keycode to check</param>
        ///<returns>short</returns>
        ///<remarks>
        /// When the Hi-order bit of the return is set then the key is pressed
        /// When the Lo-order bit of the return is set, it means the key is toggled (CAPSLOCK in on)
        [DllImport("user32.dll", CharSet=CharSet.Auto, ExactSpelling=true, CallingConvention=CallingConvention.Winapi)]
        public static extern ushort GetKeyState(int keyCode);

        /*
        ***************************************************************************
        **
        ** Property(bool): ShiftPressed
        */
        ///<summary>
        /// Return true if either shift key is pressed
        ///</summary>
        ///<value>
        ///
        ///</value>
        ///
        public static bool ShiftPressed
        {
            get
            {
                ushort sStateL    = 0;
                ushort sStateR    = 0;

                sStateL = GetKeyState((int)VirtualKeys.LSHIFT);
                sStateR = GetKeyState((int)VirtualKeys.RSHIFT);

                return ((0 != (sStateL & 0x8000)) || (0 != (sStateR & 0x8000)));
            }
        }

        /*
        ***************************************************************************
        **
        ** Property(bool): CtrlPressed
        */
        ///<summary>
        /// See if a Control key is pressed
        ///</summary>
        ///<value>
        ///
        ///</value>
        ///
        public static bool CtrlPressed
        {
            get
            {
                ushort sStateL    = 0;
                ushort sStateR    = 0;

                sStateL = GetKeyState((int)VirtualKeys.LCONTROL);
                sStateR = GetKeyState((int)VirtualKeys.RCONTROL);

                return ((0 != (sStateL & 0x8000)) || (0 != (sStateR & 0x8000)));
            }
        }

    }

    /*
    ***************************************************************************
    **
    ** Class: Window
    **
    */
    ///<summary>
    /// More window APIS
    ///</summary>
    ///
    public class Window
    {
        ///<summary>Window Messages, added as requried</summary>
        public enum Msgs
        {
            WM_KEYDOWN  = 0x100,
        }

        /*
        ***************************************************************************
        **
        ** Function: SendMessage
        */
        ///<summary>
        /// Send Message To a window
        ///</summary>
        ///<param name="hWnd" type="System.Runtime.InteropServices.HandleRef">Window Handle to Send Message To</param>
        ///<param name="msg" type="int">Message Code</param>
        ///<param name="wParam" type="int">WParameter to the Message</param>
        ///<param name="lParam" type="int">lParameter to the Message</param>
        ///<returns>System.IntPtr</returns>
        ///
        [DllImport("user32")]
        public static extern IntPtr SendMessage(HandleRef hWnd, int msg, int wParam, int lParam);

        /*
        ***************************************************************************
        **
        ** Function: GetCaretPos
        */
        ///<summary>
        ///Get the Current Caret Position in the box
        ///</summary>
        ///<param name="lpPoint" type="ref System.Drawing.Point">Point that is the position</param>
        ///<returns>int</returns>
        ///
        [DllImport("user32")]
        public static extern int GetCaretPos(ref Point lpPoint);

    }
}