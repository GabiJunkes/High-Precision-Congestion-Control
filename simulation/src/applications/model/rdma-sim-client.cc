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
#include <iostream>
#include <iomanip>

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
                        UintegerValue(7812),
                        MakeUintegerAccessor(&RdmaSimClient::process_time),
                        MakeUintegerChecker<uint64_t>());
  return tid;
}

RdmaSimClient::RdmaSimClient()
    : buffer_in(0), buffer_out(0), is_sending(false), is_locked(false), locked_events(0), total_steps(0), start_time(0), count(0), is_paused(false) {
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

void RdmaSimClient::SetFile(std::ofstream &file) {
  m_file = &file;
}

void RdmaSimClient::StartApplication(void) {
  // printf("Started client\n");
  NS_LOG_FUNCTION_NOARGS();

  Simulator::Schedule(NanoSeconds(0),
                      MakeEvent(&RdmaSimClient::Process, this));
  Simulator::Schedule(NanoSeconds(0),
                      MakeEvent(&RdmaSimClient::Consume, this));
}

void RdmaSimClient::StopApplication() {
  NS_LOG_FUNCTION_NOARGS();
}

void RdmaSimClient::Process() {
  if (!is_paused) {
    if (is_locked == false) {
      // Se buffers são iguais conta locked_events (buffer_in != 0 é necessario para não ficar preso sem produzir nada)
      if (buffer_in == buffer_out) {
        locked_events += 1;
        Simulator::Schedule(NanoSeconds(process_time),
                        MakeEvent(&RdmaSimClient::Process, this));
        is_locked = true;
        return;
      }

      buffer_out += 1;

      if (total_steps == STEPS_PER_DATA_SIZE) {
        double starvation = (double)locked_events / (double)total_steps;
        (*m_file) << "P; "
                  << (double)process_time << "; "
                  << m_size << "; "
                  << locked_events << "; "
                  << starvation
                  << std::endl;


        locked_events = 0;
        m_size *= 10; // a cada loop de STEPS_PER_DATA_SIZE, multiplica m_size
        count++;
        total_steps = 0;
        buffer_in = 0;
        buffer_out = 0;

        // After completing a data size pause for some time so server can receive all packets at m_size
        is_paused = true;
        if (count < 5) {
          Simulator::Schedule(NanoSeconds(process_time * 100), MakeEvent(&RdmaSimClient::Unpause, this));
        }
      }

    }

    // Usa count para determinar quantos loops de STEPS_PER_DATA_SIZE ja foram
    if (count < 5) {
      Simulator::Schedule(NanoSeconds(process_time), MakeEvent(&RdmaSimClient::Process, this));
    }
  }
}

void RdmaSimClient::Unpause() {
  is_paused = false;
}

void RdmaSimClient::Consume() {
  if (count < 5) {        // Não rode se passar de 5 iterações
    if (!is_paused) {     // Não rode se estiver pausado mas chame schedule
      if (!is_sending) {  // Não rode se estiver renviando mas chame schedule
        is_sending = true;
        // guarda o momento que enviou
        start_time = Simulator::Now().GetMicroSeconds();

        // envia pacote, quando terminar chama RdmaSimClient::Finish()
        m_rdma->AddQueuePair(m_size, m_pg, m_sip, m_dip, m_sport, m_dport, m_win,
                            m_baseRtt,
                            MakeCallback(&RdmaSimClient::Finish, this));
      }
    }

    Simulator::Schedule(NanoSeconds(100),
                        MakeEvent(&RdmaSimClient::Consume, this));
  }
}

void RdmaSimClient::Finish() {
  uint64_t now = Simulator::Now().GetMicroSeconds();
  long fct = now - start_time;
  if (fct == 0) fct = 1;
  double throughput = (double)m_size / fct;

  (*m_file) << "C; "
            << (double)process_time << "; "
            << m_size << "; "
            << fct << "; "
            << throughput
            << std::endl;

  is_sending = false;
  buffer_in += 1;
  // reseta lock (talvez tenha q ser resetado em outro lugar)
  is_locked = false;
  total_steps++;
}

void RdmaSimClient::DoDispose(void) {
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

} // namespace ns3
