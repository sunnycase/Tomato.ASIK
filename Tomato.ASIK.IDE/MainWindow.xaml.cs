using Microsoft.Win32;
using Sparrow.Chart;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using TomatoASIKCOMLib;

namespace Tomato.ASIK.IDE
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Common.Window
    {
        MFIOProvider provider = new MFIOProvider();
        

        public MainWindow()
        {
            InitializeComponent();
            Loaded += MainWindow_Loaded;
        }

        class WaveElem
        {
            public int Id { get; set; }
            public int Value { get; set; }
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            provider.Initialize();
        }

        async void ShowData(string fileName)
        {
            var data = await LoadData(fileName);
            ViewBag.MaxXValue = data.Length;
            var points = new PointsCollection();
            for (int i = 0; i < data.Length; i++)
                points.Add(new DoublePoint { Data = i, Value = data[i] });

            sc_WaveChart.Series[0].Points = points;
        }

        async Task<short[]> LoadData(string fileName)
        {
            uint bufferSize = 0;

            await Task.Run(() =>
            {
                provider.LoadFile(fileName, out bufferSize);
            });
            var buffer = new byte[bufferSize];
            provider.ReadAllSamples(buffer);

            var waveData = new short[bufferSize / 2];
            Buffer.BlockCopy(buffer, 0, waveData, 0, (int)bufferSize);
            return waveData;
        }

        private void bn_Browse_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFileDialog()
            {
                Filter = "音频文件|*.wav;*.mp3;*.wma"
            };
            if(dlg.ShowDialog() == true)
            {
                ViewBag.Path = dlg.FileName;
                ShowData(dlg.FileName);
            }
        }
    }
}
