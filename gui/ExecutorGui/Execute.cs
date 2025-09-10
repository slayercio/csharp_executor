using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ExecutorGui
{
    public partial class Execute : Form
    {
        public Execute()
        {
            InitializeComponent();
        }

        private void Execute_Shown(object sender, EventArgs e)
        {
            var names = new List<string>();

            if (PipeClient.Instance != null)
            {
                Task.Run(async () =>
                {
                    var resources = await PipeClient.Instance.ListResources();

                    if (resources != null)
                    {
                        foreach (var resource in resources.RootElement.EnumerateArray())
                        {
                            names.Add(resource.GetProperty("resource").ToString());
                        }
                    }
                }).Wait();
            }

            listBox1.Items.Clear();
            listBox1.Items.AddRange([.. names]);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var item = listBox1.SelectedItem;
            if (item == null)
            {
                return;
            }

            if (PipeClient.Instance == null)
            {
                return;
            }

            string? filePath = null;
            using (var ofd = new OpenFileDialog())
            {
                ofd.Title = "Select assembly to execute!";
                ofd.Filter = "Dll files (*.dll)|*.dll";
                if (ofd.ShowDialog() == DialogResult.OK)
                {
                    filePath = ofd.FileName;
                }
            }

            if (filePath == null)
            {
                return;
            }

            Task.Run(async () =>
            {
                await PipeClient.Instance.ExecuteInResource(item.ToString(), filePath);
            }).Wait();

            this.Hide();
        }
    }
}
