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
messages each in a single `DataResponse` message.

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
   int32 payload_size = 1;
}

// The response message containing the buffer data
message DataResponse {
  fixed32 val  = 1;
}
```

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

### Running Benchmarks

- Assuming that a `bufservice_server` is running on a machine to run the benchmarks

    ```bash
    cd dlearn/grpc_benchmarks
    ./run_send_buffer.sh
    ```

- This writes a results file to the current working dir with data format

   ```
   N, Bytes,  t base (s), t send (s)
   ```

where

- N is the number of DataResponses streamed from server to client
- Bytes is the number of bytes sent
- t base is the time to send zero bytes (handshaking etc...)
- t send is the elapsed time to complete a send request from client to server


