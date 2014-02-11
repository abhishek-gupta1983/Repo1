﻿using System;
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
        private Axis m_axis;
        private ImagePanel2 m_imagePanel;
        private MPR_UI_Interface m_UIInterface;
        private double m_position;
        private int m_index;

       
       
        public ImageControl()
        {
            InitializeComponent();
        }

        public ImageControl(Axis axis)
        {
            InitializeComponent();
            this.m_axis = axis;
            this.m_imagePanel = new ImagePanel2();
            this.m_imagePanel.Dock = DockStyle.Fill;
            this.panel1.Controls.Add(this.m_imagePanel);

            m_UIInterface = MPR_UI_Interface.GetHandle();

            
        }

        internal void InitScrollBarAndLoadImage()
        {
            this.scrollBar.Maximum = m_UIInterface.GetNumberOfImages((int)this.m_axis);
            this.scrollBar.Minimum = 0;
            this.scrollBar.SmallChange = 1;
            this.scrollBar.LargeChange = 1;
            this.scrollBar.Value = m_UIInterface.GetCurrentImageIndex((int)this.m_axis);
            // Init scroll bar event
            this.scrollBar.ValueChanged += scrollBar_ValueChanged;
            LoadImage();
        }

        void scrollBar_ValueChanged(object sender, EventArgs e)
        {
            label1.Text = Convert.ToString(scrollBar.Value);
            if (scrollBar.Value != Index)
            {
                m_UIInterface.Scroll((int)this.m_axis, scrollBar.Value - Index);
                LoadImage();
            }
        }

      
        internal void LoadImage()
        {
            BitmapWrapper bmpWrapper = m_UIInterface.GetDisplayImage((int)this.m_axis);
            Position = m_UIInterface.GetCurrentImagePosition((int)this.m_axis);
            Index = m_UIInterface.GetCurrentImageIndex((int)this.m_axis);
            if (Index != scrollBar.Value)
                MessageBox.Show("Alert");
            this.m_imagePanel.StoreBitmap = bmpWrapper.StoredBitmap;
            switch (this.m_axis)
            {
                case Axis.AxialAxis:
                    {
                        double PositionS = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.SagittalAxis);
                        double PositionC = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.CoronalAxis);
                        PointF cursorPoint = new PointF((float)PositionS, (float)PositionC);

                        this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.CoronalAxis);
                        this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.SagittalAxis);
                    }
                    break;
                case Axis.CoronalAxis:
                    {
                        double PositionA = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.AxialAxis);
                        double PositionS = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.SagittalAxis);

                        PointF cursorPoint = new PointF((float)PositionS, (float)PositionA);
                        this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.SagittalAxis);
                        this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.AxialAxis);

                    }
                    break;
                case Axis.SagittalAxis:
                    {
                        double PositionA = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.AxialAxis);
                        double PositionC = m_UIInterface.GetCurrentImagePositionRelativeToOrigin((int)Axis.CoronalAxis);

                        PointF cursorPoint = new PointF((float)PositionC, (float)PositionA);
                        this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.CoronalAxis);
                        this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.AxialAxis);
                    }
                    break;
                default:
                    break;
            }
            
            this.Invalidate();
            
        }

        internal int GetScrollbarValue()
        {
            return scrollBar.Value;
        }

        internal double Position
        {
            get { return m_position; }
            set { m_position = value; }
        }

        internal int Index
        {
            get { return m_index; }
            set { m_index = value; }
        }
    }
}
