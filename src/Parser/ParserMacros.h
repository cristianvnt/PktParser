#ifndef PARSER_MACROS_H
#define PARSER_MACROS_H

#define IMPLEMENT_SERIALIZER(Namespace, SerializerClass) \
    SerializerClass* Namespace::Parser::GetSerializer() \
    { \
        static SerializerClass serializer; \
        return &serializer; \
    }

#define IMPLEMENT_OPCODE_LOOKUP(Namespace) \
    char const* Namespace::Parser::GetOpcodeName(uint32 opcode) const \
    { \
        auto it = OpcodeNames.find(opcode); \
        return it != OpcodeNames.end() ? it->second : "UNKNOWN_OPCODE"; \
    }

#define BEGIN_PARSER_HANDLER(Namespace) \
    ParserMethod Namespace::Parser::GetParserMethod(uint32 opcode) const \
    { \
        switch (opcode) \
        {
#define REGISTER_HANDLER(OpcodeEnum, HandlerFunc) \
            case Opcodes::OpcodeEnum: \
                return &HandlerFunc;
#define END_PARSER_HANDLER() \
            default: \
                return nullptr; \
        } \
    }

#endif