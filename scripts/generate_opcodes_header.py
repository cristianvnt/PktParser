#!/usr/bin/env python3
import psycopg2
import os
import math
from pathlib import Path
from dotenv import load_dotenv

load_dotenv()

DB_CONFIG = \
{
    'host': os.getenv('POSTGRES_HOST', 'localhost'),
    'port': os.getenv('POSTGRES_PORT', '5432'),
    'dbname': os.getenv('POSTGRES_DB', 'wow_metadata'),
    'user': os.getenv('POSTGRES_USER', 'wowparser'),
    'password': os.getenv('POSTGRES_PASSWORD')
}

def to_pascal_case(opcode_name: str) -> str:
    # convert opcode name to parser function name
    if opcode_name.startswith("SMSG_"):
        opcode_name = opcode_name[5:]
    elif opcode_name.startswith("CMSG_"):
        opcode_name = opcode_name[5:]
    
    parts = opcode_name.split('_')
    return 'Parse' + ''.join(word.capitalize() for word in parts)

def generate_opcodes_header(namespace: str, opcodes: list, output_path: Path):
    # generate Opcodes.h file with opcodes
    header = f"""// AUTO-GENERATED from database - DO NOT EDIT
// parser version: {namespace}
// total opcodes: {len(opcodes)}

#ifndef OPCODES_{namespace.upper()}_H
#define OPCODES_{namespace.upper()}_H

#include "Misc/Define.h"

namespace PktParser::Versions::{namespace}::Opcodes
{{
"""
    
    for name, value, direction in opcodes:
        header += f"    constexpr uint32 {name} = 0x{value:X};\n"
    
    header += """
}

#endif
"""
    
    with open(output_path, 'w') as f:
        f.write(header)

def generate_registration_file(namespace: str, opcodes: list, opcode_count: int, reserve_size: int, output_path: Path):
    # generate RegisterHandlers.inl file with handler registrations
    registration = f"""// AUTO-GENERATED from database - DO NOT EDIT
// parser version: {namespace}
// opcode count: {opcode_count}
// reserve size: {reserve_size}

#ifndef REGISTER_HANDLERS_{namespace.upper()}_INL
#define REGISTER_HANDLERS_{namespace.upper()}_INL

#include "Opcodes.h"

namespace PktParser::Versions::{namespace}
{{
    constexpr size_t OPCODE_COUNT = {opcode_count};
    constexpr size_t REGISTRY_RESERVE_SIZE = {reserve_size};
    
    template<typename ParserType, typename RegistryType>
    inline void RegisterAllHandlers(ParserType*, RegistryType& registry)
    {{
"""
    
    for name, value, direction in opcodes:
        handler_name = to_pascal_case(name)
        registration += f"        registry.Register(Opcodes::{name}, &ParserType::{handler_name});\n"
    
    registration += """    }
}

#endif
"""
    
    with open(output_path, 'w') as f:
        f.write(registration)

def generate_parser_files(parser_version: str, version_dir: str):
    # generate C++ opcode files for a parser version
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cursor = conn.cursor()
        
        cursor.execute("""
            SELECT opcode_name, opcode_value, direction
            FROM opcodes
            WHERE parser_version = %s
            ORDER BY opcode_name
        """, (parser_version,))
        
        opcodes = cursor.fetchall()
        conn.close()
        
        if not opcodes:
            print(f"Error: No opcodes found for {parser_version}")
            return
        
        output_dir = Path(version_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        opcode_count = len(opcodes)
        
        reserve_size = 2 ** math.ceil(math.log2(opcode_count)) if opcode_count > 0 else 16
        
        generate_opcodes_header(parser_version, opcodes, output_dir / "Opcodes.h")
        generate_registration_file(parser_version, opcodes, opcode_count, reserve_size, output_dir / "RegisterHandlers.inl")
        
        print(f"Generated for {parser_version}:")
        print(f"  Opcodes.h ({opcode_count} opcodes)")
        print(f"  RegisterHandlers.inl ({opcode_count} registrations)")
        print(f"  Output: {output_dir}")
        
    except psycopg2.Error as e:
        print(f"Error: Database operation failed")
        print(f"Details: {e}")
        exit(1)

def main():
    print("Generating C++ opcode files from database")
    print()
    
    if len(sys.argv) < 2:
        print("Error: Missing parser version")
        print("Usage: ./generate_opcodes_header.py <parser_version>")
        print()
        print("Example:")
        print("  ./generate_opcodes_header.py V11_2_5_63506")
        exit(1)
    
    parser_version = sys.argv[1]
    version_dir = f"src/Parser/Versions/{parser_version}"
    
    generate_parser_files(parser_version, version_dir)

if __name__ == "__main__":
    import sys
    main()