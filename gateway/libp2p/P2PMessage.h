/*
 *  Copyright (C) 2021 FISCO BCOS.
 *  SPDX-License-Identifier: Apache-2.0
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @file P2PMessage.h
 * @author: octopus
 * @date 2021-05-04
 */

#pragma once

#include <bcos-framework/libutilities/Common.h>
#include <gateway/libnetwork/Common.h>
#include <gateway/libnetwork/Message.h>

namespace bcos {
namespace gateway {

// Message type definition
enum MessageType {
  Heartbeat = 0x1,
  Handshake = 0x2,
  RequestNodeIDs = 0x3,
  ResponseNodeIDs = 0x4,
  PeerToPeerMessage = 0x5,
  BroadcastMessage = 0x6
};

enum MessageExtFieldFlag {
  Response = 0x0001,
};

enum MessageDecodeStatus {
  MESSAGE_ERROR = -1,
  MESSAGE_INCOMPLETE = 0,
};

/// Options format definition
///   options(default version):
///       groupID length    :1 bytes
///       groupID           : bytes
///       nodeID length     :2 bytes
///       src nodeID        : bytes
///       src nodeID count  :1 bytes
///       dst nodeIDs       : bytes
class P2PMessageOptions {
public:
  using Ptr = std::shared_ptr<P2PMessageOptions>;
  /// groupID length(1) + nodeID length(2) + dst nodeID count(1)
  const static size_t OPTIONS_MIN_LENGTH = 5;

public:
  P2PMessageOptions() { m_srcNodeID = std::make_shared<bytes>(); }

  virtual ~P2PMessageOptions() {}

  /// The maximum gateway transport protocol supported groupID length  65535
  const static size_t MAX_GROUPID_LENGTH = 65535;
  /// The maximum gateway transport protocol supported nodeID length  65535
  const static size_t MAX_NODEID_LENGTH = 65535;
  /// The maximum gateway transport protocol supported dst nodeID count  127
  const static size_t MAX_DST_NODEID_COUNT = 255;

  bool encode(bytes &_buffer);
  ssize_t decode(bytesConstRef _buffer);

public:
  std::string groupID() const { return m_groupID; }
  void setGroupID(const std::string &_groupID) { m_groupID = _groupID; }

  std::shared_ptr<bytes> srcNodeID() const { return m_srcNodeID; }
  void setSrcNodeID(std::shared_ptr<bytes> _srcNodeID) {
    m_srcNodeID = _srcNodeID;
  }

  std::vector<std::shared_ptr<bytes>> &dstNodeIDs() { return m_dstNodeIDs; }
  void setDstNodeIDs(const std::vector<std::shared_ptr<bytes>> &_dstNodeIDs) {
    m_dstNodeIDs = _dstNodeIDs;
  }

protected:
  std::string m_groupID;
  std::shared_ptr<bytes> m_srcNodeID;
  std::vector<std::shared_ptr<bytes>> m_dstNodeIDs;
};

/// Message format definition of gateway P2P network
///
/// fields:
///   length            :4 bytes
///   version           :2 bytes
///   packet type       :2 bytes
///   seq               :4 bytes
///   ext               :2 bytes
///   options(default version):
///       groupID length    :1 bytes
///       groupID           : bytes
///       nodeID length     :2 bytes
///       src nodeID        : bytes
///       src nodeID count  :1 bytes
///       dst nodeIDs       : bytes
///   payload           :X bytes
class P2PMessage : public Message {
public:
  using Ptr = std::shared_ptr<P2PMessage>;

  /// length(4) + version(2) + packetType(2) + seq(4) + ext(2)
  const static size_t MESSAGE_HEADER_LENGTH = 14;

public:
  P2PMessage() {
    m_payload = bytesConstRef();
    m_options = std::make_shared<P2PMessageOptions>();
  }

  virtual ~P2PMessage() {}

public:
  virtual uint32_t length() const override { return m_length; }
  virtual void setLength(uint32_t length) { m_length = length; }

  virtual uint16_t version() const override { return m_version; }
  virtual void setVersion(uint16_t version) { m_version = version; }

  virtual uint16_t packetType() const override { return m_packetType; }
  virtual void setPacketType(uint16_t packetType) { m_packetType = packetType; }

  virtual uint32_t seq() const override { return m_seq; }
  virtual void setSeq(uint32_t seq) { m_seq = seq; }

  virtual uint16_t ext() const override { return m_ext; }
  virtual void setExt(uint16_t _ext) { m_ext = _ext; }

  P2PMessageOptions::Ptr options() const { return m_options; }
  void setOptions(P2PMessageOptions::Ptr _options) { m_options = _options; }

  bytesConstRef payload() const { return m_payload; }
  void setPayload(bytesConstRef _payload) { m_payload = _payload; }

public:
  ssize_t decodeHeader(bytesConstRef _buffer);
  void setRespPacket() { m_ext |= MessageExtFieldFlag::Response; }
  bool hasOptions() const {
    return (m_packetType == MessageType::PeerToPeerMessage) ||
           (m_packetType == MessageType::BroadcastMessage);
  }

  virtual bool encode(bytes &_buffer) override;
  virtual ssize_t decode(bytesConstRef _buffer) override;
  virtual bool isRespPacket() const override {
    return (m_ext & MessageExtFieldFlag::Response) != 0;
  }

protected:
  uint32_t m_length = 0;
  uint16_t m_version = 0;
  uint16_t m_packetType = 0;
  uint32_t m_seq = 0;
  uint16_t m_ext = 0;

  P2PMessageOptions::Ptr m_options; ///< options fields

  bytesConstRef m_payload; ///< payload data
};

class P2PMessageFactory : public MessageFactory {
public:
  using Ptr = std::shared_ptr<P2PMessageFactory>;
  virtual ~P2PMessageFactory() {}

public:
  virtual Message::Ptr buildMessage() {
    auto message = std::make_shared<P2PMessage>();
    return message;
  }
};

inline std::ostream &operator<<(std::ostream &_out,
                                const P2PMessage _p2pMessage) {
  _out << "P2PMessage {"
       << " length: " << _p2pMessage.length()
       << " version: " << _p2pMessage.version()
       << " packetType: " << _p2pMessage.packetType()
       << " seq: " << _p2pMessage.seq() << " ext: " << _p2pMessage.ext()
       << " }";
  return _out;
}

inline std::ostream &operator<<(std::ostream &_out,
                                P2PMessage::Ptr _p2pMessage) {
  _out << (*_p2pMessage.get());
  return _out;
}

} // namespace gateway
} // namespace bcos
