using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace GUI.UserControls
{
    public sealed partial class CollapsibleSearchBox : UserControl
    {
        private double RequestedWidth = 32;


        public CollapsibleSearchBox()
        {
            this.InitializeComponent();
            Loaded += CollapsableSearchBox_Loaded;
            Window.Current.SizeChanged += Current_SizeChanged;
            myAutoSuggestBox = searchBox;
        }



        public static readonly DependencyProperty CollapseWidthProperty = DependencyProperty.Register(
            "CollapseWidth", 
            typeof(double), 
            typeof(CollapsibleSearchBox), 
            new PropertyMetadata(0.0)
        );


        public double CollapseWidth
        {
            get { return (double)GetValue(CollapseWidthProperty); }
            set { SetValue(CollapseWidthProperty, value); }
        }


        private AutoSuggestBox myAutoSuggestBox;
        public AutoSuggestBox AutoSuggestBox
        {
            get { return myAutoSuggestBox; }
            private set { myAutoSuggestBox = value; }
        }


        private void CollapsableSearchBox_Loaded(object sender, RoutedEventArgs e)
        {
            RequestedWidth = Width;
            SetState(Window.Current.Bounds.Width);
        }

        private void Current_SizeChanged(object sender, Windows.UI.Core.WindowSizeChangedEventArgs e)
        {
            SetState(e.Size.Width);
        }

        private void SearchBox_LostFocus(object sender, RoutedEventArgs e)
        {
            SetState(Window.Current.Bounds.Width);
            searchButton.IsChecked = false;
        }

        private void SetState(double width)
        {
            if (width <= CollapseWidth)
            {
                VisualStateManager.GoToState(this, "CollapsedState", false);
                Width = 32;
            }
            else
            {
                VisualStateManager.GoToState(this, "OpenState", false);
                Width = RequestedWidth;
            }
        }

        private void SearchButton_Checked(object sender, RoutedEventArgs e)
        {
            VisualStateManager.GoToState(this, "OpenState", false);
            Width = RequestedWidth;
            if (searchBox != null)
            {
                searchBox.Focus(FocusState.Programmatic);
            }
        }
    }
}
