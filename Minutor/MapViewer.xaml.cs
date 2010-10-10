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

        public static RoutedUICommand Open1 = new RoutedUICommand("World 1", "World1", typeof(MapViewer));
        public static RoutedUICommand Open2 = new RoutedUICommand("World 2", "World2", typeof(MapViewer));
        public static RoutedUICommand Open3 = new RoutedUICommand("World 3", "World3", typeof(MapViewer));
        public static RoutedUICommand Open4 = new RoutedUICommand("World 4", "World4", typeof(MapViewer));
        public static RoutedUICommand Open5 = new RoutedUICommand("World 5", "World5", typeof(MapViewer));
        
        public MapViewer()
        {
            Open1.InputGestures.Add(new KeyGesture(Key.D1, ModifierKeys.Control));
            Open2.InputGestures.Add(new KeyGesture(Key.D2, ModifierKeys.Control));
            Open3.InputGestures.Add(new KeyGesture(Key.D3, ModifierKeys.Control));
            Open4.InputGestures.Add(new KeyGesture(Key.D4, ModifierKeys.Control));
            Open5.InputGestures.Add(new KeyGesture(Key.D5, ModifierKeys.Control));
            
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
                    else
                    {
                        var rect = new Int32Rect(0, 0, curWidth, curHeight);
                        for (int i = 0; i < curWidth * curHeight * 4; i++)
                            bits[i] = 0xff;
                        mapbits.WritePixels(rect, bits, curWidth * 4, 0);
                    }
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
            LayerSlider.IsEnabled = false;
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
            Map.Focus();
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
            LayerSlider.IsEnabled = true;
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
            if (layerLabel!=null)
                layerLabel.Text = curDepth.ToString();
            if (loaded)
                RenderMap();
        }

        int Moving = 0; //key bitmask
        private void map_keyDown(object sender, KeyEventArgs e)
        {
            bool changed=false;
            switch (e.Key)
            {
                case Key.Up:
                case Key.W:
                    Moving |= 1;
                    break;
                case Key.Down:
                case Key.S:
                    Moving |= 2;
                    break;
                case Key.Left:
                case Key.A:
                    Moving |= 4;
                    break;
                case Key.Right:
                case Key.D:
                    Moving |= 8;
                    break;
                case Key.PageUp:
                case Key.E:
                    curScale += 0.5;
                    if (curScale > 5.0)
                        curScale = 5.0;
                    changed = true;
                    break;
                case Key.PageDown:
                case Key.Q:
                    curScale -= 0.5;
                    if (curScale < 1.0)
                        curScale = 1.0;
                    changed = true;
                    break;
            }
            if (Moving != 0)
            {
                if ((Moving & 1) != 0) //up
                    curX -= 10.0 / curScale;
                if ((Moving & 2) != 0) //down
                    curX += 10.0 / curScale;
                if ((Moving & 4) != 0) //left
                    curZ += 10.0 / curScale;
                if ((Moving & 8) != 0) //right
                    curZ -= 10.0 / curScale;
                changed = true;
            }
            if (changed)
            {
                e.Handled = true;
                if (loaded)
                    RenderMap();
            }
        }

        private void map_keyUp(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Up:
                case Key.W:
                    Moving &= ~1;
                    break;
                case Key.Down:
                case Key.S:
                    Moving &= ~2;
                    break;
                case Key.Left:
                case Key.A:
                    Moving &= ~4;
                    break;
                case Key.Right:
                case Key.D:
                    Moving &= ~8;
                    break;
            }
        }

        private void Open_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            int num = 0;
            if (e.Command == Open1) num = 1;
            if (e.Command == Open2) num = 2;
            if (e.Command == Open3) num = 3;
            if (e.Command == Open4) num = 4;
            if (e.Command == Open5) num = 5;
            if (num==0)
            {
                var dlg = new Microsoft.Win32.OpenFileDialog();
                dlg.FileName = "level.dat";
                dlg.Filter = "Minecraft saves|level.dat";
                var result = dlg.ShowDialog();
                if (result == true)
                {
                    MapDll.CloseAll();
                    var path = Path.GetDirectoryName(dlg.FileName);
                    Loading load = new Loading();
                    load.Show();
                    Load(path);
                    load.Close();
                }
            }
            else
            {
                MapDll.CloseAll();
                Loading load = new Loading();
                load.Show();
                Load(WorldPath(num));
                load.Close();
            }
        }

        private void Open_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            int num = 0;
            if (e.Command == Open1) num = 1;
            if (e.Command == Open2) num = 2;
            if (e.Command == Open3) num = 3;
            if (e.Command == Open4) num = 4;
            if (e.Command == Open5) num = 5;

            if (num==0) //always able to open arbitrary world
            {
                e.CanExecute = true;
                return;
            }
            DirectoryInfo di = new DirectoryInfo(WorldPath(num));
            e.CanExecute = di.Exists;
        }
        private string WorldPath(int num)
        {
            var path = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                ".minecraft");
            path = Path.Combine(path, "saves");
            path = Path.Combine(path, String.Format("World{0}", num));
            return path;
        }

        private void Close_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            this.Close();
        }
        private void Close_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
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
