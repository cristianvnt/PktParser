CREATE KEYSPACE IF NOT EXISTS wow_packets 
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS wow_packets.packets (
    packet_number int PRIMARY KEY,
    direction text,
    packet_name text,
    packet_len int,
    opcode text,
    timestamp text,
    build int,
    pkt_json text
);