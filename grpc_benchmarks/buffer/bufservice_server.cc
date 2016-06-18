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
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <grpc++/grpc++.h>

#include "bufstreaming.grpc.pb.h"
#include "grpc_byte_buffer.h"

#define BUFSIZE 1000000000 // maximum buffer size of 128 MB 

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
	      buf = new char[BUFSIZE];
	      std::fill(buf, buf + BUFSIZE, 'a');
    };
    ~BufferServiceImpl() {
	free(buf);
    };

   // Send encodes data in buf on the fly to send to client.
   Status Send(ServerContext* context, const BufRequest* request,
	      DataResponse* resp) override {
     int n = request->payload_size();
     std::string s1 (buf, n);
     resp->set_val(s1);
     if (request->debug()) {
       WritePayload("debug_payload_srv.out", resp);
     }
     return Status::OK;
  }
   
  int WritePayload(std::string filename, DataResponse* resp) {
      std::fstream output(filename, std::ios::out | std::ios::binary);
      if (!resp->SerializeToOstream(&output)) {
          std::cerr << "Failed to write buffer." << std::endl;
          return -1;
      }
      return 0;
  }
   
 private:
  char *buf;
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  BufferServiceImpl service;

  ServerBuilder builder;

  // Set max message size in bytes. This allows us to handle large message sizes.
  std::cout << "SettingMaxMessageSize to " << BUFSIZE << std::endl;
  builder.SetMaxMessageSize(BUFSIZE);
  
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
