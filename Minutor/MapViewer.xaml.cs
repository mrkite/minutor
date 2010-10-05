/*
Copyright (c) 2010, Sean Kasun
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using System.Runtime.InteropServices;

namespace Minutor
{
    /// <summary>
    /// Interaction logic for MapViewer.xaml
    /// </summary>
    /// 
    public partial class MapViewer : Window
    {
        WriteableBitmap mapbits;
        byte[] bits;
        string world;
        double curX, curZ, curScale;
        int curDepth;
        Popup popup;
        TextBox popupText;
        int curWidth, curHeight, newWidth, newHeight;
        DispatcherTimer resizeTimer;
        
        public MapViewer()
        {
            InitializeComponent();

            // this resize timer is used because windows sends a bajillion resize events while
            // resizing the window.  That will kill us.  So instead we wait 50ms to see if new
            // resize events come in before responding to one.
            resizeTimer = new DispatcherTimer(
                TimeSpan.FromMilliseconds(50), DispatcherPriority.Normal,
                delegate
                {
                    resizeTimer.IsEnabled = false;
                    //a resize requires a new bitmap source for the image
                    curWidth = newWidth;
                    curHeight = newHeight;
                    mapbits = new WriteableBitmap(curWidth, curHeight, 96.0, 96.0, PixelFormats.Bgr32, null);
                    Map.Source = mapbits;
                    //and a new buffer for the raw data
                    bits = new byte[curWidth * curHeight * 4];
                    Map.Width = curWidth;
                    Map.Height = curHeight;
                    if (loaded)
                        RenderMap();
                },
                Dispatcher
                ) { IsEnabled = false };
            curWidth = 496;
            curHeight = 400;
            newWidth = 496;
            newHeight = 400;
            mapbits = new WriteableBitmap(curWidth, curHeight, 96.0, 96.0, PixelFormats.Bgr32, null);
            Map.Source = mapbits;
            bits = new byte[curWidth * curHeight * 4];
            curX = curZ = 0;
            curScale = 1.0;
            curDepth = 127;
            popup = new Popup();
            popupText = new TextBox();
            popupText.Background = Brushes.LightGray;
            popupText.Foreground = Brushes.Black;
            popupText.BorderThickness = new Thickness(1.0);
            popupText.BorderBrush = Brushes.Black;
            popupText.MouseRightButtonUp += new MouseButtonEventHandler(Map_MouseRightButtonUp);
            popup.Child = popupText;
            popup.Placement = PlacementMode.Mouse;
        }

        void Map_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            newWidth = curWidth;
            newHeight = curHeight;
            if (e.HeightChanged)
                newHeight = (int)e.NewSize.Height;
            if (e.WidthChanged)
                newWidth = (int)e.NewSize.Width;
            if (e.HeightChanged || e.WidthChanged)
            {
                resizeTimer.IsEnabled = true;
                resizeTimer.Stop();
                resizeTimer.Start();
            }
            e.Handled = true;
        }

        void Map_MouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            popup.IsOpen = false;
            e.Handled = true;
        }

        void Map_MouseRightButtonDown(object sender, MouseButtonEventArgs e)
        {
            Point pos = e.GetPosition(Map);
            // make sure mouse is inbounds of the map.
            if (pos.X < 0 || pos.Y < 0 || pos.X >= curWidth || pos.Y >= curHeight)
                return;
            popupText.Text = MapDll.IDBlock((int)pos.X, (int)pos.Y, curX, curZ, curWidth, curHeight, curScale);
            popup.IsOpen = true;
            e.Handled = true;
        }

        void Map_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            Map.ReleaseMouseCapture();
        }

        void Map_MouseMove(object sender, MouseEventArgs e)
        {
            if (Map.IsMouseCaptured)
            {
                Point curPos = e.GetPosition(Map);
                Vector v = start - curPos;
                curX += v.Y / curScale;
                curZ -= v.X / curScale;
                start = curPos;
                if (loaded)
                    RenderMap();
            }
        }

        Point start;
        void Map_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Map.CaptureMouse();
            start = e.GetPosition(Map);
        }

        void Map_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            curScale += (double)e.Delta / 500.0;
            if (curScale < 1.0)
                curScale = 1.0;
            if (curScale > 5.0)
                curScale = 5.0;
            if (loaded)
                RenderMap();
        }
        bool loaded = false;
        public void Load(string saves)
        {
            world = saves;
            Title = Path.GetFileName(saves);
            int x, y, z;
            MapDll.GetSpawn(world, out x, out y, out z);
            curX = x;
            curZ = z;
            loaded = true;
            RenderMap();
        }
        void RenderMap()
        {
            var rect=new Int32Rect(0,0,curWidth,curHeight);
            MapDll.DrawMap(world, curX, curZ, curDepth, curWidth, curHeight, curScale, bits);
            for (var i = 0; i < curWidth * curHeight * 4; i+=4)
            {
                bits[i] ^= bits[i + 2];
                bits[i + 2] ^= bits[i];
                bits[i] ^= bits[i + 2];
            }
            mapbits.WritePixels(rect,bits,curWidth*4,0);
        }
        private void LayerSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            curDepth = (int)e.NewValue;
            if (loaded)
                RenderMap();
        }

        private void map_keyDown(object sender, KeyEventArgs e)
        {
            bool changed=false;
            switch (e.Key)
            {
                case Key.Up:
                    curX -= 10.0 / curScale;
                    changed = true;
                    break;
                case Key.Down:
                    curX += 10.0 / curScale;
                    changed = true;
                    break;
                case Key.Left:
                    curZ += 10.0 / curScale;
                    changed = true;
                    break;
                case Key.Right:
                    curZ -= 10.0 / curScale;
                    changed = true;
                    break;
                case Key.PageUp:
                    curScale += 1.0;
                    if (curScale > 5.0)
                        curScale = 5.0;
                    changed = true;
                    break;
                case Key.PageDown:
                    curScale -= 1.0;
                    if (curScale < 1.0)
                        curScale = 1.0;
                    changed = true;
                    break;
            }
            if (changed)
            {
                e.Handled = true;
                if (loaded)
                    RenderMap();
            }
        }
    }
    class MapDll
    {
        [DllImport("MinutorMap.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DrawMap(string world, double cx, double cz, int y, int w, int h, double zoom, byte[] bits);
        [DllImport("MinutorMap.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern string IDBlock(int bx, int by, double cx, double cz, int w, int h, double zoom);
        [DllImport("MinutorMap.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CloseAll();
        [DllImport("MinutorMap.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetSpawn(string world, out int x, out int y, out int z);

    }
}
