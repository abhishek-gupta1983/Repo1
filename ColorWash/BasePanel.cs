using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.Drawing;

namespace ColorWash
{
    public partial class BasePanel : Control
    {
        public BasePanel()
        {
            InitializeComponent();
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.SupportsTransparentBackColor, true);
            SetStyle(ControlStyles.UserMouse, true);
          //  SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

            


        }
        private bool isHit = false;
        private Region region1; 
        private Region region2;
        SolidBrush solidBrush = new SolidBrush(Color.Black);
        GraphicsPath path1;
        public BasePanel(IContainer container)
        {
            container.Add(this);

            InitializeComponent();
            path1 = new GraphicsPath();
            path1.AddEllipse(150, 150, 300, 300);
            region1 = new Region(path1);
            region2 = new Region(new Rectangle(0, 50, 150, 50));

            // Create a plus-shaped region by forming the union of region1 and  
            // region2. 
            // The union replaces region1.
            //region1.Union(region2);
            
        }
        
        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            Point point = new Point(e.Location.X,e.Location.Y);
            if (path1.IsVisible(point))
            {
                // The point is in the region. Use an opaque brush.
                solidBrush.Color = Color.FromArgb(255, 255, 0, 0);
            }
            else
            {
                // The point is not in the region. Use a semitransparent brush.
                solidBrush.Color = Color.FromArgb(64, 255, 0, 0);
            }
            Invalidate();
        }
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            //Point point = new Point(60, 10);
            Point point = new Point(50, 0);

            // Assume that the variable "point" contains the location of the
            // most recent mouse click. 
            // To simulate a hit, assign (60, 10) to point. 
            // To simulate a miss, assign (0, 0) to point.


            if (region1 != null)
            {

                e.Graphics.FillRegion(solidBrush, region1);
            }
// create an ellipse
            //GraphicsPath path = new GraphicsPath();
            //path.AddEllipse(100,100, 300, 100);

            //GraphicsPath path1 = new GraphicsPath();
            //path1.AddEllipse(150, 150, 300, 100);
            
            //// draw ellipse with black color
            //Pen blackPen = new Pen(Color.Black, 2.0F);
            //e.Graphics.FillPath(blackPen.Brush, path);

            //Pen whitePen = new Pen(Color.Wheat, 2.0F);
            //e.Graphics.DrawPath(whitePen, path1);

            //// get bounding rectangle
            //Rectangle boundingRect = Rectangle.Round(path.GetBounds());
            
            //// draw bounding rect with red color
            //Pen redPen = new Pen(Color.Red, 1.0F);
            //e.Graphics.DrawRectangle(redPen, boundingRect);

            //// get bounding rectangle
            //Rectangle boundingRect1 = Rectangle.Round(path1.GetBounds());

            //// draw bounding rect with red color
            //e.Graphics.DrawRectangle(redPen, boundingRect1);
        }
    }
}
