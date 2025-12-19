#!/usr/bin/env python3
"""
Import opcodes from WPP into PostgreSQL.
Usage:
    python3 import_opcodes.py <path-to-Opcodes.cs>

Example:
    python3 import_opcodes.py ~/WowPacketParser/WowPacketParser/Enums/Version/V11_2_5_63506/Opcodes.cs
"""
import re
import psycopg2
from pathlib import Path
import sys
import os
from dotenv import load_dotenv

ScriptDir = Path(__file__).parent
ProjectRoot = ScriptDir.parent
EnvPath = ProjectRoot / '.env'

if not EnvPath.exists():
    print(f"Error: .env not found at {EnvPath}")
    sys.exit(1)

load_dotenv(EnvPath)

DB_CONFIG = \
{
    'host': os.getenv('POSTGRES_HOST', 'localhost'),
    'port': os.getenv('POSTGRES_PORT', '5432'),
    'dbname': os.getenv('POSTGRES_DB', 'wow_metadata'),
    'user': os.getenv('POSTGRES_USER', 'wowparser'),
    'password': os.getenv('POSTGRES_PASSWORD')
}

if not DB_CONFIG['password']:
    print("Error: POSTGRES_PASSWORD not set in .env")
    sys.exit(1)

def ParseWppOpcodesFile(FilePath):
    """
    Parse WPP opcode file.
    """
    with open(FilePath, 'r', encoding='utf-8') as F:
        Content = F.read()

    NamespacePattern = r'namespace\s+WowPacketParser\.Enums\.Version\.(V\d+_\d+_\d+_\d+)'
    NamespaceMatch = re.search(NamespacePattern, Content)

    if not NamespaceMatch:
        return [], None

    Version = NamespaceMatch.group(1)
    Opcodes = []

    ClientPattern = r'private static readonly BiDictionary<Opcode, int> ClientOpcodes = new\(\)\s*\{(.*?)\};'
    ClientMatch = re.search(ClientPattern, Content, re.DOTALL)
    if ClientMatch:
        Opcodes.extend(ParseOpcodeBlock(ClientMatch.group(1), 'ClientToServer', Version))
    
    ServerPattern = r'private static readonly BiDictionary<Opcode, int> ServerOpcodes = new\(\)\s*\{(.*?)\};'
    ServerMatch = re.search(ServerPattern, Content, re.DOTALL)
    if ServerMatch:
        Opcodes.extend(ParseOpcodeBlock(ServerMatch.group(1), 'ServerToClient', Version))
    
    MiscPattern = r'private static readonly BiDictionary<Opcode, int> MiscOpcodes = new\(\)\s*\{(.*?)\};'
    MiscMatch = re.search(MiscPattern, Content, re.DOTALL)
    if MiscMatch:
        MiscContent = MiscMatch.group(1).strip()
        if MiscContent and not MiscContent.isspace():
            Opcodes.extend(ParseOpcodeBlock(MiscContent, 'Misc', Version))
    
    return Opcodes, Version

def ParseOpcodeBlock(BlockContent, Direction, Version):
    Opcodes = []
    Pattern = r'\{\s*Opcode\.(\w+),\s*0x([0-9A-Fa-f]+)\s*\}'
    Matches = re.findall(Pattern, BlockContent)
    
    for OpcodeName, OpcodeHex in Matches:
        Opcodes.append(
        {
            'Name': OpcodeName,
            'Value': int(OpcodeHex, 16),
            'Direction': Direction,
            'Version': Version
        })
    
    return Opcodes

def ExtractBuildNumber(Version):
    BuildMatch = re.search(r'_(\d{5})$', Version)
    return int(BuildMatch.group(1)) if BuildMatch else 0

def ExtractPatchVersion(Version):
    PatchMatch = re.search(r'V(\d+)_(\d+)_(\d+)_', Version)
    if PatchMatch:
        return f"{PatchMatch.group(1)}.{PatchMatch.group(2)}.{PatchMatch.group(3)}"
    return "unknown"

def ImportToDatabase(Connection, Opcodes, Version):
    Cursor = Connection.cursor()
    
    BuildNumber = ExtractBuildNumber(Version)
    PatchVersion = ExtractPatchVersion(Version)

    Cursor.execute(
    """
        INSERT INTO builds (build_number, patch_version, parser_version)
        VALUES (%s, %s, %s)
        ON CONFLICT (build_number) 
        DO UPDATE SET parser_version = EXCLUDED.parser_version
    """, (BuildNumber, PatchVersion, Version))

    # insert opcodes
    Inserted = 0
    Updated = 0
    
    for Opc in Opcodes:
        Cursor.execute(
        """
            SELECT 1 FROM opcodes 
            WHERE opcode_value = %s AND parser_version = %s
        """, (Opc['Value'], Opc['Version']))
        
        Exists = Cursor.fetchone() is not None

        Cursor.execute(
        """
            INSERT INTO opcodes (opcode_value, opcode_name, parser_version, direction)
            VALUES (%s, %s, %s, %s)
            ON CONFLICT (opcode_value, parser_version) 
            DO UPDATE SET
                opcode_name = EXCLUDED.opcode_name,
                direction = EXCLUDED.direction
        """, (Opc['Value'], Opc['Name'], Opc['Version'], Opc['Direction']))
        
        if Exists:
            Updated += 1
        else:
            Inserted += 1
    
    Connection.commit()
    return Inserted, Updated

def Main():
    print("================================")
    print("Opcode Import MAGIK")
    print("================================")
    print("")

    if len(sys.argv) < 2:
        print("Error: Missing opcode file path")
        print("")
        sys.exit(1)

    OpcodeFile = Path(sys.argv[1])
    
    if not OpcodeFile.exists():
        print(f"Error: File not found")
        print(f"Path: {OpcodeFile}")
        sys.exit(1)

    print(f"File: {OpcodeFile.name}")
    print("")
    
    # parse WPP file
    Opcodes, Version = ParseWppOpcodesFile(OpcodeFile)

    if not Opcodes:
        print("Error: No opcodes found in file")
        sys.exit(1)

    Cmsg = sum(1 for O in Opcodes if O['Direction'] == 'ClientToServer')
    Smsg = sum(1 for O in Opcodes if O['Direction'] == 'ServerToClient')
    Misc = sum(1 for O in Opcodes if O['Direction'] == 'Misc')

    print(f"Parsed: {Version}")
    print(f"    CMSG: {Cmsg}")
    print(f"    SMSG: {Smsg}")
    print(f"    Misc: {Misc}")
    print(f"    Total: {len(Opcodes)}")
    print("")

    # conn to db
    try:
        Conn = psycopg2.connect(**DB_CONFIG)
        print(f"Database: Connected to {DB_CONFIG['dbname']}")
    except Exception as E:
        print(f"Error: Database connection FAILED")
        print(f"Details: {E}")
        sys.exit(1)

    # import to db
    try:
        Inserted, Updated = ImportToDatabase(Conn, Opcodes, Version)
        print(f"    Inserted: {Inserted}")
        print(f"    Updated: {Updated}")
    except Exception as E:
        print(f"Error: Import failed")
        print(f"Details: {E}")
        Conn.close()
        sys.exit(1)

    Conn.close();

if __name__ == "__main__":
    Main()