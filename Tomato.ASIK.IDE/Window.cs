using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Tomato.ASIK.Common
{
    public class Window : System.Windows.Window
    {
        public static readonly DependencyPropertyKey ViewBagPropertyKey = DependencyProperty.
            RegisterReadOnly("ValueBag", typeof(ExpandoObject), typeof(Window),
            new PropertyMetadata(new ExpandoObject()));

        public dynamic ViewBag
        {
            get { return GetValue(ViewBagPropertyKey.DependencyProperty); }
        }

        public Window()
        {
        }
    }
}
