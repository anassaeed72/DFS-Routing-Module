/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DFS_ROUTING_MODULE_H
#define DFS_ROUTING_MODULE_H

#include <list>
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/random-variable-stream.h"

#include <vector>
#include <iomanip>
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/boolean.h"
#include "ns3/node.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/global-route-manager.h"
#include "MyTag.h"
namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Ipv4RoutingTableEntry;
class Ipv4MulticastRoutingTableEntry;
class Node;


/**
 * \brief Global routing protocol for IP version 4 stacks.
 *
 * In ns-3 we have the concept of a pluggable routing protocol.  Routing
 * protocols are added to a list maintained by the Ipv4L3Protocol.  Every 
 * stack gets one routing protocol for free -- the Ipv4StaticRouting routing
 * protocol is added in the constructor of the Ipv4L3Protocol (this is the 
 * piece of code that implements the functionality of the IP layer).
 *
 * As an option to running a dynamic routing protocol, a GlobalRouteManager
 * object has been created to allow users to build routes for all participating
 * nodes.  One can think of this object as a "routing oracle"; it has
 * an omniscient view of the topology, and can construct shortest path
 * routes between all pairs of nodes.  These routes must be stored 
 * somewhere in the node, so therefore this class DFSModuleRouting
 * is used as one of the pluggable routing protocols.  It is kept distinct
 * from Ipv4StaticRouting because these routes may be dynamically cleared
 * and rebuilt in the middle of the simulation, while manually entered
 * routes into the Ipv4StaticRouting may need to be kept distinct.
 *
 * This class deals with Ipv4 unicast routes only.
 *
 * \see Ipv4RoutingProtocol
 * \see GlobalRouteManager
 */
class DFSModuleRouting : public Ipv4RoutingProtocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Construct an empty DFSModuleRouting routing protocol,
   *
   * The DFSModuleRouting class supports host and network unicast routes.
   * This method initializes the lists containing these routes to empty.
   *
   * \see DFSModuleRouting
   */
  DFSModuleRouting ();
  virtual ~DFSModuleRouting ();

  // These methods inherited from base class
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  virtual bool RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  /**
   * \brief Add a host route to the global routing table.
   *
   * \param dest The Ipv4Address destination for this route.
   * \param nextHop The Ipv4Address of the next hop in the route.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see Ipv4Address
   */
  void AddHostRouteTo (Ipv4Address dest, 
                       Ipv4Address nextHop, 
                       uint32_t interface);
  /**
   * \brief Add a host route to the global routing table.
   *
   * \param dest The Ipv4Address destination for this route.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see Ipv4Address
   */
  void AddHostRouteTo (Ipv4Address dest, 
                       uint32_t interface);

  /**
   * \brief Add a network route to the global routing table.
   *
   * \param network The Ipv4Address network for this route.
   * \param networkMask The Ipv4Mask to extract the network.
   * \param nextHop The next hop in the route to the destination network.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see Ipv4Address
   */
  void AddNetworkRouteTo (Ipv4Address network, 
                          Ipv4Mask networkMask, 
                          Ipv4Address nextHop, 
                          uint32_t interface);

  /**
   * \brief Add a network route to the global routing table.
   *
   * \param network The Ipv4Address network for this route.
   * \param networkMask The Ipv4Mask to extract the network.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see Ipv4Address
   */
  void AddNetworkRouteTo (Ipv4Address network, 
                          Ipv4Mask networkMask, 
                          uint32_t interface);

  /**
   * \brief Add an external route to the global routing table.
   *
   * \param network The Ipv4Address network for this route.
   * \param networkMask The Ipv4Mask to extract the network.
   * \param nextHop The next hop Ipv4Address
   * \param interface The network interface index used to send packets to the
   * destination.
   */
  void AddASExternalRouteTo (Ipv4Address network,
                             Ipv4Mask networkMask,
                             Ipv4Address nextHop,
                             uint32_t interface);

  /**
   * \brief Get the number of individual unicast routes that have been added
   * to the routing table.
   *
   * \warning The default route counts as one of the routes.
   * \returns the number of routes
   */
  uint32_t GetNRoutes (void) const;

  /**
   * \brief Get a route from the global unicast routing table.
   *
   * Externally, the unicast global routing table appears simply as a table with
   * n entries.  The one subtlety of note is that if a default route has been set
   * it will appear as the zeroth entry in the table.  This means that if you
   * add only a default route, the table will have one entry that can be accessed
   * either by explicitly calling GetDefaultRoute () or by calling GetRoute (0).
   *
   * Similarly, if the default route has been set, calling RemoveRoute (0) will
   * remove the default route.
   *
   * \param i The index (into the routing table) of the route to retrieve.  If
   * the default route has been set, it will occupy index zero.
   * \return If route is set, a pointer to that Ipv4RoutingTableEntry is returned, otherwise
   * a zero pointer is returned.
   *
   * \see Ipv4RoutingTableEntry
   * \see DFSModuleRouting::RemoveRoute
   */
  Ipv4RoutingTableEntry *GetRoute (uint32_t i) const;

  /**
   * \brief Remove a route from the global unicast routing table.
   *
   * Externally, the unicast global routing table appears simply as a table with
   * n entries.  The one subtlety of note is that if a default route has been set
   * it will appear as the zeroth entry in the table.  This means that if the
   * default route has been set, calling RemoveRoute (0) will remove the
   * default route.
   *
   * \param i The index (into the routing table) of the route to remove.  If
   * the default route has been set, it will occupy index zero.
   *
   * \see Ipv4RoutingTableEntry
   * \see DFSModuleRouting::GetRoute
   * \see DFSModuleRouting::AddRoute
   */
  void RemoveRoute (uint32_t i);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  void DoDispose (void);
  int IndexOfNetDeviceInNode(Ptr<const NetDevice> netDevice) const;
  int NodeId(Ptr< const NetDevice> netDevice) const;
  Ptr<Ipv4Route> CreateRouteFromHeader(const Ipv4Header &header) const;
  bool RouteNotWorking(Ptr<Ipv4Route> route);
  bool DeviceOutOfRange(Ptr<const NetDevice> netDevice);
  int CurrVal(MyTag tag, int index);
  int ParVal(MyTag tag, int index);
  bool NormalRouting(MyTag tag);
  bool NetDeviceNotWorking(Ptr<const NetDevice> netDevice);
  bool OutputIsParent(Ptr<const NetDevice>netDevice, MyTag tag, Ptr<const NetDevice> idev);
  bool ActualSend(UnicastForwardCallback ucb,Ptr<const NetDevice>idev,Ptr<Ipv4Route> route,Ptr<const Packet> p, const Ipv4Header &header,MyTag &tag);
private:

  /// Set to true if packets are randomly routed among ECMP; set to false for using only one route consistently
  bool m_randomEcmpRouting;
  /// Set to true if this interface should respond to interface events by globallly recomputing routes 
  bool m_respondToInterfaceEvents;
  /// A uniform random number generator for randomly routing packets among ECMP 
  Ptr<UniformRandomVariable> m_rand;

  /// container of Ipv4RoutingTableEntry (routes to hosts)
  typedef std::list<Ipv4RoutingTableEntry *> HostRoutes;
  /// const iterator of container of Ipv4RoutingTableEntry (routes to hosts)
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator HostRoutesCI;
  /// iterator of container of Ipv4RoutingTableEntry (routes to hosts)
  typedef std::list<Ipv4RoutingTableEntry *>::iterator HostRoutesI;

  /// container of Ipv4RoutingTableEntry (routes to networks)
  typedef std::list<Ipv4RoutingTableEntry *> NetworkRoutes;
  /// const iterator of container of Ipv4RoutingTableEntry (routes to networks)
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator NetworkRoutesCI;
  /// iterator of container of Ipv4RoutingTableEntry (routes to networks)
  typedef std::list<Ipv4RoutingTableEntry *>::iterator NetworkRoutesI;

  /// container of Ipv4RoutingTableEntry (routes to external AS)
  typedef std::list<Ipv4RoutingTableEntry *> ASExternalRoutes;
  /// const iterator of container of Ipv4RoutingTableEntry (routes to external AS)
  typedef std::list<Ipv4RoutingTableEntry *>::const_iterator ASExternalRoutesCI;
  /// iterator of container of Ipv4RoutingTableEntry (routes to external AS)
  typedef std::list<Ipv4RoutingTableEntry *>::iterator ASExternalRoutesI;

  Ptr<Ipv4Route> LookupGlobal (Ipv4Address dest, Ptr<NetDevice> oif = 0);

  HostRoutes m_hostRoutes;             //!< Routes to hosts
  NetworkRoutes m_networkRoutes;       //!< Routes to networks
  ASExternalRoutes m_ASexternalRoutes; //!< External routes imported

  Ptr<Ipv4> m_ipv4; //!< associated IPv4 instance
  int delta = 1;
  int numberOfSwitches = 5;
};

} // Namespace ns3

#endif /* DFS_ROUTING_MODULE_H */

