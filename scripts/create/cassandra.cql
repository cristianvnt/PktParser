CREATE KEYSPACE IF NOT EXISTS wow_packets 
WITH REPLICATION = { 'class': 'NetworkTopologyStrategy', 'dc1': 3 };

CREATE TABLE IF NOT EXISTS wow_packets.file_metadata (
    file_id uuid PRIMARY KEY,
    source_file text,
    build int,
    start_time bigint,
    packet_count int
);

CREATE TABLE IF NOT EXISTS wow_packets.packets (
    build int,
    file_id uuid,
    bucket int,
    packet_number int,
    direction tinyint,
    packet_len int,
    opcode int,
    timestamp bigint,
    pkt_json blob,
    PRIMARY KEY ((build, file_id, bucket), packet_number)
) WITH CLUSTERING ORDER BY (packet_number ASC)
  AND COMPACTION = { 
    'class': 'UnifiedCompactionStrategy',
    'scaling_parameters': 'T4',
    'target_sstable_size': '1GiB',
    'base_shard_count': '4'
  };
