#!/usr/bin/env python3

import re
import psycopg2
import sys
import os
from pathlib import Path
from dotenv import load_dotenv

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent

load_dotenv(PROJECT_ROOT / '.env')

DB_CONFIG = \
{
    'host': os.getenv('POSTGRES_HOST', 'localhost'),
    'port': os.getenv('POSTGRES_PORT', '5432'),
    'dbname': os.getenv('POSTGRES_DB', 'wow_metadata'),
    'user': os.getenv('POSTGRES_USER', 'wowparser'),
    'password': os.getenv('POSTGRES_PASSWORD')
}

def extract_build_mappings(content: str) -> dict[int, dict[str, str]]:
    mappings = {}
    lines = content.split('\n')
    current_cases = []
    
    for line in lines:
        # match: case ClientVersionBuild.V11_2_5_63506:
        case_match = re.search(r'case ClientVersionBuild\.V(\d+)_(\d+)_(\d+)_(\d+)', line)
        if case_match:
            major, minor, patch, build = case_match.groups()
            current_cases.append({
                'build': int(build),
                'patch': f"{major}.{minor}.{patch}"
            })
        
        # match: return ClientVersionBuild.V11_2_5_63506;
        return_match = re.search(r'return ClientVersionBuild\.V(\d+)_(\d+)_(\d+)_(\d+)', line)
        if return_match and current_cases:
            def_major, def_minor, def_patch, def_build = return_match.groups()
            parser_version = f"V{def_major}_{def_minor}_{def_patch}_{def_build}"
            
            # all builds in current_cases map to this parser_version
            for case in current_cases:
                mappings[case['build']] = {
                    'patch_version': case['patch'],
                    'parser_version': parser_version
                }
            
            current_cases = []
    
    return mappings

def import_to_database(mappings: dict[int, dict[str, str]]) -> tuple[int, int]:
    # insert all build mappings into database
    conn = psycopg2.connect(**DB_CONFIG)
    cursor = conn.cursor()
    
    inserted = 0
    updated = 0
    
    for build_number, info in sorted(mappings.items()):
        cursor.execute("""
            INSERT INTO builds (build_number, patch_version, parser_version)
            VALUES (%s, %s, %s)
            ON CONFLICT (build_number) 
            DO UPDATE SET 
                patch_version = EXCLUDED.patch_version,
                parser_version = EXCLUDED.parser_version
            RETURNING (xmax = 0) AS was_inserted
        """, (build_number, info['patch_version'], info['parser_version']))
        
        was_inserted = cursor.fetchone()[0]
        if was_inserted:
            inserted += 1
        else:
            updated += 1
    
    conn.commit()
    conn.close()
    
    return inserted, updated

def main():
    print("Importing build mappings to database")
    print()
    
    if len(sys.argv) < 2:
        print("Error: Missing file path")
        print("Usage: ./build_mappings_to_db.py <path-to-Opcodes.cs>")
        print()
        print("Example:")
        print("  ./build_mappings_to_db.py ~/WowPacketParser/Enums/Version/Opcodes.cs")
        sys.exit(1)
    
    wpp_file = Path(sys.argv[1])
    
    if not wpp_file.exists():
        print(f"Error: File not found: {wpp_file}")
        sys.exit(1)
    
    print(f"Parsing: {wpp_file.name}")
    
    with open(wpp_file, 'r') as f:
        content = f.read()
    
    mappings = extract_build_mappings(content)
    
    print(f"Found: {len(mappings)} build mappings")
    print()
    
    try:
        inserted, updated = import_to_database(mappings)
        print(f"Inserted: {inserted}")
        print(f"Updated: {updated}")
        print()
        print("Import complete")
    except psycopg2.Error as e:
        print(f"Error: Database import failed")
        print(f"Details: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()