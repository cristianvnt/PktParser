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

def ToPascalCase(snake_case):
    if snake_case.startswith("SMSG_"):
        snake_case = snake_case[5:]
    elif snake_case.startswith("CMSG_"):
        snake_case = snake_case[5:]
    
    parts = snake_case.split('_')
    return 'Parse' + ''.join(word.capitalize() for word in parts)

def GenerateParserFiles(parserVersion, versionDir):
    conn = psycopg2.connect(**DB_CONFIG)
    cursor = conn.cursor()
    
    cursor.execute(
    """
        SELECT opcode_name, opcode_value, direction
        FROM opcodes
        WHERE parser_version = %s
        ORDER BY opcode_name
    """, (parserVersion,))
    
    opcodes = cursor.fetchall()
    conn.close()
    
    if not opcodes:
        print(f"No opcodes found for {parserVersion}")
        return
    
    namespace = parserVersion
    outputDir = Path(versionDir)
    outputDir.mkdir(parents=True, exist_ok=True)

    opcodeCount = len(opcodes)
    # calc next power of 2 for optimal hash table size
    reserveSize = 2 ** math.ceil(math.log2(opcodeCount)) if opcodeCount > 0 else 16
    
    GenerateOpcodesHeader(namespace, opcodes, outputDir / "Opcodes.h")
    GenerateRegistrationFile(namespace, opcodes, opcodeCount, reserveSize, outputDir / "RegisterHandlers.inl")
    
    print(f"Generated for {parserVersion}:")
    print(f"  - Opcodes.h ({len(opcodes)} opcodes)")
    print(f"  - RegisterHandlers.inl ({len(opcodes)} registrations)")

def GenerateOpcodesHeader(namespace, opcodes, outputPath):
    header = f"""// AUTO-GENERATED from database - DO NOT EDIT
// Parser version: {namespace}
// Total opcodes: {len(opcodes)}

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
    
    with open(outputPath, 'w') as f:
        f.write(header)

def GenerateRegistrationFile(namespace, opcodes, opcodeCount, reserveSize, outputPath):
    registration = f"""// AUTO-GENERATED from database - DO NOT EDIT
// Parser version: {namespace}
// Opcode count: {opcodeCount}
// Reserve size: {reserveSize}

#ifndef REGISTER_HANDLERS_{namespace.upper()}_INL
#define REGISTER_HANDLERS_{namespace.upper()}_INL

#include "Opcodes.h"

namespace PktParser::Versions::{namespace}
{{
    constexpr size_t OPCODE_COUNT = {opcodeCount};
    constexpr size_t REGISTRY_RESERVE_SIZE = {reserveSize};
    
    template<typename ParserType, typename RegistryType>
    inline void RegisterAllHandlers(ParserType*, RegistryType& registry)
    {{
"""
    
    for name, value, direction in opcodes:
        handlerName = ToPascalCase(name)
        registration += f"        registry.Register(Opcodes::{name}, &ParserType::{handlerName});\n"
    
    registration += """    }
}

#endif
"""
    
    with open(outputPath, 'w') as f:
        f.write(registration)

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python3 generate_opcodes.py <parser_version>")
        sys.exit(1)
    
    parserVersion = sys.argv[1]
    versionDir = f"src/Parser/Versions/{parserVersion}"
    
    GenerateParserFiles(parserVersion, versionDir)