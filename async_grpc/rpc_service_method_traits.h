/*
 * Copyright 2017 The Cartographer Authors
 * Copyright 2021 Kyle Ambroff-Kao <kyle@ambroffkao.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <flatbuffers/grpc.h>

#include "async_grpc/type_traits.h"

namespace async_grpc {

DEFINE_HAS_SIGNATURE(has_service_method_name, T::MethodName,
                     const char* (*)(void));

DEFINE_HAS_MEMBER_TYPE(has_incoming_type, IncomingType);
DEFINE_HAS_MEMBER_TYPE(has_outgoing_type, OutgoingType);

// The RPC service method concept describes types from which properties of an
// RPC service can be inferred. The type RpcServiceMethod satisfies the RPC
// service concept if:
//   1) it provides a static member function ' const char* MethodName()'
//   2) it provides an 'IncomingType' typedef; i.e. the proto message passed to
//      the service method
//   3) it provides an 'OutgoingType' typedef; i.e. the proto message passed to
//      the service method
// Note: the IncomingType and OutgoingType specified above may be wrapped (or
//       tagged) by async_grpc::Stream.
template <typename RpcServiceMethodConcept>
struct RpcServiceMethodTraits {
  static_assert(has_service_method_name<RpcServiceMethodConcept>::value,
                "The RPC service method concept must provide a static member "
                "'const char* MethodName()'.");

  static_assert(has_incoming_type<RpcServiceMethodConcept>::value,
                "The RPC service method concept must provide an IncomingType.");

  static_assert(has_outgoing_type<RpcServiceMethodConcept>::value,
                "The RPC service method concept must provide an OutgoingType.");

  // Returns the fully qualified name of the gRPC method this handler is
  // implementing. The fully qualified name has the structure
  // '/<<full service name>>/<<method name>>', where the service name is the
  // fully qualified proto package name of the service and method name the
  // name of the method as defined in the service definition of the proto.
  static constexpr const char* MethodName() {
    return RpcServiceMethodConcept::MethodName();
  }

  // An object derived from ::flatbuffers::Table which is passed to a
  // specific service method.
  using RequestType =
      StripStream<typename RpcServiceMethodConcept::IncomingType>;

  // An object derived from ::flatbuffers::Table which is returned from a
  // specific service method.
  using ResponseType =
      StripStream<typename RpcServiceMethodConcept::OutgoingType>;

  static_assert(std::is_base_of<::flatbuffers::Table, RequestType>::value,
                "The RPC request type must be derived from "
                "::flatbuffers::grpc::Message.");

  static_assert(std::is_base_of<::flatbuffers::Table, ResponseType>::value,
                "The RPC response type must be derived from "
                "::flatbuffers::grpc::Message.");

  // The streaming type of the service method. See also
  // ::grpc::internal::RpcMethod.
  static constexpr auto StreamType =
      RpcType<typename RpcServiceMethodConcept::IncomingType,
              typename RpcServiceMethodConcept::OutgoingType>::value;
};

}  // namespace async_grpc
