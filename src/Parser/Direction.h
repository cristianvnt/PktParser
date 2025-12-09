#ifndef DIRECTION_H
#define DIRECTION_H

namespace PktParser
{
	enum class Direction : uint8
	{
		/// <summary>
		/// CMSG opcodes.
		/// </summary>
		ClientToServer = 0,
		/// <summary>
		/// SMSG opcodes.
		/// </summary>
		ServerToClient = 1,
		/// <summary>
		/// Battle.NET Client to Server.
		/// </summary>
		BNClientToServer = 2,
		/// <summary>
		/// Battle.NET Server to Client
		/// </summary>
		BNServerToClient = 3,
		/// <summary>
		/// Bidirectional opcode (MSG, UMSG, TEST...)
		/// </summary>
		Bidirectional = 4
	};
}

#endif // !DIRECTION_H