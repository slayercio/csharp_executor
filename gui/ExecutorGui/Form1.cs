using System.Collections;

namespace ExecutorGui
{
    public partial class Form1 : Form
    {
        private PipeClient pipeClient;
        private Execute Execute;

        public Form1()
        {
            pipeClient = new PipeClient();
            Execute = new Execute();
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Task.Run(pipeClient.Initialize);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var names = new List<string>();
            Task.Run(async () =>
            {
                var resources = await pipeClient.ListResources();

                if (resources != null)
                {
                    foreach (var resource in resources.RootElement.EnumerateArray())
                    {
                        names.Add(resource.GetProperty("resource").ToString());
                    }
                }
            }).Wait();

            listBox1.Items.Clear();
            listBox1.Items.AddRange([.. names]);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Execute.ShowDialog(this);
        }
    }
}
