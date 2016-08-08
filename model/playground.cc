if (IndexOfNetDeviceInNode(outputDevice) == -1){
	outputDevice = idev->GetNode()->GetDevice(0);
	while ( outputDevice->IsLinkUp() == false){
		outputDevice = idev->GetNode()->GetDevice (IndexOfNetDeviceInNode(outputDevice)+1);
		if ( IndexOfNetDeviceInNode(outputDevice) == delta +1){
			// drop packet
		}
	}
	tagForPacket.SetCurrValueByIndex(idev->GetNode()->GetId(),IndexOfNetDeviceInNode(outputDevice));
	// send on outputDevice
	lcb(p,header,m_ipv4->GetInterfaceForDevice(outputDevice));
}

while ( outputDevice->IsLinkUp() == false 
	||IndexOfNetDeviceInNode(outputDevice)== tagForPacket.GetParentValueByIndex(idev->GetNode()->GetId()) ){
	outputDevice = idev->GetNode()->GetDevice (IndexOfNetDeviceInNode(outputDevice)+1);
	if ( IndexOfNetDeviceInNode(outputDevice) == delta +1){
		int outputDeviceIndex = tagForPacket.GetParentValueByIndex(idev->GetNode()->GetId());
		outputDevice = idev->GetNode()->GetDevice(outputDeviceIndex);
		tagForPacket.SetCurrValueByIndex(idev->GetNode()->GetId(),0x00);
		// call third function and 
		return true;
	}
}
tagForPacket.SetCurrValueByIndex(idev->GetNode()->GetId(),IndexOfNetDeviceInNode(outputDevice));
// now call thied function


if ( tagForPacket.GetCurrValueByIndex(idev->GetNode()->GetId()) == 0x00 
	&& tagForPacket.GetParentValueByIndex(idev->GetNode()->GetId()) ==0x00){
	tagForPacket.SetParentValueByIndex(idev->GetNode()->GetId(),IndexOfNetDeviceInNode(idev));
	// send on idev
	return true;
}
if (tagForPacket.GetCurrValueByIndex(idev->GetNode()->GetId()) != IndexOfNetDeviceInNode(idev) &&
 tagForPacket.GetParentValueByIndex(idev->GetNode()->GetId()) != IndexOfNetDeviceInNode(idev)){
 	// send on idev
 	return true;	
}

outputDevice = idev->GetNode()->GetDevice(tagForPacket.GetCurrValueByIndex(idev->GetNode()->GetId())+1);
if (IndexOfNetDeviceInNode(outputDevice) == delta +1){
	outputDevice = idev->GetNode()->GetDevice(tagForPacket.GetParentValueByIndex(idev->GetNode()->GetId()));
	// go to third function
	return true;
}
   
if (tagForPacket.GetPacketStart() ==0x00){
	// get default route
	if (outputDevice->IsLinkUp() == false){
		tagForPacket.SetPacketStart();
		tagForPacket.SetParentValueByIndex(idev->GetNode()->GetId(),0x00);
		outputDevice = idev->GetNode()->GetDevice(0);
	}
}