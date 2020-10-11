#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include <unistd.h>
#include <cstdlib>
#include <signal.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "greet.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using greet::Greeter;
using greet::HelloReply;
using greet::HelloRequest;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service
{
	Status SayHello(ServerContext *context, const HelloRequest *request,
					HelloReply *reply) override
	{
		std::cout << "Message received: " << request->name() << std::endl;
		
		//Do some job
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		//send the response to the client
		std::string prefix("Hello ");
		reply->set_message(prefix + request->name());

		return Status::OK;
	}
};

void RunServer()
{
	//The address to try to bind to the server in URI form. If the scheme name is omitted, "dns:///" is assumed. 
	//To bind to any address, please use IPv6 any, i.e., [::]:<port>, which also accepts IPv4 connections. 
	//Valid values include dns:///localhost:1234, / 192.168.1.1:31416, dns:///[::1]:27182, etc.).
	std::string server_address("[::]:5051");
	GreeterServiceImpl service;

	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	ServerBuilder builder;
	
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	
	// Finally assemble the server.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}

void signal_callback_handler(int signum)
{
	std::cout << "Terminating... signal(" << signum << ")" << std::endl;
	exit(signum);
}

int main(int argc, char **argv)
{
	signal(SIGINT, signal_callback_handler);

	RunServer();

	return 0;
}