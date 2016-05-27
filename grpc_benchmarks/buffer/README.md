# gRPC C++ Buffer Service

### Install gRPC
Make sure you have installed gRPC on your system. Follow the instructions here:
[https://github.com/grpc/grpc/blob/master/INSTALL](../../../INSTALL.md).

### Defining a buffer service for benchmarking grpc

Here's our service definition, defined using protocol buffers IDL in
[bufservice.proto](./bufservice.proto). The `BufferService`
service has one method, `send`, that lets the server receive a single
`BufRequest` message from the remote client containing the desired number of
unsigned long ints, then the server send a streaming response containing n
messages each in a single `DataReply` message.

```protobuf
syntax = "proto3";

package bufstreamingrpc;

// The BufferReader service definition for a unary client streaming server.
service BufferService {
  // Sends a request to read n unsigned long ints from a remote buffer
  rpc Send (ReadRequest) returns (stream DataResponse) {}
}

// The request message containing the number of bytes to read.
message BufRequest {
   int32 n = 1;
}

// The response message containing the buffer data
message DataResponse {
  fixed32 val  = 1;
}
```

<a name="generating"></a>
### Generating gRPC code

To generate the client and server side interfaces:

```sh
$ make bufstreaming.grpc.pb.cc bufstreaming.pb.cc
```
Which internally invokes the proto-compiler as:

```sh
$ protoc -I ../../protos/ --grpc_out=. --plugin=protoc-gen-grpc=grpc_cpp_plugin ./bufstreaming.proto
$ protoc -I ../../protos/ --cpp_out=. ./bufstreaming.proto
```

### Writing a client

- Create a channel. A channel is a logical connection to an endpoint. A gRPC
  channel can be created with the target address, credentials to use and
  arguments as follows

    ```cpp
    auto channel = CreateChannel("localhost:50051", InsecureChannelCredentials());
    ```

- Create a stub. A stub implements the rpc methods of a service and in the
  generated code, a method is provided to created a stub with a channel:

    ```cpp
    auto stub = bufservice::BufferService::NewStub(channel);
    ```

- Make a unary rpc, with `ClientContext` and request/response proto messages.

    ```cpp
    ClientContext context;
    ReadRequest request;
    request.set_n(1000);
    Data reply;
    Status status = stub->Read(&context, request, &reply);
    ```

- Check returned status and response.

    ```cpp
    if (status.ok()) {
      // check reply.message()
    } else {
      // rpc failed.
    }
    ```

For a working example, refer to [greeter_client.cc](greeter_client.cc).

### Writing a server

- Implement the service interface

    ```cpp
    class GreeterServiceImpl final : public Greeter::Service {
      Status SayHello(ServerContext* context, const HelloRequest* request,
          HelloReply* reply) override {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return Status::OK;
      }
    };

    ```

- Build a server exporting the service

    ```cpp
    GreeterServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    ```



