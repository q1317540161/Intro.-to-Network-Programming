from scapy.all import *
import base64
pkts = rdpcap('lab2_try.pcap')
target = pkts[0]
for pkt in pkts:
	if IP in pkt and(pkt[IP].ttl>target[IP].ttl):
		target = pkt

txt = target.load
decoded_txt = base64.b64decode(txt).decode('UTF-8')
print(decoded_txt, file = open('output.txt', 'w'))
