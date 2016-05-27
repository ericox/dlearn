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
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include <grpc++/grpc++.h>

#include "bufstreaming.grpc.pb.h"

#define NBZERO 1000 // benchmark iterations for handshake overhead

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using bufstreamingrpc::BufRequest;
using bufstreamingrpc::DataResponse;
using bufstreamingrpc::BufferService;

class BufferServiceClient {
 public:
    BufferServiceClient(std::shared_ptr<Channel> channel, int n)
      : stub_(BufferService::NewStub(channel)) {
	nmax = n;
	buf = new unsigned int[nmax];
  }

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  int Send(const long int& ndata) {
    // Data we are sending to the server.
    BufRequest request;
    request.set_n(ndata);

    // Container for the data we expect from the server.
    DataResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Read byte stream from reader.
    std::unique_ptr<ClientReader<DataResponse> > reader(stub_->Send(&context, request));
    int n = 0;
    while(reader->Read(&reply)) {
	buf[n % nmax] = reply.val(); // rewrite buffer using mod
	n++;
    }
    Status status = reader->Finish();
    
    // Act upon its status.
    if (status.ok()) {
      return n;
    } else {
      return -1;
    }
  }

  void PrintBuf(const int n) {
      int i;
      for (i = 0; i < n; i++)
	  std::cout << buf[i] << std::endl;
  }
    
 private:
    std::unique_ptr<BufferService::Stub> stub_;
    // refrence to a buffer of 4 byte uints
    unsigned int *buf;
    int nmax;
};

int main(int argc, char** argv) {
    int i, reply;
    clock_t t;
    int nints, b;
    if (argc < 3) {
	std::cout << "usage: bufservice_client NINTS BENCHMARKITER" << std::endl;
	return 1;
    }
    nints = atoi(argv[1]);
    b = atoi(argv[2]);

    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    BufferServiceClient bufservice(grpc::CreateChannel(
	    "localhost:50051", grpc::InsecureChannelCredentials()), 1638400); // max 64 kB

    // run benchmark to measure roundtrip overhead.
    t = clock();
    for (i = 0; i < NBZERO; i++)
	reply = bufservice.Send(0);
    double tzero = ((double)clock() - (double)t)/CLOCKS_PER_SEC;

    // run benchmark for send
    t = clock();
    for (i = 0; i < b; i++)
	reply = bufservice.Send(nints);
    double tsend = ((double)clock() - (double)t)/CLOCKS_PER_SEC;

    std::cout  << reply
	       << ", " << reply*4
	       << ", " << tzero / NBZERO
	       << ", " << tsend / ((double)b) << std::endl;
    
    return 0;
}

