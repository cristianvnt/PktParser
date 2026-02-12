#!/bin/bash

time for f in ~/Downloads/stuff/*.pkt; do
	echo "->>> Processing: $f <<<-"
	./build/PktParser "$f"
	echo ""
done
