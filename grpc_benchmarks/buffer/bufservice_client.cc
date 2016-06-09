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
#include <chrono>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

#include <grpc++/grpc++.h>

#include "bufstreaming.grpc.pb.h"

using namespace std::chrono;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using bufstreamingrpc::BufRequest;
using bufstreamingrpc::DataResponse;
using bufstreamingrpc::BufferService;

    
class BufferServiceClient {
 public:
    BufferServiceClient(std::shared_ptr<Channel> channel)
	: stub_(BufferService::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  DataResponse Recv(const int& payload_size) {
    // Data we are sending to the server.
    BufRequest request;
    request.set_payload_size(payload_size);

    // Container for the data we expect from the server.
    DataResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // get buffer from server from reader.
    Status status = stub_->Send(&context, request, &reply);
    if (status.ok()) {
      return reply;
    } else {
      return reply;
    }
  }
    
  void RunBench(const int& payload_size, int nruns) {
    DataResponse reply;
    int i;
    steady_clock::time_point t1 = steady_clock::now();
    for(i = 0; i < 100; i++)
        reply = Recv(payload_size);
    steady_clock::time_point t2 = steady_clock::now();
    std::cout  << reply.ByteSize()
	       << ", " << reply.val().size()
	       << ", " << ((float)(nanoseconds(t2-t1).count()) / 1e9) / 100 << std::endl;
  }
    
  void WritePayload(std::string filename) {}
  std::string md5sum(std::string filename) {}

 private:
    std::unique_ptr<BufferService::Stub> stub_;
    // refrence to a buffer of 4 byte uints
    int nmax;
};

int main(int argc, char** argv) {
    if (argc < 2) {
	std::cout << "usage: bufservice_client BUFSIZE" << std::endl;
	return 1;
    }
    int payload_size = atoi(argv[1]);

    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    BufferServiceClient bufservice(grpc::CreateChannel(
	    "localhost:50051", grpc::InsecureChannelCredentials()));

    // Time a single rpc recv-send for a buffer size of n and an average of 100
    // requests.
    bufservice.RunBench(payload_size, 100);
    return 0;
}
