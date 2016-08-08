/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dfs-routing-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DFSModuleRouting");

NS_OBJECT_ENSURE_REGISTERED (DFSModuleRouting);

TypeId 
DFSModuleRouting::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::DFSModuleRouting")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

DFSModuleRouting::DFSModuleRouting () 
  : m_randomEcmpRouting (false),
    m_respondToInterfaceEvents (false)
{
  NS_LOG_FUNCTION (this);

  m_rand = CreateObject<UniformRandomVariable> ();
}

DFSModuleRouting::~DFSModuleRouting ()
{
  NS_LOG_FUNCTION (this);
}

void 
DFSModuleRouting::AddHostRouteTo (Ipv4Address dest, 
                                   Ipv4Address nextHop, 
                                   uint32_t interface)
{
  NS_LOG_FUNCTION (this << dest << nextHop << interface);
  Ipv4RoutingTableEntry *route = new Ipv4RoutingTableEntry ();
  *route = Ipv4RoutingTableEntry::CreateHostRouteTo (dest, nextHop, interface);
  m_hostRoutes.push_back (route);
}

void 
DFSModuleRouting::AddHostRouteTo (Ipv4Address dest, 
                                   uint32_t interface)
{
  NS_LOG_FUNCTION (this << dest << interface);
  Ipv4RoutingTableEntry *route = new Ipv4RoutingTableEntry ();
  *route = Ipv4RoutingTableEntry::CreateHostRouteTo (dest, interface);
  m_hostRoutes.push_back (route);
}

void 
DFSModuleRouting::AddNetworkRouteTo (Ipv4Address network, 
                                      Ipv4Mask networkMask, 
                                      Ipv4Address nextHop, 
                                      uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << nextHop << interface);
  Ipv4RoutingTableEntry *route = new Ipv4RoutingTableEntry ();
  *route = Ipv4RoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        nextHop,
                                                        interface);
  m_networkRoutes.push_back (route);
}

void 
DFSModuleRouting::AddNetworkRouteTo (Ipv4Address network, 
                                      Ipv4Mask networkMask, 
                                      uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << interface);
  Ipv4RoutingTableEntry *route = new Ipv4RoutingTableEntry ();
  *route = Ipv4RoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        interface);
  m_networkRoutes.push_back (route);
}

void 
DFSModuleRouting::AddASExternalRouteTo (Ipv4Address network, 
                                         Ipv4Mask networkMask,
                                         Ipv4Address nextHop,
                                         uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << nextHop << interface);
  Ipv4RoutingTableEntry *route = new Ipv4RoutingTableEntry ();
  *route = Ipv4RoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        nextHop,
                                                        interface);
  m_ASexternalRoutes.push_back (route);
}


Ptr<Ipv4Route>
DFSModuleRouting::LookupGlobal (Ipv4Address dest, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << dest << oif);
  NS_LOG_LOGIC ("Looking for route for destination " << dest);
  Ptr<Ipv4Route> rtentry = 0;
  // store all available routes that bring packets to their destination
  typedef std::vector<Ipv4RoutingTableEntry*> RouteVec_t;
  RouteVec_t allRoutes;

  NS_LOG_LOGIC ("Number of m_hostRoutes = " << m_hostRoutes.size ());
  for (HostRoutesCI i = m_hostRoutes.begin (); 
       i != m_hostRoutes.end (); 
       i++) 
    {
      NS_ASSERT ((*i)->IsHost ());
      if ((*i)->GetDest ().IsEqual (dest)) 
        {
          if (oif != 0)
            {
              if (oif != m_ipv4->GetNetDevice ((*i)->GetInterface ()))
                {
                  NS_LOG_LOGIC ("Not on requested interface, skipping");
                  continue;
                }
            }
          allRoutes.push_back (*i);
          NS_LOG_LOGIC (allRoutes.size () << "Found global host route" << *i); 
        }
    }
  if (allRoutes.size () == 0) // if no host route is found
    {
      NS_LOG_LOGIC ("Number of m_networkRoutes" << m_networkRoutes.size ());
      for (NetworkRoutesI j = m_networkRoutes.begin (); 
           j != m_networkRoutes.end (); 
           j++) 
        {
          Ipv4Mask mask = (*j)->GetDestNetworkMask ();
          Ipv4Address entry = (*j)->GetDestNetwork ();
          if (mask.IsMatch (dest, entry)) 
            {
              if (oif != 0)
                {
                  if (oif != m_ipv4->GetNetDevice ((*j)->GetInterface ()))
                    {
                      NS_LOG_LOGIC ("Not on requested interface, skipping");
                      continue;
                    }
                }
              allRoutes.push_back (*j);
              NS_LOG_LOGIC (allRoutes.size () << "Found global network route" << *j);
            }
        }
    }
  if (allRoutes.size () == 0)  // consider external if no host/network found
    {
      for (ASExternalRoutesI k = m_ASexternalRoutes.begin ();
           k != m_ASexternalRoutes.end ();
           k++)
        {
          Ipv4Mask mask = (*k)->GetDestNetworkMask ();
          Ipv4Address entry = (*k)->GetDestNetwork ();
          if (mask.IsMatch (dest, entry))
            {
              NS_LOG_LOGIC ("Found external route" << *k);
              if (oif != 0)
                {
                  if (oif != m_ipv4->GetNetDevice ((*k)->GetInterface ()))
                    {
                      NS_LOG_LOGIC ("Not on requested interface, skipping");
                      continue;
                    }
                }
              allRoutes.push_back (*k);
              break;
            }
        }
    }
  if (allRoutes.size () > 0 ) // if route(s) is found
    {
      // pick up one of the routes uniformly at random if random
      // ECMP routing is enabled, or always select the first route
      // consistently if random ECMP routing is disabled
      uint32_t selectIndex;
      if (m_randomEcmpRouting)
        {
          selectIndex = m_rand->GetInteger (0, allRoutes.size ()-1);
        }
      else 
        {
          selectIndex = 0;
        }
      Ipv4RoutingTableEntry* route = allRoutes.at (selectIndex); 
      // create a Ipv4Route object from the selected routing table entry
      rtentry = Create<Ipv4Route> ();
      rtentry->SetDestination (route->GetDest ());
      /// \todo handle multi-address case
      rtentry->SetSource (m_ipv4->GetAddress (route->GetInterface (), 0).GetLocal ());
      rtentry->SetGateway (route->GetGateway ());
      uint32_t interfaceIdx = route->GetInterface ();
      rtentry->SetOutputDevice (m_ipv4->GetNetDevice (interfaceIdx));
      return rtentry;
    }
  else 
    {
      return 0;
    }
}

uint32_t 
DFSModuleRouting::GetNRoutes (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t n = 0;
  n += m_hostRoutes.size ();
  n += m_networkRoutes.size ();
  n += m_ASexternalRoutes.size ();
  return n;
}

Ipv4RoutingTableEntry *
DFSModuleRouting::GetRoute (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_hostRoutes.size ())
    {
      uint32_t tmp = 0;
      for (HostRoutesCI i = m_hostRoutes.begin (); 
           i != m_hostRoutes.end (); 
           i++) 
        {
          if (tmp  == index)
            {
              return *i;
            }
          tmp++;
        }
    }
  index -= m_hostRoutes.size ();
  uint32_t tmp = 0;
  if (index < m_networkRoutes.size ())
    {
      for (NetworkRoutesCI j = m_networkRoutes.begin (); 
           j != m_networkRoutes.end ();
           j++)
        {
          if (tmp == index)
            {
              return *j;
            }
          tmp++;
        }
    }
  index -= m_networkRoutes.size ();
  tmp = 0;
  for (ASExternalRoutesCI k = m_ASexternalRoutes.begin (); 
       k != m_ASexternalRoutes.end (); 
       k++) 
    {
      if (tmp == index)
        {
          return *k;
        }
      tmp++;
    }
  NS_ASSERT (false);
  // quiet compiler.
  return 0;
}
void 
DFSModuleRouting::RemoveRoute (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_hostRoutes.size ())
    {
      uint32_t tmp = 0;
      for (HostRoutesI i = m_hostRoutes.begin (); 
           i != m_hostRoutes.end (); 
           i++) 
        {
          if (tmp  == index)
            {
              NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_hostRoutes.size ());
              delete *i;
              m_hostRoutes.erase (i);
              NS_LOG_LOGIC ("Done removing host route " << index << "; host route remaining size = " << m_hostRoutes.size ());
              return;
            }
          tmp++;
        }
    }
  index -= m_hostRoutes.size ();
  uint32_t tmp = 0;
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j++) 
    {
      if (tmp == index)
        {
          NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_networkRoutes.size ());
          delete *j;
          m_networkRoutes.erase (j);
          NS_LOG_LOGIC ("Done removing network route " << index << "; network route remaining size = " << m_networkRoutes.size ());
          return;
        }
      tmp++;
    }
  index -= m_networkRoutes.size ();
  tmp = 0;
  for (ASExternalRoutesI k = m_ASexternalRoutes.begin (); 
       k != m_ASexternalRoutes.end ();
       k++)
    {
      if (tmp == index)
        {
          NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_ASexternalRoutes.size ());
          delete *k;
          m_ASexternalRoutes.erase (k);
          NS_LOG_LOGIC ("Done removing network route " << index << "; network route remaining size = " << m_networkRoutes.size ());
          return;
        }
      tmp++;
    }
  NS_ASSERT (false);
}

int64_t
DFSModuleRouting::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rand->SetStream (stream);
  return 1;
}

void
DFSModuleRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (HostRoutesI i = m_hostRoutes.begin (); 
       i != m_hostRoutes.end (); 
       i = m_hostRoutes.erase (i)) 
    {
      delete (*i);
    }
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j = m_networkRoutes.erase (j)) 
    {
      delete (*j);
    }
  for (ASExternalRoutesI l = m_ASexternalRoutes.begin (); 
       l != m_ASexternalRoutes.end ();
       l = m_ASexternalRoutes.erase (l))
    {
      delete (*l);
    }

  Ipv4RoutingProtocol::DoDispose ();
}

// Formatted like output of "route -n" command
void
DFSModuleRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  *os << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
      << ", Time: " << Now().As (Time::S)
      << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (Time::S)
      << ", DFSModuleRouting table" << std::endl;

  if (GetNRoutes () > 0)
    {
      *os << "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface" << std::endl;
      for (uint32_t j = 0; j < GetNRoutes (); j++)
        {
          std::ostringstream dest, gw, mask, flags;
          Ipv4RoutingTableEntry route = GetRoute (j);
          dest << route.GetDest ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << dest.str ();
          gw << route.GetGateway ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << gw.str ();
          mask << route.GetDestNetworkMask ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << mask.str ();
          flags << "U";
          if (route.IsHost ())
            {
              flags << "H";
            }
          else if (route.IsGateway ())
            {
              flags << "G";
            }
          *os << std::setiosflags (std::ios::left) << std::setw (6) << flags.str ();
          // Metric not implemented
          *os << "-" << "      ";
          // Ref ct not implemented
          *os << "-" << "      ";
          // Use not implemented
          *os << "-" << "   ";
          if (Names::FindName (m_ipv4->GetNetDevice (route.GetInterface ())) != "")
            {
              *os << Names::FindName (m_ipv4->GetNetDevice (route.GetInterface ()));
            }
          else
            {
              *os << route.GetInterface ();
            }
          *os << std::endl;
        }
    }
  *os << std::endl;
}

Ptr<Ipv4Route>
DFSModuleRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p << &header << oif << &sockerr);
  // adding tag to packet
  
  MyTag tag(numberOfSwitches);
  p->AddPacketTag(tag);
	//
	// First, see if this is a multicast packet we have a route for.  If we
	// have a route, then send the packet down each of the specified interfaces.
    //
  if (header.GetDestination ().IsMulticast ())
    {
      NS_LOG_LOGIC ("Multicast destination-- returning false");
      return 0; // Let other routing protocols try to handle this
    }
	//
	// See if this is a unicast packet we have a route for.
	//
  NS_LOG_LOGIC ("Unicast destination- looking up");
  Ptr<Ipv4Route> rtentry = LookupGlobal (header.GetDestination (), oif);
  if (rtentry)
    {
      sockerr = Socket::ERROR_NOTERROR;
    }
  else
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
    }
  return rtentry;
}

bool 
DFSModuleRouting::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                LocalDeliverCallback lcb, ErrorCallback ecb)
{ 

  NS_LOG_FUNCTION (this << p << header << header.GetSource () << header.GetDestination () << idev << &lcb << &ecb);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  MyTag tag(numberOfSwitches);
  if (m_ipv4->IsDestinationAddress (header.GetDestination (), iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Local delivery to " << header.GetDestination ());

          lcb (p, header, iif);
          return true;
        }
      else
        {
          return false;
        }
    }
  
  p->PeekPacketTag(tag);
	if (NormalRouting(tag))
  {

    Ptr<Ipv4Route>route = LookupGlobal (header.GetDestination ());
   
    if (RouteNotWorking(route)) {
      Ptr<NetDevice> outputDevice = InitializeAlgoForPacket(tag,idev);
      route =CreateRouteFromHeader(header);
      route->SetOutputDevice(outputDevice);
    }
    return ActualSend(ucb,idev,route,p,header,tag);
  } else {
    if (CurrVal(tag,NodeId(idev)) == 0x00){
      tag.SetParentValueByIndex(NodeId(idev),IndexOfNetDeviceInNode(idev));
    }
    Ptr<NetDevice> outputDevice = idev->GetNode()->GetDevice(CurrVal(tag,NodeId(idev))+1);
    if (DeviceOutOfRange(outputDevice)){
      outputDevice = idev->GetNode()->GetDevice(ParVal(tag,NodeId(idev)));
      Ptr<Ipv4Route> route =CreateRouteFromHeader(header);
      route->SetOutputDevice(outputDevice);
      return ActualSend(ucb,idev,route,p,header,tag);
    }
    while ( NetDeviceNotWorking(outputDevice) || OutputIsParent(outputDevice,tag,idev)){
      outputDevice = idev->GetNode()->GetDevice(IndexOfNetDeviceInNode(outputDevice) +1);
      if (DeviceOutOfRange(outputDevice)){
        outputDevice = idev->GetNode()->GetDevice(ParVal(tag,NodeId(idev)));
        Ptr<Ipv4Route> route =CreateRouteFromHeader(header);
        route->SetOutputDevice(outputDevice);
        return ActualSend(ucb,idev,route,p,header,tag);
      }
    }
  }
}
Ptr<NetDevice>
DFSModuleRouting::InitializeAlgoForPacket(MyTag &tag,Ptr<const NetDevice> idev){
  tag.SetPacketStart();
  tag.SetParentValueByIndex(NodeId(idev),0);
  return idev->GetNode()->GetDevice(0);
}
bool
DFSModuleRouting::ActualSend(UnicastForwardCallback ucb,Ptr<const NetDevice>idev,Ptr<Ipv4Route> route,Ptr<const Packet> p, const Ipv4Header &header, MyTag &tag){
  tag.SetCurrValueByIndex(NodeId(idev),IndexOfNetDeviceInNode(route->GetOutputDevice()));
  ucb(route,p,header);
  return true;
}
bool
DFSModuleRouting::OutputIsParent(Ptr<const NetDevice>netDevice, MyTag tag, Ptr<const NetDevice> idev){
  if (IndexOfNetDeviceInNode(netDevice) == ParVal(tag,NodeId(idev)))
  {
    return true;
  }
  return false;
}
bool
DFSModuleRouting::NetDeviceNotWorking(Ptr<const NetDevice> netDevice){
  if (netDevice->IsLinkUp()== false)
  {
    return true;
  }
  return false;
}
bool
DFSModuleRouting::NormalRouting(MyTag tag){
  if (tag.GetPacketStart() ==0x00)
  {
      return true;
  }
  return false;
}
int
DFSModuleRouting::CurrVal(MyTag tag, int index){
  return tag.GetCurrValueByIndex(index);
}
int
DFSModuleRouting::ParVal(MyTag tag, int index){
  return tag.GetParentValueByIndex(index);
}
bool
DFSModuleRouting::RouteNotWorking(Ptr<Ipv4Route> route){
  if (route->GetOutputDevice()->IsLinkUp() == false)
  {
    return true;
  }
  return false;
}
bool 
DFSModuleRouting::DeviceOutOfRange(Ptr<const NetDevice> netDevice){
  if (IndexOfNetDeviceInNode(netDevice) >= delta +1){
    return true;
  }
  return false;
}
Ptr<Ipv4Route> 
DFSModuleRouting::CreateRouteFromHeader(const Ipv4Header &header) const{
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetSource(header.GetSource());
  route->SetDestination(header.GetDestination());
  return route;
}
int 
DFSModuleRouting::IndexOfNetDeviceInNode(Ptr<const NetDevice> netDevice) const{
	int numberOfDevicesinNode  = netDevice->GetNode()->GetNDevices();
	for (int i = 0; i < numberOfDevicesinNode; ++i){
		if (netDevice->GetIfIndex() == netDevice->GetNode()->GetDevice(i)->GetIfIndex())
		{
			/* code */
			return i;
		}
	}
  return -1;
}
int DFSModuleRouting::NodeId(Ptr< const NetDevice> netDevice) const {
	return netDevice->GetNode()->GetId();
}
void 
DFSModuleRouting::NotifyInterfaceUp (uint32_t i)
{
 
  NS_LOG_FUNCTION (this << i);
}

void 
DFSModuleRouting::NotifyInterfaceDown (uint32_t i)
{
  
  NS_LOG_FUNCTION (this << i);
}

void 
DFSModuleRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
 
  NS_LOG_FUNCTION (this << interface << address); 
}

void 
DFSModuleRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{


 NS_LOG_FUNCTION (this << interface << address);  
}

void 
DFSModuleRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;
}


} // namespace ns3
