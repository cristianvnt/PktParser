#!/bin/bash

curl -X PUT "http://localhost:9200/wow_packets" -H "Content-Type: application/json" -d '{
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0,
        "refresh_interval": "5s",
        "index.translog.durability": "async",
        "index.translog.sync_interval": "30s"
    },
    "mappings": {
        "_source": { "enabled": false },
        "properties": {
            "build":            { "type": "integer" },
            "file_id":          { "type": "keyword" },
            "packet_number":    { "type": "integer" },
            "source_file":      { "type": "keyword" },
            "direction":        { "type": "keyword" },
            "opcode":           { "type": "integer" },
            "packet_name":      { "type": "keyword" },
            "timestamp":        { "type": "date", "format": "epoch_millis" },
            
            "original_cast_id": { "type": "keyword" },
            "spell_id":         { "type": "integer" },
            "cast_id":          { "type": "keyword" },
            "caster_guid":      { "type": "keyword" },
            "caster_type":      { "type": "keyword" },
            "caster_entry":     { "type": "integer" },
            "caster_low":       { "type": "long" },
            "map_id":           { "type": "integer" }
        }
    }
}'

echo ""
echo "Index created."