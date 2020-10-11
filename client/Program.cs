using System;
using Grpc.Net.Client;
using System.Threading.Tasks;
using Grpc.Core;
using Microsoft.Extensions.Logging;

namespace client
{
    class Program
    {
        static async Task Main(string[] args)
        {
			using var loggerFactory = LoggerFactory.Create(builder =>
            {
                builder
                    .AddFilter("client.Program", LogLevel.Debug)
					.AddFilter("Grpc", LogLevel.Debug)
                    .AddConsole();
            });
			var logger = loggerFactory.CreateLogger<Program>();

			AppContext.SetSwitch("System.Net.Http.SocketsHttpHandler.Http2UnencryptedSupport", true);
			// AppContext.SetSwitch("System.Net.Http.SocketsHttpHandler.Http2Support", true);

            var options = new GrpcChannelOptions();
			options.Credentials = ChannelCredentials.Insecure;
			options.LoggerFactory = loggerFactory;

			using var channel = GrpcChannel.ForAddress("http://localhost:5051", options);
			var client = new Greeter.GreeterClient(channel);
			
            var input = string.Empty;
            while(true) {
				Console.WriteLine("Press enter the message to send or type `quit` to exit");
				input = Console.ReadLine();

				if (input.Equals("quit"))
					break;

                try
                {
					var reply = await client.SayHelloAsync(
				        new HelloRequest { Name = input },
				        deadline: DateTime.UtcNow.AddSeconds(30));

					Console.WriteLine("Response received: " + reply.Message);
				}
				catch (RpcException ex) when (ex.StatusCode == StatusCode.DeadlineExceeded)
				{
					logger.LogError(ex, "Sending message timed out.");
				}
            }
            while (input != "quit");
        }
    }
}
