using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MPR_UI
{
    public partial class ImageControl : UserControl
    {
        private Axis axis;

        public ImageControl()
        {
            InitializeComponent();
        }

        public ImageControl(Axis axis)
        {
            InitializeComponent();
            // TODO: Complete member initialization
            this.axis = axis;
        }
    }
}
