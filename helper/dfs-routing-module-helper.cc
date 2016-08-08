/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "dfs-routing-module-helper.h"
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "dfs-routing-module-helper.h"
#include "ns3/global-router-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DFSModuleRoutingHelper");

DFSModuleRoutingHelper::DFSModuleRoutingHelper ()
{
}

DFSModuleRoutingHelper::DFSModuleRoutingHelper (const DFSModuleRoutingHelper &o)
{
}

DFSModuleRoutingHelper*
DFSModuleRoutingHelper::Copy (void) const
{
  return new DFSModuleRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
DFSModuleRoutingHelper::Create (Ptr<Node> node) const
{
  NS_LOG_LOGIC ("Adding GlobalRouter interface to node " <<
                node->GetId ());

  Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter> ();
  node->AggregateObject (globalRouter);

  NS_LOG_LOGIC ("Adding GlobalRouting Protocol to node " << node->GetId ());
  Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting> ();
  globalRouter->SetRoutingProtocol (globalRouting);

  return globalRouting;
}

void 
DFSModuleRoutingHelper::PopulateRoutingTables (void)
{
  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();
}
void 
DFSModuleRoutingHelper::RecomputeRoutingTables (void)
{
  GlobalRouteManager::DeleteGlobalRoutes ();
  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();
}


} // namespace ns3
