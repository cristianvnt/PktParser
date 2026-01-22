CREATE KEYSPACE IF NOT EXISTS wow_packets 
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS wow_packets.packets (
    build int,
    file_id uuid,
    packet_number int,
    source_file text,
    direction text,
    packet_name text,
    packet_len int,
    opcode text,
    timestamp text,
    pkt_json blob,
    PRIMARY KEY ((build), file_id, packet_number)
) WITH CLUSTERING ORDER BY (file_id ASC, packet_number ASC);