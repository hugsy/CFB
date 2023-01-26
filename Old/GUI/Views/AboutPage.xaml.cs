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

using GUI.Models;
using Windows.System;

namespace GUI.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class AboutPage : Page
    {
        public readonly string GithubRepositoryLabel = "Visit Github Repository";
        public readonly string GithubIssueLabel = "Submit an issue";
        public readonly string GithubPullRequestLabel = "Submit a pull request";

        public AboutPage()
        {
            this.InitializeComponent();
        }


        private async void OnLink_Clicked(object sender, RoutedEventArgs e)
        {
            HyperlinkButton clickedBtn = (HyperlinkButton)e.OriginalSource;
            string buttonLabel = clickedBtn.Content.ToString();

            await Launcher.LaunchUriAsync(new Uri("http://localhost/?"+buttonLabel));

            if (buttonLabel == GithubRepositoryLabel)
                await Launcher.LaunchUriAsync(new Uri(Constants.ProjectUrl));
            else if (buttonLabel == GithubIssueLabel)
                await Launcher.LaunchUriAsync(new Uri(Constants.ProjectIssueUrl));
        }
    }
}
