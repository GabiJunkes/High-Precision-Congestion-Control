#ifndef RDMA_SIM_TRAFFIC_H
#define RDMA_SIM_TRAFFIC_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/rdma-driver.h"
#include "ns3/event-id.h"

namespace ns3 {

class RdmaSimTraffic : public Application {
public:
  static TypeId GetTypeId(void);
  RdmaSimTraffic();
  virtual ~RdmaSimTraffic();

  void SetRemote(Ipv4Address ip, uint16_t port);
  void SetLocal(Ipv4Address ip, uint16_t port);
  void SetPG(uint16_t pg);
  void SetSize(uint64_t size);
  void SetNode(Ptr<Node> node);

protected:
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  virtual void DoDispose(void);

private:
  void Finish();

  // Atributos RDMA e IP/Portas
  Ptr<Node> m_node;
  Ptr<RdmaDriver> m_rdma;

  Ipv4Address m_sip;
  Ipv4Address m_dip;
  uint16_t m_sport;
  uint16_t m_dport;
  uint16_t m_pg;
  uint32_t m_win;
  uint64_t m_baseRtt;
  uint64_t m_size;         // tamanho de cada envio

};

} // namespace ns3

#endif // RDMA_SIM_TRAFFIC_H
