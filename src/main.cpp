#include <string>

#include "bitbuf.h"

#include "../lib/urmem/urmem.hpp"
#include "../lib/subhook/subhook.h"

urmem::sig_scanner sigscan;

class INetChannel;
class bf_write;
struct netadr_t;

//urmem::hook hook_NSP;
subhook_t hook_NSP;
typedef int(__cdecl* FnSendPacket)(INetChannel*, int, const netadr_t&, const unsigned char*, int, bf_write*, bool);
int __cdecl NET_SendPacket(INetChannel* chan, int sock, const netadr_t& to, const unsigned char* data, int length, bf_write* pVoicePayload /* = NULL */, bool bUseCompression /*=false*/)
{
	if (length > 5)
	{
		if (length == 115 && data[4] == 73) //as2_info
		{
			bf_read bf;
			bf.bytes = length;
			for (int i = 0; i < bf.bytes; i++) bf.raw[i] = data[i];
			bf.it = bf.raw;

			//header
			bf.Skip(5);

			auto protocol = bf.Read<unsigned char>();
			auto name = bf.ReadString();
			auto map = bf.ReadString();
			auto folder = bf.ReadString();
			auto game = bf.ReadString();
			auto appid = bf.Read<unsigned short>();
			auto players = bf.Read<unsigned char>();

			char hexstr[231];
			int i;
			for (i = 0; i<length; i++) {
				sprintf(hexstr + i * 2, "%02x", data[i]);
			}
			hexstr[i * 2] = 0;

			MessageBoxA(0, std::string(hexstr).c_str(), 0, 0);


			/*char* ptr = (char*)data;
			ptr += 12;
			//*(unsigned char*)(data += 12) = (unsigned char)12;
			*(unsigned char*)ptr = (unsigned char)12;
			MessageBoxA(0, std::to_string(data[12]).c_str(), 0, 0);
			ptr -= 12;*/

			/*//intercept
			players = 9;

			bf_write bfw;
			bfw.data = (char*)data;

			bfw.Skip(5);

			bfw.Write(protocol);
			bfw.WriteString(name);
			bfw.WriteString(map);
			bfw.WriteString(folder);
			bfw.WriteString(game);
			bfw.Write(appid);
			bfw.Write(players);*/
		}
	}

	//return hook_NSP.call<urmem::calling_convention::cdeclcall, int>(chan, sock, to, data, length, pVoicePayload, bUseCompression);
	return ((FnSendPacket)subhook_get_trampoline(hook_NSP))(chan, sock, to, data, length, pVoicePayload, bUseCompression);
	//return 0;
}

void Init()
{
	sigscan.init((unsigned long)GetModuleHandleA("engine.dll"));
	unsigned long func;
	if (!sigscan.find("\x55\x8B\xEC\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x57", "xxxx????x????xx????x", func))
		return;

	hook_NSP = subhook_new((void*)func, (void*)NET_SendPacket, (subhook_flags)0);
	if (subhook_install(hook_NSP) != 0)
		MessageBoxA(0, "fck", 0, 0);

	//hook_NSP.install(urmem::get_func_addr(func), urmem::get_func_addr(&NET_SendPacket), urmem::hook::type::call);
}

DWORD __stdcall DllMain(HMODULE dll, DWORD callreason, void* reserved)
{
	if (callreason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(dll);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Init, NULL, NULL, NULL);

		return 1;
	}

	return 0;
}