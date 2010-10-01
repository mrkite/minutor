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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;

namespace Minutor
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        List<string> worldPaths;
        public Window1()
        {
            worldPaths = new List<string>();
            for (var i = 0; i < 5; i++)
                worldPaths.Add(null);
            var root = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                ".minecraft");
            root = Path.Combine(root, "saves");
            DirectoryInfo di = new DirectoryInfo(root);
            if (di.Exists)
            {
                foreach (DirectoryInfo ddi in di.GetDirectories("World*"))
                {
                    switch (ddi.Name)
                    {
                        case "World1": worldPaths[0] = ddi.FullName; break;
                        case "World2": worldPaths[1] = ddi.FullName; break;
                        case "World3": worldPaths[2] = ddi.FullName; break;
                        case "World4": worldPaths[3] = ddi.FullName; break;
                        case "World5": worldPaths[4] = ddi.FullName; break;
                    }
                }
            }

            InitializeComponent();

            for (var i = 0; i < worldPaths.Count; i++)
            {
                if (worldPaths[i] == null)
                    (worlds.Items[i] as ComboBoxItem).IsEnabled = false;
            }
        }
        private void worlds_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            standardWorld.IsChecked = true;
        }
        string customPath = null;
        private void customButton_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "level.dat";
            dlg.Filter = "Minecraft saves|level.dat";
            var result = dlg.ShowDialog();
            if (result == true)
            {
                customWorld.IsChecked = true;
                customButton.Content = dlg.FileName;
                customPath = dlg.FileName;
            }
        }
        private void viewButton_Click(object sender, RoutedEventArgs e)
        {
            MapViewer map=new MapViewer();
            map.Show();
            try
            {
                if (standardWorld.IsChecked == true)
                {
                    if (worlds.SelectedIndex != -1 && worldPaths[worlds.SelectedIndex] != null)
                    {
                        Loading load = new Loading();
                        load.Show();
                        this.Close();
                        map.Load(worldPaths[worlds.SelectedIndex]);
                        load.Close();
                    }
                } else if (customPath != null)
                {
                    Loading load = new Loading();
                    load.Show();
                    this.Close();
                    map.Load(Path.GetDirectoryName(customPath));
                    load.Close();
                }
            }
            catch (Exception ex)
            {
                map.Close();
                MessageBox.Show("Couldn't load world: "+ex.Message);
            }
        }
    }
}
