--wireshark -X lua_script:distpi.lua
--port 60606 (SocketPassive(60606)); - musi byc staly
--wybierz interfejs i capture using this filter: udp and port 60606

distpi_port = 50001
distpi_proto = Proto("distpi", "Distributed PI")

function distpi_proto.dissector(buffer, pinfo, tree)
	local msgtypes = {  [0]= "MessageHeartbeat", 
						[1]= "MessageHeartbeatACK", 
						[2]= "MessageInterrupt", 
						[3]= "MessageHello", 
						[4]= "MessageACK", 
						[5]= "MessageClose", 
						[6]= "MessageResult", 
						[7]= "MessageWork", 
						[255]= "MessageInvalid"}
	pinfo.cols.protocol = "DISTPI"
	local subtree = tree:add(distpi_proto, buffer(), "Distributed PI Message")
	msgtype = buffer(0,1):uint()
	subtree:add(msgtype, "Message type: " .. msgtypes[msgtype])
	if msgtype > 2 then
		subtree:add(buffer(1,4), "Sequence number: " .. buffer(1,4):uint())
		if msgtype > 5 then
			subtree:add(buffer(5, 8), "Points: " .. buffer(5,8):uint64())
			subtree:add(buffer(13, 4), "Segment ID: " .. buffer(13, 4):uint())
			if msgtype > 6 then
				subtree:add(buffer(17, 4), "Side: " .. buffer(17, 4):uint())
			end
		end
	end
end

udp_table = DissectorTable.get("udp.port")
udp_table:add(distpi_port, distpi_proto)
