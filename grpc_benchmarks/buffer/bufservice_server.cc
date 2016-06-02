/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <grpc++/grpc++.h>

#include "bufstreaming.grpc.pb.h"

#define BUFSIZE 1000000000 // maximum buffer size of 10 GB 

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using bufstreamingrpc::BufRequest;
using bufstreamingrpc::DataResponse;
using bufstreamingrpc::BufferService;

// Logic and data behind the server's behavior.
class BufferServiceImpl final : public BufferService::Service {
 public:
    BufferServiceImpl() {
	int i;
	// create a buffer char * and every reply create a string * that points
	// to the buffer using std::string::string. Reply with this buffer.
	char *buf = new char[BUFSIZE];
	std::fill(buf, buf + BUFSIZE, '0');

    };
    ~BufferServiceImpl() {
	free(buf);
    };
    
  Status Send(ServerContext* context, const BufRequest* request,
	      ServerWriter<DataResponse>* writer) override {
      int i;
      long int n = request->payload_size();
      long int nsent = 0;
      std::cout << "payload size: " << n << std::endl;
      // Repeatedly send 64KB sized chunks. The protobuf docs mention that performance
      // is poor if the message sizes are too large. Sending in chunks reduces overhead
      // of a header per element if we send single values.
      //    buf->add_val(i);
      //  writer->Write(*buf);
    return Status::OK;
  }

  void PrintBuf() {
      int i;
  }
  
 private:
  char *buf;
};


void RunServer() {
  std::string server_address("0.0.0.0:50051");
  BufferServiceImpl service;

  service.PrintBuf();
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

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
