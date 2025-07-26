#include "rdma-sim-server.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RdmaSimServer");
NS_OBJECT_ENSURE_REGISTERED(RdmaSimServer);

TypeId RdmaSimServer::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::RdmaSimServer")
          .SetParent<Application>()
          .AddConstructor<RdmaSimServer>()
          .AddAttribute("ProcessTime", "Time to process data (ns)",
                        UintegerValue(100),
                        MakeUintegerAccessor(&RdmaSimServer::process_time),
                        MakeUintegerChecker<uint64_t>())
          .AddAttribute("BufferSize", "Maximum buffer size",
                        UintegerValue(100),
                        MakeUintegerAccessor(&RdmaSimServer::buffer_size),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute("DataSize", "Expected total data (bytes)",
                        UintegerValue(100000),
                        MakeUintegerAccessor(&RdmaSimServer::data_size),
                        MakeUintegerChecker<uint64_t>());
  return tid;
}

RdmaSimServer::RdmaSimServer()
    : buffer(0), processing(false) {
  NS_LOG_FUNCTION_NOARGS();
}

RdmaSimServer::~RdmaSimServer() {
  NS_LOG_FUNCTION_NOARGS();
}

void RdmaSimServer::SetNode(Ptr<Node> node) {
  m_node = node;
  m_rdma = m_node->GetObject<RdmaDriver>();
}

void RdmaSimServer::StartApplication(void) {
  NS_LOG_FUNCTION_NOARGS();
  // Nenhuma ação inicial, espera receber dados
}

void RdmaSimServer::StopApplication(void) {
  NS_LOG_FUNCTION_NOARGS();
}

void RdmaSimServer::Receive() {
  NS_LOG_FUNCTION_NOARGS();
  if (buffer < buffer_size) {
    buffer += 1;
    if (!processing) {
      processing = true;
      Simulator::Schedule(NanoSeconds(process_time),
                          MakeEvent(&RdmaSimServer::Process, this));
    }
  } else {
    NS_LOG_WARN("Buffer overflow, data dropped");
  }
}

void RdmaSimServer::Process() {
  NS_LOG_FUNCTION_NOARGS();
  if (buffer > 0) {
    buffer -= 1;
  }

  if (buffer > 0) {
    Simulator::Schedule(NanoSeconds(process_time),
                        MakeEvent(&RdmaSimServer::Process, this));
  } else {
    processing = false;
  }
}

void RdmaSimServer::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

} // namespace ns3
