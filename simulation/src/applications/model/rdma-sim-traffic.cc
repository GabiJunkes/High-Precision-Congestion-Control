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
#include "rdma-sim-traffic.h"
#include <ns3/rdma-driver.h>
#include <stdio.h>
#include <stdlib.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RdmaSimTraffic");
NS_OBJECT_ENSURE_REGISTERED(RdmaSimTraffic);

TypeId RdmaSimTraffic::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RdmaSimTraffic")
          .SetParent<Application>()
          .AddConstructor<RdmaSimTraffic>()
          .AddAttribute("WriteSize", "The number of bytes to write per send",
                        UintegerValue(10000),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_size),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("SourceIP", "Source IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaSimTraffic::m_sip),
                        MakeIpv4AddressChecker())
          .AddAttribute("DestIP", "Dest IP", Ipv4AddressValue("0.0.0.0"),
                        MakeIpv4AddressAccessor(&RdmaSimTraffic::m_dip),
                        MakeIpv4AddressChecker())
          .AddAttribute("SourcePort", "Source Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_sport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("DestPort", "Dest Port", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_dport),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("PriorityGroup", "The priority group of this flow",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_pg),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute("Window", "Bound of on-the-fly packets",
                        UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_win),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("BaseRtt", "Base Rtt", UintegerValue(0),
                        MakeUintegerAccessor(&RdmaSimTraffic::m_baseRtt),
                        MakeUintegerChecker<uint64_t>());
  return tid;
}

RdmaSimTraffic::RdmaSimTraffic() {
  NS_LOG_FUNCTION_NOARGS();
}

RdmaSimTraffic::~RdmaSimTraffic() {
  NS_LOG_FUNCTION_NOARGS();
}

void RdmaSimTraffic::SetRemote(Ipv4Address ip, uint16_t port) {
  m_dip = ip;
  m_dport = port;
}

void RdmaSimTraffic::SetLocal(Ipv4Address ip, uint16_t port) {
  m_sip = ip;
  m_sport = port;
}

void RdmaSimTraffic::SetPG(uint16_t pg) {
  m_pg = pg;
}

void RdmaSimTraffic::SetSize(uint64_t size) {
  m_size = size;
}

void RdmaSimTraffic::SetNode(Ptr<Node> node) {
  m_node = node;
  m_rdma = m_node->GetObject<RdmaDriver>();
}

void RdmaSimTraffic::StartApplication(void) {
  printf("Started traffic\n");
  NS_LOG_FUNCTION_NOARGS();
  for (uint16_t i = 0; i < 50; i++){
    m_rdma->AddQueuePair(m_size, m_pg, m_sip, m_dip, m_sport, m_dport + i, m_win,
      m_baseRtt,
      MakeCallback(&RdmaSimTraffic::Finish, this));
  }

}

void RdmaSimTraffic::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
}

// Chamado toda vez que termina de enviar
void RdmaSimTraffic::Finish() {
  NS_LOG_FUNCTION_NOARGS();
  m_rdma->AddQueuePair(m_size, m_pg, m_sip, m_dip, m_sport, m_dport, m_win,
    m_baseRtt,
    MakeCallback(&RdmaSimTraffic::Finish, this));
  
}

void RdmaSimTraffic::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

} // namespace ns3
