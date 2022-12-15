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
            // On valide le nom entré
            string name = textBox2.Text;
            if (string.IsNullOrEmpty(name))
                MessageBox.Show("Vous devez entrer un nom pour vous connecter!");
            else if (name.Contains(' '))
                MessageBox.Show("Votre nom ne peut contenir d'espaces!");
            else if (name.Contains(','))
                MessageBox.Show("Votre nom ne peut contenir de virgules!");
            else
            {
                try
                {
                    // On tente d'établir une connexion et on envoie notre nom comme premier message
                    TcpClient client = new TcpClient(textBox1.Text, (int)numericUpDown1.Value);
                    byte[] nameMsg = Encoding.ASCII.GetBytes(name);
                    client.GetStream().Write(nameMsg, 0, nameMsg.Length);

                    // On créer une fenetre de chat auquel on envoit le client, notre nom et cette fenetre
                    Chat chat = new Chat(client, name, this);
                    // On affiche la fenetre
                    chat.Show();

                }
                catch (SocketException ex)
                {
                    MessageBox.Show("La connection au serveur a échouée.");
                }
            }
        }
    }
}
