using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Client
{
    public partial class Connection : Form
    {
        public Connection()
        {
            InitializeComponent();
        }

        private void OnButtonClick(object sender, EventArgs e)
        {
            string ip = textBox1.Text;
            int port = (int) numericUpDown1.Value;
            string name = textBox2.Text;

            try
            {
                TcpClient client = new TcpClient(ip, port);
                byte[] nameMsg = Encoding.ASCII.GetBytes(name);
                client.GetStream().Write(nameMsg, 0, nameMsg.Length);
                Chat chat = new Chat(client, this);
                chat.Show();

            }
            catch (SocketException ex)
            {
                MessageBox.Show("La connection au serveur a échouée.\n");
            }
        }
    }
}
