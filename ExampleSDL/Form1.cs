using System.Runtime.InteropServices;
using System.Threading;
namespace ExampleSDL
{
    public partial class Form1 : Form
    {
        [DllImport(@"SDLCall.dll")]
        static extern void create_window(IntPtr window);
        [DllImport(@"SDLCall.dll")]
        static extern void play_video();

        bool started_playing = false;
        public Form1()
        {
            InitializeComponent();
            IntPtr hwnd = pictureBox1.Handle;
            Task.Run(() => create_window(hwnd));
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
        }

        private void button1_Click(object sender, EventArgs e)
        {
            play_video();
        }
    }
}
