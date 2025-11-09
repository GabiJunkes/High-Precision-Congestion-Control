#include "rdma-sim-server.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

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
                        MakeUintegerChecker<uint64_t>());
  return tid;
}

RdmaSimServer::RdmaSimServer() : buffer_in(0), buffer_out(0), is_locked(false), is_processing(false), locked_events(0), total_steps(0) {
  NS_LOG_FUNCTION_NOARGS();
}

RdmaSimServer::~RdmaSimServer() { NS_LOG_FUNCTION_NOARGS(); }

void RdmaSimServer::SetNode(Ptr<Node> node) {
  m_node = node;
}

void RdmaSimServer::SetRdma(Ptr<RdmaDriver> rdma) {
  m_rdma = rdma;
}

void RdmaSimServer::SetFile(std::ofstream &file) {
  m_file = &file;
}

void RdmaSimServer::StartApplication(void) {
  NS_ASSERT(m_rdma != nullptr);

  NS_LOG_FUNCTION_NOARGS();
  Simulator::Schedule(NanoSeconds(0), MakeEvent(&RdmaSimServer::Process, this));
  m_rdma->TraceConnectWithoutContext("QpComplete", MakeCallback(&RdmaSimServer::Receive, this));
}

void RdmaSimServer::StopApplication(void) { NS_LOG_FUNCTION_NOARGS(); }

void RdmaSimServer::Receive(Ptr<RdmaQueuePair> qp) {
    uint64_t now = Simulator::Now().GetMicroSeconds();

    long fct = now - qp->startTime.GetMicroSeconds();

    if (fct == 0) fct = 1;

    double throughput = (double)qp->m_size / fct;

    m_size = qp->m_size;

    (*m_file) << "C; "
              << (double)process_time << "; "
              << qp->m_size << "; "
              << fct << "; "
              << throughput
              << std::endl;

    buffer_in += 1;
    is_locked = false;
}

void RdmaSimServer::Process() {
  if (is_locked == false) {
    if (buffer_in == buffer_out) {
      locked_events += 1;
      // Agenda proxima chamada dessa função, returnando a seguir para não executar lógica, assim simulando lock
      Simulator::Schedule(NanoSeconds(process_time),
                      MakeEvent(&RdmaSimServer::Process, this));
      is_locked = true;
      return;
    }

    buffer_out += 1;

    total_steps++;

    if (total_steps == STEPS_PER_DATA_SIZE) {
      double starvation = (double)locked_events / (double)STEPS_PER_DATA_SIZE;
      (*m_file) << "P; "
                << (double)process_time << "; "
                << m_size << "; "
                << locked_events << "; "
                << starvation
                << std::endl;
    }

  }

  Simulator::Schedule(NanoSeconds(process_time),
                      MakeEvent(&RdmaSimServer::Process, this));
}

void RdmaSimServer::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

} // namespace ns3
