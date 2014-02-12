using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class UserControl1 : UserControl
    {
        public UserControl1()
        {
            InitializeComponent();
            this.Load += UserControl1_Load;
        }

        void UserControl1_Load(object sender, EventArgs e)
        {
            UserPanel panel = new UserPanel();
            panel.Dock = DockStyle.Fill;
            this.Controls.Add(panel);

        }
    }
}
