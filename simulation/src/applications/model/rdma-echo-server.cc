#include "rdma-echo-server.h"
#include "ns3/log.h"
#include "ns3/rdma-client-helper.h"
#include "ns3/rdma-client.h"
#include "ns3/ipv4.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RdmaEchoServer");
NS_OBJECT_ENSURE_REGISTERED(RdmaEchoServer);

TypeId RdmaEchoServer::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::RdmaEchoServer")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<RdmaEchoServer>();
  return tid;
}

RdmaEchoServer::RdmaEchoServer()
  : m_rdma(0), m_node(0) {}

void RdmaEchoServer::SetRdma(Ptr<RdmaDriver> rdma) {
  m_rdma = rdma;
}

void RdmaEchoServer::SetNode(Ptr<Node> node) {
  m_node = node;
}

void RdmaEchoServer::StartApplication(void) {
  NS_ASSERT(m_rdma != nullptr);
  NS_LOG_INFO("EchoServer started on node " << m_node->GetId());

  m_rdma->TraceConnectWithoutContext("QpComplete", MakeCallback(&RdmaEchoServer::HandleQpComplete, this));
}

void RdmaEchoServer::StopApplication(void) {
  NS_LOG_INFO("EchoServer stopped on node " << m_node->GetId());
}

void RdmaEchoServer::HandleQpComplete(Ptr<RdmaQueuePair> qp) {
  Ipv4Address src = qp->dip;
  Ipv4Address dst = qp->sip;
  uint16_t sport = qp->dport;
  uint16_t dport = qp->sport;
  uint16_t pg = qp->m_pg;

  // TODO: adicionar valores no constructor
  uint64_t maxPackets = 1000;
  uint64_t rtt = 100; // em us
  uint32_t win = 64 * 1024; // bytes

  NS_LOG_INFO("EchoServer respondendo de " << dst << " para " << src);

  // TODO: melhorar isso
  RdmaClientHelper echoHelper(pg, dst, src, sport, dport, maxPackets, win, rtt);
  ApplicationContainer appCon = echoHelper.Install(m_node);

  // Schedule consumir buffer (taxaDeConsumo, Tamanho)
  appCon.Start(Seconds(0));
}

} // namespace ns3
