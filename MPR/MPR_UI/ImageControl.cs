using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using MPR_VTK_BRIDGE;
using ImageUtils;

namespace MPR_UI
{
    public partial class ImageControl : UserControl
    {
        private Axis axis;
        private ImagePanel2 imagePanel;
        private MPR_UI_Interface ui_interface;
        public ImageControl()
        {
            InitializeComponent();
        }

        public ImageControl(Axis axis)
        {
            InitializeComponent();
            this.axis = axis;
            this.imagePanel = new ImagePanel2();
            this.imagePanel.Dock = DockStyle.Fill;
            this.panel1.Controls.Add(this.imagePanel);

            ui_interface = MPR_UI_Interface.GetHandle();

            
        }

        internal void InitScrollBarAndLoadImage()
        {
            this.scrollBar.Maximum = ui_interface.GetNumberOfImages((int)this.axis);
            this.scrollBar.Minimum = 1;
            this.scrollBar.Value = ui_interface.GetCurrentImageIndex((int)this.axis);
                // Init scroll bar event
            this.scrollBar.Scroll += scrollBar_Scroll;
            LoadImage();
        }

      

        internal void LoadImage()
        {
            BitmapWrapper bmpWrapper = ui_interface.GetDisplayImage((int)this.axis);
            this.imagePanel.StoreBitmap = bmpWrapper.StoredBitmap;
            this.Invalidate();
            
        }

        void scrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            int diff = e.NewValue - e.OldValue;
            ui_interface.Scroll((int)this.axis, diff);
            LoadImage();
            Application.DoEvents();
        }

        internal int GetScrollbarValue()
        {
            return scrollBar.Value;
        }
    }
}
