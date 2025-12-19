#!/usr/bin/env python3
"""
Parse WPP's GetOpcodeDefiningBuild and populate build mappings.
"""
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

def ExtractBuildMappings(content):
    mappings = {}
    
    lines = content.split('\n')
    currentCases = []
    
    for line in lines:
        caseMatch = re.search(r'case ClientVersionBuild\.V(\d+)_(\d+)_(\d+)_(\d+)', line)
        if caseMatch:
            major, minor, patch, build = caseMatch.groups()
            currentCases.append({
                'build': int(build),
                'patch': f"{major}.{minor}.{patch}"
            })
        
        returnMatch = re.search(r'return ClientVersionBuild\.V(\d+)_(\d+)_(\d+)_(\d+)', line)
        if returnMatch and currentCases:
            defMajor, defMinor, defPatch, defBuild = returnMatch.groups()
            parserVersion = f"V{defMajor}_{defMinor}_{defPatch}_{defBuild}"
            
            for case in currentCases:
                mappings[case['build']] = \
                {
                    'patch_version': case['patch'],
                    'parser_version': parserVersion
                }
            
            currentCases = []
    
    return mappings

def SeedDatabase(mappings):
    """Insert all build mappings into database"""
    conn = psycopg2.connect(**DB_CONFIG)
    cursor = conn.cursor()
    
    inserted = 0
    updated = 0
    
    for buildNumber, info in sorted(mappings.items()):
        cursor.execute(
        """
            SELECT 1 FROM builds WHERE build_number = %s
        """, (buildNumber,))
        
        exists = cursor.fetchone() is not None

        cursor.execute(
        """
            INSERT INTO builds (build_number, patch_version, parser_version)
            VALUES (%s, %s, %s)
            ON CONFLICT (build_number) 
            DO UPDATE SET 
                patch_version = EXCLUDED.patch_version,
                parser_version = EXCLUDED.parser_version
        """, (buildNumber, info['patch_version'], info['parser_version']))
        
        if exists == 1:
            updated += 1
        else:
            inserted += 1
    
    conn.commit()
    conn.close()
    
    return inserted, updated

def Main():
    if len(sys.argv) < 2:
        print("Usage: python3 import_build_mappings.py <path-to-Opcodes.cs>")
        sys.exit(1)
    
    wppFile = Path(sys.argv[1])
    
    if not wppFile.exists():
        print(f"Error: File not found: {wppFile}")
        sys.exit(1)
    
    print(f"File: {wppFile.name}")
    print("")
    
    with open(wppFile, 'r') as f:
        content = f.read()
    
    print("Parsing build mappings...")
    mappings = ExtractBuildMappings(content)
    
    print(f"  Found: {len(mappings)} builds")
    print("")
    
    print("Importing to database...")
    inserted, updated = SeedDatabase(mappings)
    
    print(f"  Inserted: {inserted}")
    print(f"  Updated: {updated}")

if __name__ == "__main__":
    Main()