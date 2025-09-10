using System.IO.Pipes;
using System.Text;
using System.Text.Json;

namespace ExecutorGui
{
    internal class PipeClient
    {
        private const string IPC_PIPE_NAME = "my_ipc_pipe";
        private NamedPipeClientStream pipeClientStream;

        public static PipeClient? Instance { get; private set; }

        private static async Task SendJsonAsync(NamedPipeClientStream pipe, string serialized)
        {
            byte[] payload = Encoding.UTF8.GetBytes(serialized);
            byte[] lengthPrefix = BitConverter.GetBytes((UInt32) payload.Length);

            await pipe.WriteAsync(lengthPrefix, 0, lengthPrefix.Length);
            await pipe.WriteAsync(payload, 0, payload.Length);
            await pipe.FlushAsync();
        }

        private static async Task<JsonDocument?> ReadJsonAsync(NamedPipeClientStream pipe)
        {
            byte[] lenBuf = new byte[4];
            int read = await pipe.ReadAsync(lenBuf, 0, 4);
            if (read < 4)
            {
                return null;
            }

            int length = BitConverter.ToInt32(lenBuf, 0);
            if (length < 0)
            {
                return null;
            }

            byte[] buffer = new byte[length];
            int offset = 0;

            while (offset < length)
            {
                int chunk = await pipe.ReadAsync(buffer, offset, length - offset);
                if (chunk <= 0)
                {
                    return null;
                }

                offset += chunk;
            }

            return JsonDocument.Parse(buffer);
        }

        public PipeClient()
        {
            pipeClientStream = new NamedPipeClientStream(
                ".",
                IPC_PIPE_NAME,
                PipeDirection.InOut,
                PipeOptions.Asynchronous
            );

            Instance = this;
        }

        public async Task Initialize()
        {
            Console.WriteLine("connecting to server!");
            await pipeClientStream.ConnectAsync(5000);
            Console.WriteLine("connected!");
        }

        public async Task<JsonDocument?> ListResources()
        {
            var req = JsonSerializer.Serialize(new
            {
                cmd = "list_resources_with_runtimes"
            }); 

            await SendJsonAsync(pipeClientStream, req);
            var res = await ReadJsonAsync(pipeClientStream);
            Console.WriteLine(res);

            return res;
        }
        
        public async Task ExecuteInResource(string resourceName, string filePath)
        {
            var serialized = JsonSerializer.Serialize(new
            {
                cmd = "execute_in_resource",
                resource = resourceName,
                scriptFilePath = filePath
            });

            await SendJsonAsync(pipeClientStream, serialized);
            var res = await ReadJsonAsync(pipeClientStream);
            Console.WriteLine(res);
        }
    }
}
