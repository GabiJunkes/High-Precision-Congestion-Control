#ifndef RDMA_SIM_CLIENT_H
#define RDMA_SIM_CLIENT_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/rdma-driver.h"
#include "ns3/event-id.h"
#include <fstream>

namespace ns3 {

class RdmaSimClient : public Application {
public:
  static TypeId GetTypeId(void);
  RdmaSimClient();
  virtual ~RdmaSimClient();

  void SetRemote(Ipv4Address ip, uint16_t port);
  void SetLocal(Ipv4Address ip, uint16_t port);
  void SetPG(uint16_t pg);
  void SetSize(uint64_t size);
  void SetNode(Ptr<Node> node);
  void SetFile(std::ofstream &m_file);

protected:
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  virtual void DoDispose(void);

private:
  void Process();
  void Consume();
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
  uint64_t m_size;         // tamanho de cada envio (o quanto cada process gera de dados)

  // Controle de buffer e estado
  uint32_t buffer_in;         // atual ocupação do buffer
  uint32_t buffer_out;         // atual ocupação do buffer
  
  uint64_t process_time;   // tempo entre produções (ns)

  uint32_t locked_events;
  uint32_t total_steps;
  uint32_t start_time;
  std::ofstream* m_file;

  uint32_t count;

  bool is_sending;         // se está aguardando envio ser finalizado
  bool is_locked;          // simula pc_mutex

  static const uint32_t STEPS_PER_DATA_SIZE = 100;
};

} // namespace ns3

#endif // RDMA_SIM_CLIENT_H
