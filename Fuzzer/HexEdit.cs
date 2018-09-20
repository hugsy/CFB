using System;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

using Fuzzer;

namespace HexEdit
{
    public class HexEditBase : RichTextBox
    {
        protected bool              m_bNoUpdate         = false;  // Control the Updates between windows
        protected ContextMenu       m_menuContext       = null;
        protected const int EM_LINEINDEX        = 0xbb;
        protected const int EM_LINEFROMCHAR     = 0xc9;
        protected const int EM_GETSEL           = 0xb0;

        public class CharPosition
        {
            protected int m_iLine   = 0;
            protected int m_iChar   = 0;

            public CharPosition()
            {
            }


            public CharPosition(int iLine, int iChar)
            {
                LinePos = iLine;
                CharPos = iChar;
            }

            public int LinePos
            {
                get{return m_iLine;}
                set{m_iLine = value;}
            }

            public int CharPos
            {
                get{return m_iChar;}
                set{m_iChar = value;}
            }

            public override string ToString()
            {
                return(String.Format("{{L={0}, C={1}}}", LinePos, CharPos));
            }
        }


        public Point CaretPosition
        {
            get
            {
                Point pt = Point.Empty;
                return pt;
            }
        }


        public int LineIndex(int iLine)
        {
            return (int)Win32Window.SendMessage(new HandleRef(this, Handle), EM_LINEINDEX, iLine, 0);
        }


        public int LineIndex()
        {
            return LineIndex(-1);
        }


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


        protected char GetDisplayChar(char cData)
        {
            if(20 > cData)
            {
                cData = (char)0xB7;
            }

            return cData;
        }


        protected char GetDisplayChar(byte byData)
        {
            return GetDisplayChar((char)byData);
        }


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


        virtual public void InitializeComponent()
        {
        }
    }


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


        protected void Menu_SelectAll(object sender, System.EventArgs e)
        {
            SelectAll();
        }


        protected void Menu_Copy(object sender, System.EventArgs e)
        {
            Copy();
        }


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


        public void LinkDisplay(LinkedBox rtbLink)
        {
            m_rtbLink = rtbLink;
            m_rtbLink.LinkHex(this);
        }


        string GetDisplayForByte(byte byData)
        {
            return string.Format("{0:X2}", byData);
        }


        public void LoadData(byte[] abyData)
        {
            m_abyData = new byte[abyData.Length];
            Array.Copy(abyData, 0, m_abyData, 0, abyData.Length);
            UpdateDisplay();
        }


        public void NewData(int iSize)
        {
            m_abyData = new byte[iSize];
            UpdateDisplay();
        }


        public byte[] ArrayData
        {
            get{return m_abyData;}
        }


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

        protected void OnResizeBox(object sender, System.EventArgs e)
        {
            if(null != m_abyData)
            {
                UpdateDisplay();
            }
        }

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

   public class LinkedBox : HexEditBase
   {
        ///<summary>Length of the Data this box should allow</summary>
        protected int           m_iDataLength       = 0;

        ///<summary></summary>
        protected HexEditBox    m_edtHex            = null;

        ///<summary></summary>
        protected MenuItem      m_miCopy            = null;

        ///<summary></summary>
        protected MenuItem      m_miPaste           = null;


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


        protected void Menu_Copy(object sender, System.EventArgs e)
        {
            Copy();
        }

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


        public void LinkHex(HexEditBox HexEdit)
        {
            m_edtHex = HexEdit;
        }


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


        protected void OnKeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            e.Handled = true;

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
