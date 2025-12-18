
CREATE TABLE IF NOT EXISTS builds
(
    build_number INTEGER PRIMARY KEY,
    patch_version VARCHAR(20) NOT NULL,
    parser_version VARCHAR(50),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_builds_patch ON builds(patch_version);
CREATE INDEX IF NOT EXISTS idx_builds_parser ON builds(parser_version);

CREATE TABLE IF NOT EXISTS opcodes
(
    id SERIAL PRIMARY KEY,
    opcode_value INTEGER NOT NULL,
    opcode_name VARCHAR(100) NOT NULL,
    parser_version VARCHAR(50) NOT NULL,
    direction VARCHAR(20) NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(opcode_value, parser_version)
);

CREATE INDEX IF NOT EXISTS idx_opcodes_value ON opcodes(opcode_value);
CREATE INDEX IF NOT EXISTS idx_opcodes_parser ON opcodes(parser_version);
CREATE INDEX IF NOT EXISTS idx_opcodes_name ON opcodes(opcode_name);
CREATE INDEX IF NOT EXISTS idx_opcodes_direction ON opcodes(direction);

GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO wowparser;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO wowparser;
