#!/bin/bash

curl -X PUT "http://localhost:9200/wow_packets" -H "Content-Type: application/json" -d '{
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0,
        "refresh_interval": "5s"
    },
    "mappings": {
        "properties": {
            "build":            { "type": "integer" },
            "file_id":          { "type": "keyword" },
            "packet_number":    { "type": "integer" },
            "source_file":      { "type": "keyword" },
            "direction":        { "type": "keyword" },
            "opcode":           { "type": "integer" },
            "packet_name":      { "type": "keyword" },
            "timestamp":        { "type": "date", "format": "epoch_millis" },

            "spell_id":         { "type": "integer" },
            "cast_id":          { "type": "keyword" },
            "original_cast_id": { "type": "keyword" },
            "caster_guid":      { "type": "keyword" },
            "target_guids":     { "type": "keyword" },
            "hit_count":        { "type": "integer" },
            "miss_count":       { "type": "integer" }
        }
    }
}'

echo ""
echo "Index created."