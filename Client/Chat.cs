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

        public Chat(TcpClient client, string name, Connection connectionPage)
        {
            InitializeComponent();
            this.client = client;
            this.connectionPage = connectionPage;
            label1.Text = "Votre nom: " + name;
        }

        // Lors de l'affichage de la fenetre
        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            // On cache la fenetre de connexion
            connectionPage.Hide();
            // On commence la lecture
            client.GetStream().BeginRead(buffer, 0, buffer.Length, OnMessage, null);
        }

        // Lorsque la fenetre se ferme
        protected override void OnClosed(EventArgs e)
        {
            // On ferme la connexion
            client.Close();
            // On réaffiche la fenetre de connexion
            connectionPage.Show();
        }

        private void OnMessage(IAsyncResult result)
        {
            // On valide le resultat et la connextion
            if (!result.IsCompleted || !client.Connected) return;

            int bytes = 0;
            try
            {
                // On tente de lire le message
                bytes = client.GetStream().EndRead(result);

            } catch (IOException e)
            {
                // Si il y a erreur, on affiche un messagebox qui nous indique qu'on a été déconnecté
                MessageBox.Show("Déconnecté du serveur.");
                // On ferme cette fenetre et on affiche la fenetre de connexion
                BeginInvoke((Action)(() => {
                    Close();
                    connectionPage.Show();
                }));
            }

            // Si message est vide, on abandonne
            if (bytes <= 0) return;

            // On va récupérer le message sous forme de string
            byte[] temp = new byte[bytes];
            Array.Copy(buffer, 0, temp, 0, bytes);
            string message = Encoding.ASCII.GetString(temp);

            // On ajoute le message dans la liste et on update la selection pour faire descendre l'affichage
            BeginInvoke((Action)(() =>
            {
                listBox1.Items.Add(message);
                listBox1.SelectedIndex = listBox1.Items.Count - 1;
                listBox1.ClearSelected();
            }));

            // On vide le buffer et on continue la lecture
            Array.Clear(buffer, 0, buffer.Length);
            client.GetStream().BeginRead(buffer, 0, buffer.Length, OnMessage, null);
        }

        private void OnButtonClick(object sender, EventArgs e)
        {
            // On converti le message en byte array, on l'envoie et on vide le textbox
            byte[] message = Encoding.ASCII.GetBytes(textBox1.Text);
            client.GetStream().Write(message, 0, message.Length);
            textBox1.Text = "";
        }
    }
}
