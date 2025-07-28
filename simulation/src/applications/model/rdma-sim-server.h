#ifndef RDMA_SIM_SERVER_H
#define RDMA_SIM_SERVER_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/rdma-driver.h"

namespace ns3 {

class RdmaSimServer : public Application {
public:
  static TypeId GetTypeId(void);
  RdmaSimServer();
  virtual ~RdmaSimServer();

  void SetNode(Ptr<Node> node);
  void Receive(); // chamada pela RDMA ao receber dados

protected:
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  virtual void DoDispose(void);

private:
  void Process();

  // Atributos de simulação
  Ptr<Node> m_node;
  Ptr<RdmaDriver> m_rdma;

  uint64_t process_time;     // tempo de consumo (ns)
  uint32_t buffer_size;      // limite do buffer
  uint32_t buffer;           // quantidade de itens no buffer

  bool processing;           // está processando ou não
};

} // namespace ns3

#endif // RDMA_SIM_SERVER_H
