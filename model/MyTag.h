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
#include "ns3/log.h"

#include <iostream>
#include <vector>
using namespace ns3;
using namespace ns3;
class MyTag : public Tag
{
public:
  MyTag(){}
  MyTag(int numberOfSwitches){
    std::vector<uint8_t> vectorForPacket(numberOfSwitches);
    for (int i = 0; i < numberOfSwitches*2+2; ++i){

      vectorForPacket.push_back(0x00);
    }
    SetSimpleValue(vectorForPacket);
  }
  static TypeId GetTypeId (void){
  static TypeId tid = TypeId ("ns3::MyTag")
    .SetParent<Tag> ()
    .AddConstructor<MyTag> ()
  ;
  return tid;
}
  virtual TypeId GetInstanceTypeId (void) const{
  return GetTypeId ();
}
  virtual uint32_t GetSerializedSize (void) const{
  return GetTagItems();
}
  virtual void Serialize (TagBuffer i) const{
  
  std::cout <<"Serialize called\n";
  i.WriteU8((uint8_t)m_simpleValue.size());
  i.WriteVector (m_simpleValue);
  std::cout << "Serialize done\n";
  printTag();
}
  virtual void Deserialize (TagBuffer i){
  std::cout <<"Deserialize called\n";
  int itemsToRead = (int) i.ReadU8();
  SetTagItems(itemsToRead);
  m_simpleValue = i.ReadVector (itemsToRead);
  printTag();
}
  virtual void Print (std::ostream &os) const{
  os << "v=";
  for (auto x : m_simpleValue){

    os << x << " ";
  }
}

  // these are our accessors to our tag structure
  void SetSimpleValue (std::vector<uint8_t> value){
  m_simpleValue = value;
  SetTagItems(m_simpleValue.size());
}
  std::vector<uint8_t> GetSimpleValue (void) const{
  return m_simpleValue;
}
  void SetTagItems(int maxItems) {
  m_items = maxItems;
}
  uint32_t GetTagItems (void)const{
  return (uint32_t)(m_items+2);
}
  void SetSimpleValueByIndex(int index, uint8_t value){
    std::cout << "SetSimpleValueByIndex called\n";
    m_simpleValue[index] = value;
    printTag();
}
  uint8_t GetSimpleValueByIndex(int index) const{
  return m_simpleValue[index];
}
  void SetPacketStart(void) {
    std::cout <<"SetPacketStart called\n";
  SetSimpleValueByIndex(0,0x01);
}
  uint8_t GetPacketStart(void) const{
  return GetSimpleValueByIndex(0);
}
  void SetParentValueByIndex(int index, uint8_t value){
  std::cout << "SetParentValueByIndex i="  << index << " v=" << value <<"\n";
  SetSimpleValueByIndex(((index)*2)+1,value);
}
  uint8_t GetParentValueByIndex(int index) const{
  return GetSimpleValueByIndex(((index)*2)+1);
}
  void SetCurrValueByIndex( int index, uint8_t value){
    std::cout << "SetCurrValueByIndex i="  << index << " v=" << value <<"\n"; 
  SetSimpleValueByIndex(((index)*2)+2,value);  
}
  uint8_t GetCurrValueByIndex(int index) const{
  return GetSimpleValueByIndex(((index)*2)+2);
}
  void SetInputPort(uint8_t portNumber);
  uint8_t GetInputPort(void) const;
  void printTag(MyTag tag){
  std::cout << "Printing tag size = "<< tag.GetSerializedSize()  << "\nv= ";
  for( uint8_t x: tag.GetSimpleValue()){
    std::cout << x << " ";
  }
  std::cout <<"  ||\n-----------------------\n";
}
void printTag() const{
  std::cout << "Printing tag size = "<< GetSimpleValue().size()  << "\nv= ";
  for( uint8_t x: GetSimpleValue()){
     if (x ==0x00)
    {
      /* code */
      std::cout <<"-"; 
      continue;
    }
    std::cout << x << " ";
  }
  std::cout <<"  ||\n-----------------------\n";
}

private:
  std::vector<uint8_t> m_simpleValue;
  int m_items;
};


