using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Integration;

namespace Applicaiton
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            /* our WPF control */
            WPFControl.UserControl2 uc1 = new WPFControl.UserControl2();

            /* container that will host our WPF control, we set it using 
             * the Child property */
            ElementHost host = new ElementHost()
            {
                BackColor = Color.Transparent,
                Child = uc1,
                Dock = DockStyle.Fill,
            };

            /* now add the ElementHost to our controls collection 
             * normally */
            Controls.Add(host);
        }
    }
}
