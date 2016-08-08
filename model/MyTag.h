// mytag for dfs

#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/object-vector.h"
#include "ns3/ipv4-header.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/traffic-control-layer.h"

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>
#include <vector>
using namespace ns3;
using namespace ns3;
class MyTag : public Tag
{
public:
  MyTag(int numberOfSwitches){
    std::vector<uint8_t> vectorForPacket(numberOfSwitches);
    for (int i = 0; i < numberOfSwitches*2+2; ++i){

      vectorForPacket.push_back(0x00);
    }
    SetSimpleValue(vectorForPacket);
  }
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  void SetSimpleValue (std::vector<uint8_t> value);
  std::vector<uint8_t> GetSimpleValue (void) const;
  void SetTagItems(int maxItems) ;
  uint32_t GetTagItems (void)const;
  void SetSimpleValueByIndex(int index, uint8_t value);
  uint8_t GetSimpleValueByIndex(int index) const;
  void SetPacketStart(void);
  uint8_t GetPacketStart(void) const;
  void SetParentValueByIndex(int index, uint8_t value);
  uint8_t GetParentValueByIndex(int index) const;
  void SetCurrValueByIndex( int index, uint8_t value);
  uint8_t GetCurrValueByIndex(int index) const;
  void SetInputPort(uint8_t portNumber);
  uint8_t GetInputPort(void) const;
private:
  std::vector<uint8_t> m_simpleValue;
  int m_items;
};