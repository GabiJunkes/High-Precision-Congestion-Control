#ifndef RDMA_SIM_SERVER_H
#define RDMA_SIM_SERVER_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/rdma-driver.h"
#include <fstream>
#include <vector>

namespace ns3 {

class RdmaSimServer : public Application {
public:
  static TypeId GetTypeId(void);
  RdmaSimServer();
  virtual ~RdmaSimServer();

  void SetRdma(Ptr<RdmaDriver> rdma);
  void SetNode(Ptr<Node> node);
  void Receive(Ptr<RdmaQueuePair> qp); // chamada pela RDMA ao receber dados
  void SetFile(std::ofstream &m_file);

protected:
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  virtual void DoDispose(void);

private:
  void Process();
  void Consume();

  // Atributos de simulação
  Ptr<Node> m_node;
  Ptr<RdmaDriver> m_rdma;
  uint64_t process_time;     // tempo de consumo (ns)
  uint32_t buffer_in;      // limite do buffer
  uint32_t buffer_out;           // quantidade de itens no buffer

  uint32_t locked_events;
  uint32_t total_steps;

  std::ofstream* m_file;

  uint32_t count;

  bool is_processing;           // está processando ou não
  bool is_locked;               // simmula pc->mutex
  
  std::vector<uint32_t> m_sizes_per_step;

  static const uint32_t STEPS_PER_DATA_SIZE = 100;
};

} // namespace ns3

#endif // RDMA_SIM_SERVER_H
