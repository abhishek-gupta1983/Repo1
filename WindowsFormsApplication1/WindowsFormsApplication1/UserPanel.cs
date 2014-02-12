using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class UserPanel : Control
    {
        GraphicsPath objectPath;
        Point objectLocation;
        bool objectSelected;
        private struct Line
        {
            Point p1;
            Point p2;
        };

        public UserPanel()
        {
            InitializeComponent();
            objectPath = new GraphicsPath();
            objectLocation = new Point(0, 0);
            objectPath.Reset();
            objectPath.AddEllipse(objectLocation.X - 5.0F, objectLocation.Y - 5.0F, 10.0F, 10.0F);
            objectPath.CloseFigure();
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.UserMouse, true);
        }

        public UserPanel(IContainer container)
        {
            container.Add(this);

            InitializeComponent();
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            objectSelected = false;
            base.OnMouseUp(e);
        }
        protected override void OnMouseDown(MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                if (objectPath.GetBounds().Contains(e.Location))
                {
                    objectSelected = true;
                }
                objectLocation = e.Location;
            }
            base.OnMouseDown(e);
        }
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (objectSelected == true)
            {
                objectLocation = e.Location;
                objectPath.Reset();
                objectPath.AddEllipse(objectLocation.X - 5.0F, objectLocation.Y - 5.0F, 10.0F, 10.0F);
                objectPath.CloseFigure();
            }
            base.OnMouseMove(e);
            Invalidate();
        }
        protected override void OnPaint(PaintEventArgs e)
        {
            Pen p = new Pen(Color.Gold, 10.0F);
            e.Graphics.FillPath(p.Brush, objectPath);
             base.OnPaint(e);
        }
    }
}
