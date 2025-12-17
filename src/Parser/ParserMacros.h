#ifndef PARSER_MACROS_H
#define PARSER_MACROS_H

#define IMPLEMENT_SERIALIZER(Namespace, SerializerClass) \
    SerializerClass* Namespace::Parser::GetSerializer() \
    { \
        static SerializerClass serializer; \
        return &serializer; \
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