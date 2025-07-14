#ifndef RDMA_ECHO_SERVER_H
#define RDMA_ECHO_SERVER_H

#include "ns3/application.h"
#include "ns3/rdma-driver.h"
#include "ns3/rdma-hw.h"
#include "ns3/rdma-queue-pair.h"

namespace ns3 {

class RdmaEchoServer : public Application {
public:
  static TypeId GetTypeId(void);
  RdmaEchoServer();

  void SetRdma(Ptr<RdmaDriver> rdma);
  void SetNode(Ptr<Node> node);

protected:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:
  void HandleQpComplete(Ptr<RdmaQueuePair> qp);

  Ptr<RdmaDriver> m_rdma;
  Ptr<Node> m_node;
};

} // namespace ns3

#endif // RDMA_ECHO_SERVER_H
