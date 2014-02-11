using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using ImageUtils;
using MPR_UI.Properties;
namespace MPR_UI
{
    public partial class ImagePanel2 : Control
    {
        private Bitmap m_storedBitmap;
        private struct MPRCursor
        {
            public Line l1;
            public Line l2;
        };

        private MPRCursor m_mprCursor;
        private CoordinateMapping m_coordinateMapping;
        private Point mousePosition;
        private Point mousePosition2;
        private PointF cursorPosition;

        public ImagePanel2()
        {
            InitializeComponent();

            currentDisplayOffsetPt = new Point(0, 0);
            originalDisplayOffsetPt = new Point(0, 0);
            BorderSize = 2;
            currentZoomFactor = 1.0F;
            originalZoomFactor = 1.0F;
            
            // initialize MPR cursor
            this.m_mprCursor = new MPRCursor();
            // initial coordinate system mapping
            this.m_coordinateMapping = new CoordinateMapping();

            // Set few control option.
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.DoubleBuffer, true);
            SetStyle(ControlStyles.UserMouse, true);

            // handle resize
            this.Resize += new EventHandler(ImagePanel2_Resize);
        }

        /// <summary>
        /// Handle Image panel resize.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ImagePanel2_Resize(object sender, EventArgs e)
        {
            InitializeZoomAndPosition();
        }

        public ImagePanel2(IContainer container)
        {
            container.Add(this);

            InitializeComponent();
        }

        
        public Bitmap StoreBitmap
        {
            set
            {
                if (m_storedBitmap == null)
                {
                    m_storedBitmap = value;
                    InitializeZoomAndPosition();
                }
                m_storedBitmap = value;
                Invalidate();
            }
            get { return m_storedBitmap; }
        }
        private void InitializeZoomAndPosition()
        {
            if (StoreBitmap == null) return;

            float zoom = 1.0F;
            Point p = new Point(0, 0);
            Size effectivePanelSize = this.Size - new Size((10 + (int)(2 * this.BorderSize)), (10 + (int)(2 * this.BorderSize)));
            float diffHeight = ((float)effectivePanelSize.Height) / ((float)StoreBitmap.Height);
            float diffWidth = ((float)effectivePanelSize.Width) / ((float)StoreBitmap.Width);
            zoom = Math.Min(diffHeight, diffWidth);

            int w = (int)(StoreBitmap.Width * zoom);
            int h = (int)(StoreBitmap.Height * zoom);
            p.X = (int)((effectivePanelSize.Width - w) / (2 * zoom));
            p.X += 5 + (int)BorderSize;
            p.Y = (int)((effectivePanelSize.Height - h) / (2 * zoom));
            p.Y += 5 + (int)BorderSize;
            this.currentDisplayOffsetPt = p;
            this.originalDisplayOffsetPt = p;
            this.currentZoomFactor = zoom;
            this.originalZoomFactor = zoom;
            imageRect = new RectangleF((currentZoomFactor * currentDisplayOffsetPt.X) - 1,
                    (currentZoomFactor * currentDisplayOffsetPt.Y) - 1,
                    (currentZoomFactor * StoreBitmap.Width) + 2,
                    (currentZoomFactor * StoreBitmap.Height) + 2);


        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            mousePosition = getOriginalCoords(new Point(e.X, e.Y));
            mousePosition2 = getDicomCoords(new Point(e.X, e.Y));
            base.OnMouseDown(e);
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            mousePosition2 = getDicomCoords(new Point(e.X, e.Y));
            mousePosition = getOriginalCoords(new Point(e.X, e.Y));
            base.OnMouseMove(e);
            Invalidate();
        }

        // returns co-ordinate wrt 0,0 as top-left.
        private Point getOriginalCoords(Point p)
        {
            Point ret = new Point((int)((p.X / currentZoomFactor) - currentDisplayOffsetPt.X),
                (int)((p.Y / currentZoomFactor) - currentDisplayOffsetPt.Y));
            return this.m_coordinateMapping.GetActualPosition(ret);
        }

        private Point getDicomCoords(Point p)
        {
            Point ret = this.m_coordinateMapping.GetActualDisplayPosition(new Point(p.X, p.Y));
            ret = new Point((int)(this.currentZoomFactor * (p.X + this.currentDisplayOffsetPt.X)), 
                          (int)(this.currentZoomFactor * (p.Y + this.currentDisplayOffsetPt.Y)));
            return ret;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (StoreBitmap == null) return;
            RectangleF srcRect = new RectangleF((e.ClipRectangle.X / currentZoomFactor) - currentDisplayOffsetPt.X,
                    (e.ClipRectangle.Y / currentZoomFactor) - currentDisplayOffsetPt.Y,
                    e.ClipRectangle.Width / currentZoomFactor, e.ClipRectangle.Height / currentZoomFactor);

            Rectangle roundedRectangle = Rectangle.Round(imageRect);
            e.Graphics.DrawRectangle(new Pen(Color.FromArgb(255, 0, 0)), roundedRectangle);
            e.Graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
            e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
            e.Graphics.DrawImage(StoreBitmap, e.ClipRectangle, srcRect, GraphicsUnit.Pixel);

            Pen _pen1 = new Pen(Color.LightGoldenrodYellow, 2.0F);
            
            if (this.Parent.Parent.Name.CompareTo("ImageControl") == 0)
            { 
                var imgControl = (ImageControl)this.Parent.Parent;
                int idx = imgControl.GetScrollbarValue();
                Font _font = new Font("Verdana", 10.0F);
                StringBuilder _sb = new StringBuilder();
                _sb.Append("Scroll Pos#");
                _sb.Append(idx);
                e.Graphics.DrawString(_sb.ToString(), _font, _pen1.Brush, new PointF(10, 0));

                _sb.Clear();
                _sb.Append("Slicer Pos#");
                _sb.Append(imgControl.Position);
                e.Graphics.DrawString(_sb.ToString(), _font, _pen1.Brush, new PointF(10, 20));

                _sb.Clear();
                _sb.Append("Slicer idx#");
                _sb.Append(imgControl.Index);
                e.Graphics.DrawString(_sb.ToString(), _font, _pen1.Brush, new PointF(10, 40));
            }

            if (mousePosition != null)
            {
                StringBuilder _sb = new StringBuilder();
                _sb.Append("X:");
                _sb.Append(mousePosition.X);
                _sb.Append(" Y:");
                _sb.Append(mousePosition.Y);
                Font _font = new Font("Verdana", 10.0F);
                e.Graphics.DrawString(_sb.ToString(), _font, _pen1.Brush, new PointF(10, 50));
            }
            if (mousePosition2 != null)
            {
                StringBuilder _sb = new StringBuilder();
                _sb.Append("X2:");
                _sb.Append(mousePosition2.X);
                _sb.Append(" Y2:");
                _sb.Append(mousePosition2.Y);
                Font _font = new Font("Verdana", 10.0F);
                e.Graphics.DrawString(_sb.ToString(), _font, _pen1.Brush, new PointF(10, 70));
            }

            if (cursorPosition != null)
            {
                GraphicsPath cursorPath = new GraphicsPath();

                cursorPath.AddEllipse(cursorPosition.X - 5.0F, cursorPosition.Y - 5.0F, 10.0F, 10.0F);

                cursorPath.CloseFigure();
                e.Graphics.FillPath(_pen1.Brush, cursorPath);
            }
            // paint cursor
            if (this.m_mprCursor.l1 != null)
            {
                e.Graphics.DrawLine(this.m_mprCursor.l1.DisplayPen,
                    this.m_mprCursor.l1.P1.X, this.m_mprCursor.l1.P1.Y, this.m_mprCursor.l1.P2.X, this.m_mprCursor.l1.P2.Y);
            }
            if (this.m_mprCursor.l2 != null)
            {
                e.Graphics.DrawLine(this.m_mprCursor.l2.DisplayPen,
                    this.m_mprCursor.l2.P1.X, this.m_mprCursor.l2.P1.Y, this.m_mprCursor.l2.P2.X, this.m_mprCursor.l2.P2.Y);
            }
            base.OnPaint(e);
        }

        public float currentZoomFactor { get; set; }

        public float originalZoomFactor { get; set; }

        public RectangleF imageRect { get; set; }

        public int BorderSize { get; set; }

        public Point currentDisplayOffsetPt { get; set; }

        public Point originalDisplayOffsetPt { get; set; }

        public void SetCursorPositionX_Axis(PointF p, Axis axis)
        {
            if (this.m_mprCursor.l1 == null)
            {
                this.m_mprCursor.l1 = new Line();
            }

            PointF p1 = GetActualDisplayPosition(p);
            //p1 = new PointF((float)(this.currentZoomFactor * (p1.X + this.currentDisplayOffsetPt.X)), 
             //               (float)(this.currentZoomFactor * (p1.Y + this.currentDisplayOffsetPt.Y)));
            this.m_mprCursor.l1.P1 = new PointF(p1.X, imageRect.Top);
            this.m_mprCursor.l1.P2 = new PointF(p1.X, imageRect.Bottom);
            this.m_mprCursor.l1.Axis = axis;
            cursorPosition = p1;
        }


        public void SetCursorPositionY_Axis(PointF p, Axis axis)
        {
            if (this.m_mprCursor.l2 == null)
            {
                this.m_mprCursor.l2 = new Line();
            }

           
            PointF p1 = GetActualDisplayPosition(p);
            //p1 = new PointF((float)(this.currentZoomFactor * (p1.X + this.currentDisplayOffsetPt.X)),
            //                (float)(this.currentZoomFactor * (p1.Y + this.currentDisplayOffsetPt.Y)));
            this.m_mprCursor.l2.P1 = new PointF(imageRect.Left,p1.Y);
            this.m_mprCursor.l2.P2 = new PointF(imageRect.Right,p1.Y);
            this.m_mprCursor.l2.Axis = axis;
            cursorPosition = p1;
        }

        public PointF GetActualDisplayPosition(PointF point)
        {
            PointF p = this.m_coordinateMapping.GetActualDisplayPosition(point);
            p = new PointF((float)(this.currentZoomFactor * (p.X + this.currentDisplayOffsetPt.X)), (float)(this.currentZoomFactor * (p.Y + this.currentDisplayOffsetPt.Y)));
            return p;
        }
    }
}
