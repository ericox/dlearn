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
using bufstreamingrpc::SendBufResponse;
using bufstreamingrpc::SendBufRequest;

const int BUFSIZE = 1500000000; // maximum buffer size of 128 MB    

class BufferServiceClient {
 public:
    BufferServiceClient(std::shared_ptr<Channel> channel)
	: stub_(BufferService::NewStub(channel)) {
      	      buf = new char[BUFSIZE];
	      std::fill(buf, buf + BUFSIZE, 'a');
    };
  ~BufferServiceClient() { free(buf); };

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  DataResponse Recv(const int& payload_size, const bool& debug) {
    // Data we are sending to the server.
    BufRequest request;
    request.set_payload_size(payload_size);
    if (debug) {
        request.set_debug(true);
    } else {
        request.set_debug(false);
    }

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

  SendBufResponse SendBuf(const int& payload_size, const bool debug) {
    SendBufRequest request;
    std::string s1(buf, payload_size);
    request.set_buf(s1);
    request.set_debug(debug);
    request.set_payload_size(payload_size);

    SendBufResponse reply;
    ClientContext context;
    Status status = stub_->SendBuf(&context, request, &reply);
    if (status.ok()) {
      return reply;
    } else {
      return reply;
    }
  }
  
  void RunBench(const int& payload_size, std::string filename, int nruns) {
    int i;
    // DataResponse reply;
    // Run one request with debug true to dump message on server
    //reply = Recv(payload_size, true);

    SendBufResponse reply;
    reply = SendBuf(payload_size, true);
    
    // Time roundtrip for sending a buffer of chars payload_size in length.
    steady_clock::time_point t1 = steady_clock::now();
    for(i = 0; i < nruns; i++)
      //reply = Recv(payload_size, false);
      reply = SendBuf(payload_size, false);
    steady_clock::time_point t2 = steady_clock::now();
    /*
    std::cout  << reply.ByteSize()
	       << ", " << reply.val().size()
	       << ", " << ((float)(nanoseconds(t2-t1).count()) / 1e9) / nruns << std::endl;
    */
    std::cout  << reply.ByteSize()
	       << ", " << payload_size
	       << ", " << ((float)(nanoseconds(t2-t1).count()) / 1e9) / nruns << std::endl;

    //WritePayload(filename, &reply);
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
    std::unique_ptr<BufferService::Stub> stub_;
    // refrence to a buffer of 4 byte uints
    int nmax;
  char *buf;
};

int main(int argc, char** argv) {
    if (argc < 3) {
	std::cout << "usage: bufservice_client BUFSIZE DEBUGFILE" << std::endl;
	return 1;
    }
    int payload_size = atoi(argv[1]);

    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    BufferServiceClient bufservice(grpc::CreateChannel(
	    "geeker-4.news.cs.nyu.edu:50051", grpc::InsecureChannelCredentials()));

    // Time a single rpc recv-send for a buffer size of n and an average of 100
    // requests.
    bufservice.RunBench(payload_size, std::string(argv[2]), 100);

    return 0;
}
