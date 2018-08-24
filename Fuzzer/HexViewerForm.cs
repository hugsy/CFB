using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using HexEdit;

namespace Fuzzer
{
    public partial class HexViewerForm : Form
    {
        private Splitter splitter1;
        private HexEdit.HexEditBox m_edtHex;
        private HexEdit.LinkedBox m_edtASCII;
        private byte[] m_abyData;


        public HexViewerForm()
        {
            InitializeComponent();

            m_abyData = new byte[4096];
        }


        public HexViewerForm(int Index, Irp irp)
        {
            InitializeComponent();

            this.Text = String.Format("HexViewer for IRP #{0:d} (IoctlNumber={1:d})", Index, irp.Header.IoctlCode);
            m_abyData = irp.Body;
        }


        private void SetupForm()
        {
            this.m_edtHex = new HexEdit.HexEditBox();
            this.m_edtASCII = new LinkedBox();
            this.splitter1 = new Splitter();
            this.SuspendLayout();
            //
            // m_edtHex
            //
            this.m_edtHex.Dock = DockStyle.Left;
            this.m_edtHex.Location = new System.Drawing.Point(0, 0);
            this.m_edtHex.Name = "m_edtHex";
            this.m_edtHex.Size = new System.Drawing.Size(360, 326);
            this.m_edtHex.TabIndex = 0;
            this.m_edtHex.Text = "richTextBox1";
            //
            // m_edtASCII
            //
            this.m_edtASCII.Dock = DockStyle.Fill;
            this.m_edtASCII.Location = new System.Drawing.Point(168, 0);
            this.m_edtASCII.Name = "m_edtASCII";
            this.m_edtASCII.Size = new System.Drawing.Size(320, 326);
            this.m_edtASCII.TabIndex = 1;
            this.m_edtASCII.Text = "foo";
            //
            // splitter1
            //
            this.splitter1.Location = new System.Drawing.Point(168, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(1, 326);
            this.splitter1.TabIndex = 2;
            this.splitter1.TabStop = false;
            //
            // Form1
            //
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(488, 326);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.m_edtASCII);
            this.Controls.Add(this.m_edtHex);
        }

        private void HexViewerForm_Load(object sender, EventArgs e)
        {
            SetupForm();

            m_edtHex.InitializeComponent();
            m_edtASCII.InitializeComponent();
            m_edtHex.LinkDisplay(m_edtASCII);

            m_edtHex.LoadData(m_abyData);
        }
    }
}
