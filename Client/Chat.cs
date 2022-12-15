using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;

namespace Client
{
    public partial class Chat : Form
    {

        TcpClient client;
        byte[] buffer = new byte[4096];
        Connection connectionPage;

        public Chat(TcpClient client, Connection connectionPage)
        {
            InitializeComponent();
            this.client = client;
            this.connectionPage = connectionPage;
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            connectionPage.Hide();
            client.GetStream().BeginRead(buffer, 0, buffer.Length, OnMessage, null);
        }

        protected override void OnClosed(EventArgs e)
        {
            client.Close();
            connectionPage.Show();
        }

        private void OnMessage(IAsyncResult result)
        {
            if (!result.IsCompleted) return;

            if (!client.Connected) return;

            int bytes = 0;
            try
            {
                bytes = client.GetStream().EndRead(result);

            } catch (IOException e)
            {
                MessageBox.Show("Déconnecté du serveur.");
                BeginInvoke((Action)(() => {
                    Close();
                    connectionPage.Show();
                }));
            }

            if (bytes <= 0) return;

            byte[] temp = new byte[bytes];
            Array.Copy(buffer, 0, temp, 0, bytes);
            string message = Encoding.ASCII.GetString(temp);

            BeginInvoke((Action)(() =>
            {
                listBox1.Items.Add(message);
                listBox1.SelectedIndex = listBox1.Items.Count - 1;
                listBox1.ClearSelected();
            }));

            Array.Clear(buffer, 0, buffer.Length);
            client.GetStream().BeginRead(buffer, 0, buffer.Length, OnMessage, null);
        }

        private void OnButtonClick(object sender, EventArgs e)
        {
            byte[] message = Encoding.ASCII.GetBytes(textBox1.Text);
            client.GetStream().Write(message, 0, message.Length);
            textBox1.Text = "";
        }
    }
}
