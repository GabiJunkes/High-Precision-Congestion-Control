#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/qbb-net-device.h"
#include "ns3/random-variable.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "rdma-sim-client.h"
#include <ns3/rdma-driver.h>
#include <stdio.h>
#include <stdlib.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RdmaSimClient");
NS_OBJECT_ENSURE_REGISTERED(RdmaSimClient);

TypeId RdmaSimClient::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RdmaSimClient")
          .SetParent<Application>()
          .AddConstructor<RdmaSimClient>()
          .AddAttribute("WriteSize", "The number of bytes to write per send",
                        UintegerValue(10000),
                        MakeUintegerAccessor(&RdmaSimClient::m_size),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("SourceIP", "Source IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaSimClient::m_sip),
                        MakeIpv4AddressChecker())
          .AddAttribute("DestIP", "Dest IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaSimClient::m_dip),
                        MakeIpv4AddressChecker())
          .AddAttribute("SourcePort", "Source Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimClient::m_sport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("DestPort", "Dest Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimClient::m_dport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("PriorityGroup", "The priority group of this flow",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimClient::m_pg),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("Window", "Bound of on-the-fly packets",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimClient::m_win),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("BaseRtt", "Base Rtt", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimClient::m_baseRtt),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("ProcessTime", "Time to produce data (ns)",
                        UintegerValue(100),
                        MakeUintegerAccessor(&RdmaSimClient::process_time),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("BufferSize", "Max buffer size",
                        UintegerValue(10),
                        MakeUintegerAccessor(&RdmaSimClient::buffer_size),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("DataSize", "Total data to send (bytes)",
                        UintegerValue(100000),
                        MakeUintegerAccessor(&RdmaSimClient::total_data),
                        MakeUintegerChecker<uint64_t>());
  return tid;
}

RdmaSimClient::RdmaSimClient()
    : buffer(0), is_sending(false), sent_data(0) {
  NS_LOG_FUNCTION_NOARGS();
}

RdmaSimClient::~RdmaSimClient() {
  NS_LOG_FUNCTION_NOARGS();
}

void RdmaSimClient::SetRemote(Ipv4Address ip, uint16_t port) {
  m_dip = ip;
  m_dport = port;
}

void RdmaSimClient::SetLocal(Ipv4Address ip, uint16_t port) {
  m_sip = ip;
  m_sport = port;
}

void RdmaSimClient::SetPG(uint16_t pg) {
  m_pg = pg;
}

void RdmaSimClient::SetSize(uint64_t size) {
  m_size = size;
}

void RdmaSimClient::SetNode(Ptr<Node> node) {
  m_node = node;
  m_rdma = m_node->GetObject<RdmaDriver>();
}

void RdmaSimClient::StartApplication(void) {
  NS_LOG_FUNCTION_NOARGS();

  // Inicializa produção e consumo
  Simulator::Schedule(NanoSeconds(process_time),
                      MakeEvent(&RdmaSimClient::Process, this));
  Simulator::Schedule(NanoSeconds(0),
                      MakeEvent(&RdmaSimClient::Consume, this));
}

void RdmaSimClient::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
  // Aqui poderia ser adicionado cancelamento de eventos, se desejado
}

void RdmaSimClient::Process() {
  NS_LOG_FUNCTION_NOARGS();
  if (buffer < buffer_size) {
    buffer += 1;
  } else {
    // buffer cheio, não produz
  }

  Simulator::Schedule(NanoSeconds(process_time),
                      MakeEvent(&RdmaSimClient::Process, this));
}

void RdmaSimClient::Consume() {
  NS_LOG_FUNCTION_NOARGS();
  if (!is_sending && buffer > 0) {
    is_sending = true;

    m_rdma->AddQueuePair(m_size, m_pg, m_sip, m_dip, m_sport, m_dport, m_win,
                         m_baseRtt,
                         MakeCallback(&RdmaSimClient::Finish, this));
    buffer -= 1;
  }

  Simulator::Schedule(NanoSeconds(0),
                      MakeEvent(&RdmaSimClient::Consume, this));
}

// Chamado toda vez que termina de enviar
void RdmaSimClient::Finish() {
  NS_LOG_FUNCTION_NOARGS();
  is_sending = false;
}

void RdmaSimClient::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

} // namespace ns3
