CREATE KEYSPACE IF NOT EXISTS wow_packets 
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS wow_packets.packets (
    build int,
    file_id timeuuid,
    bucket int,
    packet_number int,
    source_file text,
    direction text,
    packet_name text,
    packet_len int,
    opcode text,
    timestamp bigint,
    pkt_json blob,
    PRIMARY KEY ((build, file_id, bucket), packet_number)
) WITH CLUSTERING ORDER BY (packet_number ASC);