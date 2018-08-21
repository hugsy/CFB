/*
***************************************************************************
**
** Module: HexEdit.cs
**
** Description:
** CopyRight INCO - Howard Glenn Inman
**
** Revision History:
** ------------------------------------------------------------------------
** Date        Name         Reason
** 6/24/2005    HGI          Initial Creation
**
**
**
*/
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using System.Data;
using System.Runtime.InteropServices;
using Win32API;

namespace HexEdit
{
    /*
    ***************************************************************************
    **
    ** Class: HexEditBase
    **
    */
    ///<summary>
    /// Base class for the Hex Edit Pair of boxes
    ///</summary>
    ///<remarks>
    ///</remarks>
    ///
    public class HexEditBase : RichTextBox
    {
        /*
        ** Local Variables
        */
        ///<summary>Flag to prevent linked update</summary>
        protected bool              m_bNoUpdate         = false;  // Control the Updates between windows

        ///<summary>The Controls Contect Menu</summary>
        protected ContextMenu       m_menuContext       = null;

        /*
        ** Constants
        */
        ///<summary>Constants for Line Index - SDK Call</summary>
        protected const int EM_LINEINDEX        = 0xbb;

        ///<summary>Constants for Line From Char - SDK Call</summary>
        protected const int EM_LINEFROMCHAR     = 0xc9;

        ///<summary>Constants for Get Selection - SDK Call</summary>
        protected const int EM_GETSEL           = 0xb0;

        /*
        ** Definitions from the Outer World
        */

        /*
        ***************************************************************************
        **
        ** Class: CharPosition
        **
        */
        ///<summary>
        /// A class the contains the line and character position in the bxo
        ///</summary>
        ///<remarks>
        ///</remarks>
        ///
        public class CharPosition
        {
            /*
            ** Local Variables
            */
            ///<summary>Current Line Position</summary>
            protected int m_iLine   = 0;

            ///<summary>Current Character Position</summary>
            protected int m_iChar   = 0;

            /*
            ***************************************************************************
            **
            ** Function: CharPosition
            */
            ///<summary>
            /// Default empty constructor
            ///</summary>
            ///<returns>void</returns>
            ///
            public CharPosition()
            {
            }

            /*
            ***************************************************************************
            **
            ** Function: CharPosition
            */
            ///<summary>
            /// Constructor that sets the Line and Char position
            ///</summary>
            ///<param name="iLine" type="int">Line position to create with</param>
            ///<param name="iChar" type="int">Character position to create with</param>
            ///<returns>void</returns>
            ///
            public CharPosition(int iLine, int iChar)
            {
                LinePos = iLine;
                CharPos = iChar;
            }

            /*
            ***************************************************************************
            **
            ** Property(int): LinePos
            */
            ///<summary>
            /// Line Position
            ///</summary>
            ///<value>
            ///
            ///</value>
            ///
            public int LinePos
            {
                get{return m_iLine;}
                set{m_iLine = value;}
            }

            /*
            ***************************************************************************
            **
            ** Property(int): CharPos
            */
            ///<summary>
            /// Character Position
            ///</summary>
            ///<value>
            ///
            ///</value>
            ///
            public int CharPos
            {
                get{return m_iChar;}
                set{m_iChar = value;}
            }

            /*
            ***************************************************************************
            **
            ** Function: ToString
            */
            ///<summary>
            /// Convert the Object to a string
            ///</summary>
            ///<returns>string</returns>
            ///
            public override string ToString()
            {
                return(String.Format("{{L={0}, C={1}}}", LinePos, CharPos));
            }
        }

        /*
        ***************************************************************************
        **
        ** Property(System.Drawing.Point): CaretPosition
        */
        ///<summary>
        /// Get the Caret Position
        ///</summary>
        ///<value>
        ///
        ///</value>
        ///
        public Point CaretPosition
        {
            get
            {
                Point pt = Point.Empty;
                Win32API.Window.GetCaretPos(ref pt);
                return pt;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: LineIndex
        */
        ///<summary>
        ///The return value is the number of characters that
        ///precede the first character in the line containing
        ///the caret.
        ///
        ///</summary>
        ///<param name="iLine" type="int">line to get the Caracters to</param>
        ///<returns>int - Number of characters to the beginning of iLine</returns>
        ///
        public int LineIndex(int iLine)
        {
            return (int)Win32API.Window.SendMessage(new HandleRef(this, Handle), EM_LINEINDEX, iLine, 0);
        }

        /*
        ***************************************************************************
        **
        ** Function: LineIndex
        */
        ///<summary>
        ///Send the EM_LINEINDEX message with the value of -1
        ///in wParam.
        ///
        ///</summary>
        ///<returns>int</returns>
        ///
        public int LineIndex()
        {
            return LineIndex(-1);
        }

        /*
        ***************************************************************************
        **
        ** Property(HexEdit.HexEditBase.CharPosition): Position
        */
        ///<summary>
        ///Get the Line Char positions in the buffer
        ///</summary>
        ///<value>
        ///
        ///</value>
        ///
        public CharPosition Position
        {
            get
            {
                CharPosition cp = new CharPosition();

                cp.LinePos = GetLineFromCharIndex(SelectionStart);
                cp.CharPos = SelectionStart - LineIndex();

                return cp;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: GetDisplayChar
        */
        ///<summary>
        ///Get the Display char from a entered char
        ///this prevents non displayable characters from
        ///going to the display
        ///</summary>
        ///<param name="cData" type="char">character to check</param>
        ///<returns>char</returns>
        ///
        protected char GetDisplayChar(char cData)
        {
            if(20 > cData)
            {
                cData = (char)0xB7;
            }

            return cData;
        }

        /*
        ***************************************************************************
        **
        ** Function: GetDisplayChar
        */
        ///<summary>
        ///<see cref="GetDisplayChar"/>Converts to byte to char before doing the
        ///display filtering
        ///</summary>
        ///<param name="byData" type="byte"></param>
        ///<returns>char</returns>
        ///
        protected char GetDisplayChar(byte byData)
        {
            return GetDisplayChar((char)byData);
        }

        /*
        ***************************************************************************
        **
        ** Function: GetFontWidth
        */
        ///<summary>
        /// Return the Font Width, assumes fixed font
        ///</summary>
        ///<returns>int - the width of the font</returns>
        ///
        protected int GetFontWidth()
        {
            Graphics    g       = CreateGraphics();
            int         iWidth  = 0;
            string      strTest = "WWWWWWWWWWWWWWWW";
            SizeF       Size;

            Size = g.MeasureString(strTest, Font);
            iWidth = (int)(Size.Width + .09) / strTest.Length;

            return iWidth;
        }

        /*
        ***************************************************************************
        **
        ** Function: SetTextTabLocations
        */
        ///<summary>
        ///
        ///</summary>
        ///<returns>void</returns>
        ///<exception cref="System.Exception">Thrown</exception>
        ///<remarks>
        ///</remarks>
        ///<example>How to use this function
        ///<code>
        ///</code>
        ///</example>
        ///
        public void SetTextTabLocations()
        {
            /*
            ** Local Variables
            */
            int     iTabSize    = 4;
            int     iNoTabs     = 32;
            int[]   aiTabs      = new int[iNoTabs];
            int     iWidth      = GetFontWidth();

            for(int i = 0; i < iNoTabs; i++)
            {
                aiTabs[i] = iWidth * ((i + 1) * iTabSize);
            }

            SelectionTabs = aiTabs;
        }


        /*
        ***************************************************************************
        **
        ** Function: HexCtoB
        */
        ///<summary>
        /// Convert a character to its Byte Equivalent
        ///</summary>
        ///<param name="cVar" type="char"></param>
        ///<returns>byte</returns>
        ///<example>char = A (0x41) returns a byte of A (0xA)
        ///</example>
        ///
        protected byte HexCtoB(char cVar)
        {
            byte    byRet   = 0;

            if((cVar >= '0') && (cVar <= '9'))
            {
                byRet = (byte)(cVar - '0');
            }
            else
            {
                byRet = (byte)((cVar - 'A') + 10);
            }

            return byRet;
        }

        /*
        ***************************************************************************
        **
        ** Function: InitializeComponent
        */
        ///<summary>
        /// This is a overridable function to process any initialization stuff the box
        /// might need
        ///</summary>
        ///<returns>void</returns>
        ///
        virtual public void InitializeComponent()
        {
        }
    }

    /*
    ***************************************************************************
    **
    ** Class: HexEditBox
    **
    */
    ///<summary>
    ///This is the Hex Edit Box Display
    ///</summary>
    ///<remarks>
    ///</remarks>
    ///
    public class HexEditBox : HexEditBase
    {
        /*
        ** Class Local Variables
        */
        ///<summary>used for backup, so the position alignment will not keep if from moving</summary>
        protected bool      m_bIgnorePart       = false;

        ///<summary>This is the list of characters allowed in the hex window</summary>
        protected string    m_strAllowed        = "1234567890ABCDEF";

        ///<summary>This is the array where the data is stored</summary>
        protected byte[]    m_abyData           = null;

        ///<summary>ensures we dont get a Selection Change while we are processing one already</summary>
        protected bool      m_bSelChangeProcess = false;

        ///<summary>This is a reference to the linked display (if there is one)</summary>
        protected LinkedBox m_rtbLink           = null;

        ///<summary>This is our menuItem for copy</summary>
        protected MenuItem  m_miSelectAll       = null;

        ///<summary>This is our menuItem for copy</summary>
        protected MenuItem  m_miCopy            = null;

        ///<summary>This is our menuItem for Paste</summary>
        protected MenuItem  m_miPasteASCII      = null;

        ///<summary>This is our menuItem for Paste</summary>
        protected MenuItem  m_miPasteBytes      = null;

        ///<summary>This is our menuItem for copy to ASCII</summary>
        protected MenuItem  m_miCopyASCII       = null;

        ///<summary>This is our menuItem for copy bytes</summary>
        protected MenuItem  m_miCopyBytes       = null;


        /*
        ***************************************************************************
        **
        ** Function: InitializeComponent
        */
        ///<summary>
        /// This is the custom initialization we set our menu and font here
        ///</summary>
        ///<returns>void</returns>
        ///<exception cref="System.Exception">Thrown</exception>
        ///<remarks>
        ///</remarks>
        ///<example>How to use this function
        ///<code>
        ///</code>
        ///</example>
        ///
        override public void InitializeComponent()
        {
            /*
            ** Create the Context menu
            */
            m_menuContext   = new ContextMenu();
            m_miCopy        = new MenuItem();
            m_miPasteASCII  = new MenuItem();
            m_miPasteBytes  = new MenuItem();
            m_miCopyASCII   = new MenuItem();
            m_miCopyBytes   = new MenuItem();
            m_miSelectAll   = new MenuItem();

            /*
            ** Add the Pop Message
            */
            m_menuContext.Popup += new System.EventHandler(MenuPopup);

            /*
            ** miSelectAll
            **
            */
            m_miSelectAll.Text = "Select All";
            m_miSelectAll.Click += new System.EventHandler(Menu_SelectAll);

            /*
            ** miCopyBytes
            **
            */
            m_miCopyBytes.Text = "Copy Data Bytes";
            m_miCopyBytes.Click += new System.EventHandler(Menu_CopyBytes);

            /*
            ** miCopyASCII
            **
            */
            m_miCopyASCII.Text = "Copy Bytes to ASCII";
            m_miCopyASCII.Click += new System.EventHandler(Menu_CopyASCII);

            /*
            ** miCopy
            **
            */
            m_miCopy.Text = "Copy Hex as ASCII";
            m_miCopy.Click += new System.EventHandler(Menu_Copy);

            /*
            ** miPasteASCII
            **
            */
            m_miPasteASCII.Text = "Paste ASCII";
            m_miPasteASCII.Click += new System.EventHandler(Menu_PasteASCII);

            /*
            ** miPasteBytes
            **
            */
            m_miPasteBytes.Text = "Paste Bytes";
            m_miPasteBytes.Click += new System.EventHandler(Menu_PasteBytes);

            //
            // rtbHex
            //
            this.AcceptsTab = true;
            this.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.Name = "rtbHex";
            this.TabIndex = 1;
            this.ContextMenu = m_menuContext;
            this.Text = "";
            this.WordWrap = false;
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnKeyDown);
            this.SelectionChanged += new System.EventHandler(this.OnSelectionChange);
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.OnKeyPress);
            this.Resize += new System.EventHandler(this.OnResizeBox);

        }


        /*
        ***************************************************************************
        **
        ** Function: MenuPopup
        */
        ///<summary>
        ///
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void MenuPopup(object sender, System.EventArgs e)
        {
            /*
            ** Local Variables
            */
            IDataObject DataObj = Clipboard.GetDataObject();
            int         iIndex  = 0;

            /*
            ** Clear the Items
            */
            m_menuContext.MenuItems.Clear();

            /*
            ** Add Items to the Context Menu
            */
            m_menuContext.MenuItems.Add(iIndex++, m_miSelectAll);

            /*
            ** Add in the Copy Stuff is there is a Selection
            */
            if(SelectionLength > 0)
            {
                m_menuContext.MenuItems.Add(iIndex++, new MenuItem("-"));
                m_menuContext.MenuItems.Add(iIndex++, m_miCopyBytes);
                m_menuContext.MenuItems.Add(iIndex++, m_miCopyASCII);
                m_menuContext.MenuItems.Add(iIndex++, m_miCopy);
            }

            /*
            ** Determines whether the data is in a format you can use.
            */
            if(DataObj.GetDataPresent(m_abyData.GetType()))
            {
                m_menuContext.MenuItems.Add(iIndex++, new MenuItem("-"));
                m_menuContext.MenuItems.Add(iIndex++, m_miPasteBytes);
                if(DataObj.GetDataPresent(DataFormats.Text))
                {
                    m_menuContext.MenuItems.Add(iIndex++, m_miPasteASCII);
                }
            }
            else
            {
                if(DataObj.GetDataPresent(DataFormats.Text))
                {
                    m_menuContext.MenuItems.Add(iIndex++, new MenuItem("-"));
                    m_menuContext.MenuItems.Add(iIndex++, m_miPasteASCII);
                }
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_SelectAll
        */
        ///<summary>
        ///Select all
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_SelectAll(object sender, System.EventArgs e)
        {
            SelectAll();
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_Copy
        */
        ///<summary>
        /// Process the Copy Menu Item
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_Copy(object sender, System.EventArgs e)
        {
            Copy();
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_CopyASCII
        */
        ///<summary>
        ///
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_CopyASCII(object sender, System.EventArgs e)
        {
            /*
            ** Local Variables
            */
            int             iStart  = (SelectionStart / 3);
            int             iLen    = (SelectionLength / 3);
            StringBuilder   sbVar   = new StringBuilder(iLen);

            /*
            ** Make sure our size is only as much as we have
            */
            if(iLen > m_abyData.Length)
            {
                iLen = m_abyData.Length;
            }

            for(int i = iStart; i < iLen; i++)
            {
                char    cData = GetDisplayChar(m_abyData[i]);
                sbVar.Append(cData);
            }

            /*
            ** Copy the bitmap to the clipboard.
            */
            Clipboard.SetDataObject(sbVar.ToString());
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_CopyBytes
        */
        ///<summary>
        /// Process the Menu Copy Bytes (Converts the hex to Asc in the Hex window)
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_CopyBytes(object sender, System.EventArgs e)
        {
            /*
            ** Local Variables
            */
            int         iStart  = (SelectionStart / 3);
            int         iLen    = (SelectionLength / 3);
            byte[]      abyCopy = new byte[iLen];

            /*
            ** Make sure our size is only as much as we have
            */
            if(iLen > m_abyData.Length)
            {
                iLen = m_abyData.Length;
            }

            /*
            ** Get the Bytes we want to mess with
            */
            Array.Copy(m_abyData, iStart, abyCopy, 0, iLen);

            /*
            ** Copy the bitmap to the clipboard.
            */
            Clipboard.SetDataObject(abyCopy);
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_PasteASCII
        */
        ///<summary>
        /// Processes the paste ASCII bytes to the hex winedow
        ///</summary>
        ///<remarks>
        ///This is marked internal so that the linked window can call it
        ///</remarks>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        internal void Menu_PasteASCII(object sender, System.EventArgs e)
        {
            /*
            ** Save our starting position to restore after we work
            */
            int     iSave = SelectionStart;

            /*
            ** Declares an IDataObject to hold the data returned from the clipboard.
            ** Retrieves the data from the clipboard.
            */
            IDataObject DataObj = Clipboard.GetDataObject();

            /*
            ** Determines whether the data is in a format you can use.
            */
            if(DataObj.GetDataPresent(DataFormats.Text))
            {
                /*
                ** Local Variables
                */
                string          strText = (string)DataObj.GetData(DataFormats.Text);
                int             iPos    = (SelectionStart / 3);
                int             iLen    = (strText.Length < (m_abyData.Length - iPos)) ? strText.Length : (m_abyData.Length - iPos);

                /*
                ** Yes it is, so display it in a text box.
                */
                for(int i = 0; i < iLen; i++)
                {
                    m_abyData[i + iPos] = (byte)strText[i];
                }

                UpdateDisplay();

            }
            else
            {
                MessageBox.Show("Nothing to paste");
            }

            /*
            ** Restore our original position
            */
            SelectionStart = iSave;
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_PasteBytes
        */
        ///<summary>
        ///
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        internal void Menu_PasteBytes(object sender, System.EventArgs e)
        {
            /*
            ** Save our starting position to restore after we work
            */
            int     iSave = SelectionStart;

            /*
            ** Declares an IDataObject to hold the data returned from the clipboard.
            ** Retrieves the data from the clipboard.
            */
            IDataObject DataObj = Clipboard.GetDataObject();

            /*
            ** This is a Byte Paste
            */
            if(DataObj.GetDataPresent(m_abyData.GetType()))
            {
                /*
                ** Local Variables
                */
                byte[]          abyCopy = (byte[])DataObj.GetData(m_abyData.GetType());
                int             iPos    = (SelectionStart / 3);
                int             iLen    = (abyCopy.Length < (m_abyData.Length - iPos)) ? abyCopy.Length : (m_abyData.Length - iPos);

                /*
                ** Yes it is, so display it in a text box.
                */
                for(int i = 0; i < iLen; i++)
                {
                    m_abyData[i + iPos] = abyCopy[i];
                }

                UpdateDisplay();
            }
            else
            {
                MessageBox.Show("Nothing to paste");
            }

            /*
            ** Restore our original position
            */
            SelectionStart = iSave;
        }

        /*
        ***************************************************************************
        **
        ** Function: LinkDisplay
        */
        ///<summary>
        /// This allows you to link a 'ASCII Display' to the Hex Window
        /// So you can see both
        ///</summary>
        ///<param name="rtbLink" type="HexEdit.LinkedBox">null to unlink else a LinkedBox</param>
        ///<returns>void</returns>
        ///
        public void LinkDisplay(LinkedBox rtbLink)
        {
            m_rtbLink = rtbLink;
            m_rtbLink.LinkHex(this);
        }

        /*
        ***************************************************************************
        **
        ** Function: GetDisplayForByte
        */
        ///<summary>
        /// Get the Display for a Data Byte
        ///</summary>
        ///<param name="byData" type="byte"></param>
        ///<example>byte = 0x42 = string = "42"
        ///</example>
        ///
        string GetDisplayForByte(byte byData)
        {
            return string.Format("{0:X2}", byData);
        }

        /*
        ***************************************************************************
        **
        ** Function: LoadData
        */
        ///<summary>
        /// Load data into the Hex Box
        ///</summary>
        ///<param name="abyData" type="byte[]"></param>
        ///<returns>void</returns>
        ///
        public void LoadData(byte[] abyData)
        {
            m_abyData = new byte[abyData.Length];
            Array.Copy(abyData, 0, m_abyData, 0, abyData.Length);
            UpdateDisplay();
        }

        /*
        ***************************************************************************
        **
        ** Function: NewData
        */
        ///<summary>
        ///Create a New Blank Display
        ///</summary>
        ///<param name="iSize" type="int"></param>
        ///<returns>void</returns>
        ///
        public void NewData(int iSize)
        {
            m_abyData = new byte[iSize];
            UpdateDisplay();
        }

        /*
        ***************************************************************************
        **
        ** Property(byte[]): Array
        */
        ///<summary>
        /// Get the Data that is the current buffer
        ///</summary>
        ///
        public byte[] ArrayData
        {
            get{return m_abyData;}
        }

        /*
        ***************************************************************************
        **
        ** Function: UpdateDisplay
        */
        ///<summary>
        /// This updates the display with the data that is in abyData
        /// Can sync up if data has changed in the buffer due to a paste or
        /// like function
        ///</summary>
        ///
        public void UpdateDisplay()
        {
            /*
            ** Local Variables
            */
            int             iLenData    = m_abyData.Length;
            int             iLenDisp    = 0;
            int             iCols       = 0;
            StringBuilder   sbVar       = null;

            iCols = (this.Width / GetFontWidth());
            iCols = (iCols / 3) - 1;

            /*
            ** Get a copy of the Data so we can mess with it
            ** and also have it aligned where we want it
            */
            iLenDisp = ((iLenData + (iCols - 1)) / iCols) * iCols;
            sbVar = new StringBuilder(iLenDisp);

            for(int i = 0; i < iLenDisp; i++)
            {
                byte        byData  = 0;

                /*
                ** See if we are still in the real buffer range
                ** if so display data, else display spaces
                */
                if(i < iLenData)
                {
                    byData = m_abyData[i];
                    sbVar.Append(GetDisplayForByte(byData));
                }
                else
                {
                    sbVar.Append("  ");
                }

                if(0 != ((i+1) % iCols))
                {
                    sbVar.Append(" ");
                }
                else
                {
                    sbVar.Append("\n");
                }
            }

            /*
            ** Remove the extra last char
            ** and display the data
            */
            sbVar.Remove(sbVar.Length - 1, 1);
            Text = sbVar.ToString();

            if(null != m_rtbLink)
            {
                m_rtbLink.UpdateDisplay(m_abyData);
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: GetByteAtCurrent
        */
        ///<summary>
        /// Gets the Byte at the current screen location in the hex editor
        ///</summary>
        ///<returns>byte</returns>
        ///
        protected byte GetByteAtCurrent()
        {
            int     iPutBack    = SelectionStart;
            int     iStart      = (SelectionStart-1)/3;
            byte    by1         = 0;
            byte    by2         = 0;

            /*
            ** Set the Current Position to where this byte begins
            */
            SelectionStart = iStart * 3;
            by1 = HexCtoB(GetCharFromPosition(CaretPosition));
            SelectionStart += 1;
            by2 = HexCtoB(GetCharFromPosition(CaretPosition));
            SelectionStart = iPutBack;

            by1 = (byte)(by1 << 4);
            by1 |= by2;

            return by1;
        }

        /*
        ***************************************************************************
        **
        ** Function: OnSelectionChange
        */
        ///<summary>
        ///This is our OnSelectionChange Listener
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///<remarks>
        ///here we can get our current position from selection start
        ///and if we are on a boundry (00^) then we skip one
        ///</remarks>
        ///
        protected void OnSelectionChange(object sender, System.EventArgs e)
        {
            if(!m_bSelChangeProcess)
            {
                m_bSelChangeProcess = true;
                int     iPos = ((SelectionStart + 1) % 3);

                if((0 == iPos) && !m_bIgnorePart)
                {
                    iPos      = SelectionStart/3;
                    if(iPos < m_abyData.Length)
                    {
                        m_abyData[iPos] = GetByteAtCurrent();
                        if(null != m_rtbLink)
                        {
                            m_bNoUpdate = true;
                            m_rtbLink.UpdateChar(iPos, (char)m_abyData[iPos]);
                            m_bNoUpdate = false;
                        }
                        SelectionStart += 1;
                    }
                }
                m_bIgnorePart = false;
                m_bSelChangeProcess = false;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: UpdateChar
        */
        ///<summary>
        /// Updates the character at the Position - Used to sync the
        /// LinkedBox and this box
        ///</summary>
        ///<param name="iPosition" type="int">Position of the Data To Update</param>
        ///<param name="cData" type="char">Data to Update with</param>
        ///<returns>void</returns>
        ///
        public void UpdateChar(int iPosition, char cData)
        {
            /*
            ** controled from the call to update to keep
            ** updates from linked window when we update it
            */
            if(!m_bNoUpdate)
            {
                /*
                ** Process any outstanding messages and set the focus to us
                */
                System.Windows.Forms.Application.DoEvents();
                Focus();

                /*
                ** Local Variables
                */
                int     iPos    = (iPosition * 3);
                string  strVar  = null;
                byte    bData   = (byte)cData;
                byte    bHi     = 0;
                byte    bLo     = 0;

                bLo = (byte)(bData & 0xF);
                bHi = (byte)(bData >> 4);

                strVar = string.Format("{0:X}{1:X}", bHi, bLo);
                SelectionStart = iPos;
                SelectionLength = 2;

                /*
                ** Send the keys to this window, and then process
                ** any ourstanding messages
                */
                SendKeys.Send(strVar);
                System.Windows.Forms.Application.DoEvents();
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: OnResizeBox
        */
        ///<summary>
        ///
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void OnResizeBox(object sender, System.EventArgs e)
        {
            if(null != m_abyData)
            {
                UpdateDisplay();
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: OnKeyPress
        */
        ///<summary>
        /// KeyPress Listener Used to allow only the acceptable keys to be entered
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.Windows.Forms.KeyPressEventArgs"></param>
        ///<returns>void</returns>
        ///<exception cref="System.Exception">Thrown</exception>
        ///
        private void OnKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            /*
            ** see if it is in the allowed list
            */
            if(-1 != m_strAllowed.IndexOf(e.KeyChar))
            {
                /*
                ** Are we still in the real data area,
                ** if so process if no done to it
                */
                if(SelectionStart < (m_abyData.Length * 3))
                {
                    /*
                    ** This forces a Replace instead of a insert
                    */
                    SelectionLength = 1;
                }
                else
                {
                    e.Handled = true;
                }
            }
            else
            {
                /*
                ** Local Variables
                */
                string strVar = string.Format("{0}", e.KeyChar).ToUpper();

                /*
                ** Tell the system not to handle this character
                */
                e.Handled = true;

                /*
                ** Ok lets see if the Upper case is in the list
                ** if it is good send the upper equavelent
                */
                if(-1 != m_strAllowed.IndexOf(strVar))
                {
                    SendKeys.Send(strVar);
                }
                else
                {
                    switch(e.KeyChar)
                    {
                        case (char)0x9:
                            if(null != m_rtbLink)
                            {
                                m_rtbLink.Focus();
                                m_rtbLink.SelectionStart = (SelectionStart / 3);
                            }
                        break;
                    }
                }
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: OnKeyDown
        */
        ///<summary>
        /// KeyPress Listener Used to disallow special keys from working
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.Windows.Forms.KeyEventArgs"></param>
        ///<returns>void</returns>
        ///<exception cref="System.Exception">Thrown</exception>
        ///<remarks>
        ///</remarks>
        ///<example>How to use this function
        ///<code>
        ///</code>
        ///</example>
        ///
        protected void OnKeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            e.Handled = true;

            /*
            ** This allows the back arrow to work
            */
            switch(e.KeyCode)
            {
                default:
                    e.Handled = false;
                break;

                case Keys.Left:
                    m_bIgnorePart = true;
                    e.Handled = false;
                break;

                case Keys.Tab:
                break;

                case Keys.Back:
                {
                    int             iPos    = 0;

                    iPos = SelectionStart - 1;
                    if((-1 != iPos) && (iPos < (m_abyData.Length * 3)))
                    {
                        SelectionStart = iPos;
                        m_bIgnorePart = true;
                    }
                }
                break;

                case Keys.Return:
                {
                    CharPosition    cp      = Position;
                    int             iPos    = 0;

                    iPos = LineIndex(cp.LinePos+1);
                    if((-1 != iPos) && (iPos < (m_abyData.Length * 3)))
                    {
                        SelectionStart = iPos;
                    }
                }
                break;

                case Keys.Delete:
                break;

                case Keys.Insert:
                break;
            }

        }
    }

    /*
    ***************************************************************************
    **
    ** Class: LinkedBox
    **
    */
    ///<summary>
    /// This is the Box that links with the hex edit box to show the ASCII
    ///</summary>
    ///<remarks>
    ///</remarks>
    ///
    public class LinkedBox : HexEditBase
    {
        /*
        ** Class Local Variables
        */
        ///<summary>Length of the Data this box should allow</summary>
        protected int           m_iDataLength       = 0;

        ///<summary></summary>
        protected HexEditBox    m_edtHex            = null;

        ///<summary></summary>
        protected MenuItem      m_miCopy            = null;

        ///<summary></summary>
        protected MenuItem      m_miPaste           = null;


        /*
        ***************************************************************************
        **
        ** Function: InitializeComponent
        */
        ///<summary>
        /// The Component specific initialization
        ///</summary>
        ///<returns>void</returns>
        ///
        override public void InitializeComponent()
        {
            /*
            ** Create the Context menu
            */
            m_menuContext = new ContextMenu();
            m_miCopy      = new MenuItem();
            m_miPaste     = new MenuItem();

            /*
            ** miCopy
            **
            */
            m_miCopy.Index = 0;
            m_miCopy.Text = "Copy";
            m_miCopy.Click += new System.EventHandler(Menu_Copy);

            // miPaste
            //
            m_miPaste.Index = 1;
            m_miPaste.Text = "Paste";
            m_miPaste.Click += new System.EventHandler(Menu_Paste);

            /*
            ** Add Items to the Context Menu
            */
            m_menuContext.MenuItems.AddRange(new System.Windows.Forms.MenuItem[]
            {
                m_miCopy,
                m_miPaste
            });

            //
            // The ritchbox
            //
            this.AcceptsTab = true;
            this.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.Name = "rtbHex";
            this.TabIndex = 1;
            this.ContextMenu = m_menuContext;
            this.Text = "";
            this.WordWrap = true;
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.OnKeyPress);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnKeyDown);
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_Copy
        */
        ///<summary>
        /// The Copy Menu Item
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_Copy(object sender, System.EventArgs e)
        {
            Copy();
        }

        /*
        ***************************************************************************
        **
        ** Function: Menu_Paste
        */
        ///<summary>
        /// The Paste Menu Item
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.EventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void Menu_Paste(object sender, System.EventArgs e)
        {
            if(null != m_edtHex)
            {
                /*
                ** Save our starting position
                */
                int iSave = SelectionStart;

                /*
                ** Figure out where we should start in the Hex window
                ** and set its position
                */
                m_edtHex.SelectionStart = (SelectionStart * 3);
                m_edtHex.Menu_PasteASCII(sender, e);

                /*
                ** Put the Data Base
                */
                SelectionStart = iSave;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: LinkHex
        */
        ///<summary>
        /// Link the Hex box with this display
        ///</summary>
        ///<param name="HexEdit" type="HexEdit.HexEditBox"></param>
        ///<returns>void</returns>
        ///
        public void LinkHex(HexEditBox HexEdit)
        {
            m_edtHex = HexEdit;
        }

        /*
        ***************************************************************************
        **
        ** Function: UpdateDisplay
        */
        ///<summary>
        /// Update the Box with the Bytes in abyData
        ///</summary>
        ///<param name="abyData" type="byte[]">Data to Display</param>
        ///<returns>void</returns>
        ///
        public void UpdateDisplay(byte[] abyData)
        {
            m_iDataLength = abyData.Length;

            StringBuilder sbVar = new StringBuilder(m_iDataLength);

            for(int i = 0; i < m_iDataLength; i++)
            {
                char            cData = GetDisplayChar(abyData[i]);
                sbVar.Append(cData);
            }

            /*
            ** Remove the extra last char
            ** and display the data
            */
            Text = sbVar.ToString();
        }

        /*
        ***************************************************************************
        **
        ** Function: UpdateChar
        */
        ///<summary>
        ///Used by the HexBox to update us on character changes
        ///</summary>
        ///<param name="iPosition" type="int">Position of Change</param>
        ///<param name="cData" type="char">Character to Display</param>
        ///<returns>void</returns>
        ///
        public void UpdateChar(int iPosition, char cData)
        {
            /*
            ** controled from the call to update to keep
            ** updates from linked window when we update it
            */
            if(!m_bNoUpdate)
            {
                /*
                ** Local Variables
                */
                string  strVar  = string.Format("{0}", GetDisplayChar(cData));

                SelectionStart = iPosition;
                SelectionLength = 1;
                SelectedText = strVar;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: OnKeyPress
        */
        ///<summary>
        ///Process the Key Presses, to keep in the window area
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.Windows.Forms.KeyPressEventArgs"></param>
        ///<returns>void</returns>
        ///
        private void OnKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            /*
            ** Are we still in the real data area,
            ** if so process if no done to it
            */
            if(SelectionStart < m_iDataLength)
            {
                switch(e.KeyChar)
                {
                    default:
                        /*
                        ** This forces a Replace instead of a insert
                        */
                        SelectionLength = 1;
                        if(null != m_edtHex)
                        {
                            m_bNoUpdate = true;
                            m_edtHex.UpdateChar(SelectionStart, e.KeyChar);
                            m_bNoUpdate = false;

                            /*
                            ** This places the focus back to our window
                            ** and processes anything ourstanding
                            */
                            System.Windows.Forms.Application.DoEvents();
                            Focus();
                        }
                    break;

                    case (char)0xD:
                    case (char)0x8:
                        e.Handled = true;
                    break;

                    case (char)0x9:
                        e.Handled = true;
                        if(null != m_edtHex)
                        {
                            m_edtHex.Focus();
                            m_edtHex.SelectionStart = (SelectionStart * 3);
                        }
                    break;
                }
            }
            else
            {
                e.Handled = true;
            }
        }

        /*
        ***************************************************************************
        **
        ** Function: OnKeyDown
        */
        ///<summary>
        /// To Keep some special keys from being used
        ///</summary>
        ///<param name="sender" type="object"></param>
        ///<param name="e" type="System.Windows.Forms.KeyEventArgs"></param>
        ///<returns>void</returns>
        ///
        protected void OnKeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            e.Handled = true;

            /*
            ** This allows the back arrow to work
            */
            switch(e.KeyCode)
            {
                default:
                    e.Handled = false;
                break;

                case Keys.Delete:
                break;

                case Keys.Insert:
                break;

                case Keys.Back:
                {
                    int             iPos    = 0;

                    iPos = SelectionStart - 1;
                    if((-1 != iPos) && (iPos < m_iDataLength))
                    {
                        SelectionStart = iPos;
                    }
                }
                break;

                case Keys.Return:
                {
                    CharPosition    cp      = Position;
                    int             iPos    = 0;

                    iPos = LineIndex(cp.LinePos+1);
                    if((-1 != iPos) && (iPos < m_iDataLength))
                    {
                        SelectionStart = iPos;
                    }
                }
                break;
            }
        }
    }
}
