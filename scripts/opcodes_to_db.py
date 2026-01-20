#!/usr/bin/env python3
import re
import psycopg2
from pathlib import Path
import sys
import os
from dotenv import load_dotenv

BUILD_NUMBER_PATTERN = r'_(\d{5})$'
PATCH_VERSION_PATTERN = r'V(\d+)_(\d+)_(\d+)_'
NAMESPACE_PATTERN = r'namespace\s+WowPacketParser\.Enums\.Version\.(V\d+_\d+_\d+_\d+)'

ScriptDir = Path(__file__).parent
ProjectRoot = ScriptDir.parent
EnvPath = ProjectRoot / '.env'

if not EnvPath.exists():
    print(f"Error: .env not found at {EnvPath}")
    sys.exit(1)

load_dotenv(EnvPath)

DB_CONFIG = {
    'host': os.getenv('POSTGRES_HOST', 'localhost'),
    'port': os.getenv('POSTGRES_PORT', '5432'),
    'dbname': os.getenv('POSTGRES_DB', 'wow_metadata'),
    'user': os.getenv('POSTGRES_USER', 'wowparser'),
    'password': os.getenv('POSTGRES_PASSWORD')
}

if not DB_CONFIG['password']:
    print("Error: POSTGRES_PASSWORD not set in .env")
    sys.exit(1)

def parse_wpp_opcodes_file(file_path: Path) -> tuple[list[dict], str | None]:
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # extract version from namespace
    namespace_match = re.search(NAMESPACE_PATTERN, content)
    if not namespace_match:
        return [], None
    
    version = namespace_match.group(1)
    opcodes = []

    # parse each opcode block
    patterns = [
        (r'private static readonly BiDictionary<Opcode, int> ClientOpcodes = new\(\)\s*\{(.*?)\};', 'ClientToServer'),
        (r'private static readonly BiDictionary<Opcode, int> ServerOpcodes = new\(\)\s*\{(.*?)\};', 'ServerToClient'),
        (r'private static readonly BiDictionary<Opcode, int> MiscOpcodes = new\(\)\s*\{(.*?)\};', 'Misc')
    ]

    for pattern, direction in patterns:
        match = re.search(pattern, content, re.DOTALL)
        if match:
            block_content = match.group(1).strip()
            if block_content and not block_content.isspace():
                opcodes.extend(parse_opcode_block(block_content, direction, version))
    
    return opcodes, version

def parse_opcode_block(block_content: str, direction: str, version: str) -> list[dict]:
    # extract individual opcode entries from a block.
    opcodes = []
    pattern = r'\{\s*Opcode\.(\w+),\s*0x([0-9A-Fa-f]+)\s*\}'
    matches = re.findall(pattern, block_content)
    
    for opcode_name, opcode_hex in matches:
        opcodes.append({
            'name': opcode_name,
            'value': int(opcode_hex, 16),
            'direction': direction,
            'version': version
        })
    
    return opcodes

def extract_build_number(version: str) -> int:
    # extract 5-digit build number from version string
    match = re.search(BUILD_NUMBER_PATTERN, version)
    return int(match.group(1)) if match else 0

def extract_patch_version(version: str) -> str:
    # extract patch version from version string
    match = re.search(PATCH_VERSION_PATTERN, version)
    if match:
        return f"{match.group(1)}.{match.group(2)}.{match.group(3)}"
    return "unknown"

def import_to_database(conn: psycopg2.extensions.connection, opcodes: list[dict], version: str) -> tuple[int, int]:
    # import opcodes to database
    cursor = conn.cursor()
    
    # insert build metadata
    build_number = extract_build_number(version)
    patch_version = extract_patch_version(version)
    
    cursor.execute("""
        INSERT INTO builds (build_number, patch_version, parser_version)
        VALUES (%s, %s, %s)
        ON CONFLICT (build_number) 
        DO UPDATE SET parser_version = EXCLUDED.parser_version
    """, (build_number, patch_version, version))
    
    inserted = 0
    updated = 0
    
    for opc in opcodes:
        cursor.execute("""
            INSERT INTO opcodes (opcode_value, opcode_name, parser_version, direction)
            VALUES (%s, %s, %s, %s)
            ON CONFLICT (opcode_value, parser_version) 
            DO UPDATE SET
                opcode_name = EXCLUDED.opcode_name,
                direction = EXCLUDED.direction
            RETURNING (xmax = 0) AS was_inserted
        """, (opc['value'], opc['name'], opc['version'], opc['direction']))
        
        # xmax = 0 -> inserted new row, xmax != 0 -> updated existing row
        was_inserted = cursor.fetchone()[0]
        if was_inserted:
            inserted += 1
        else:
            updated += 1
    
    conn.commit()
    return inserted, updated

def main():
    print("Importing opcodes to database")
    print()
    
    if len(sys.argv) < 2:
        print("Error: Missing opcode file path")
        print("Usage: ./opcodes_to_db.py <path-to-Opcodes.cs>")
        print()
        print("Example:")
        print("  ./opcodes_to_db.py ~/WowPacketParser/Enums/Version/VX_X_X_XXXX/Opcodes.cs")
        sys.exit(1)
    
    opcode_file = Path(sys.argv[1])
    
    if not opcode_file.exists():
        print(f"Error: File not found: {opcode_file}")
        sys.exit(1)
    
    print(f"Parsing: {opcode_file.name}")
    
    opcodes, version = parse_wpp_opcodes_file(opcode_file)
    
    if not opcodes:
        print("Error: No opcodes found in file")
        sys.exit(1)
    
    # count by direction
    cmsg_count = sum(1 for o in opcodes if o['direction'] == 'ClientToServer')
    smsg_count = sum(1 for o in opcodes if o['direction'] == 'ServerToClient')
    misc_count = sum(1 for o in opcodes if o['direction'] == 'Misc')
    
    print(f"Version: {version}")
    print(f"  CMSG: {cmsg_count}")
    print(f"  SMSG: {smsg_count}")
    print(f"  Misc: {misc_count}")
    print(f"  Total: {len(opcodes)}")
    print()
    
    # connect to database
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        print(f"Connected to database: {DB_CONFIG['dbname']}")
    except psycopg2.Error as e:
        print(f"Error: Database connection failed")
        print(f"Details: {e}")
        sys.exit(1)
    
    # import opcodes
    try:
        inserted, updated = import_to_database(conn, opcodes, version)
        print(f"  Inserted: {inserted}")
        print(f"  Updated: {updated}")
        print()
        print("Import complete")
    except psycopg2.Error as e:
        print(f"Error: Import failed")
        print(f"Details: {e}")
        conn.close()
        sys.exit(1)
    
    conn.close()

if __name__ == "__main__":
    main()