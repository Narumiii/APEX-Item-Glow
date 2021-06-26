#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <memory>
#include <string_view>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <random>
#include "offsets.h"
using namespace std;
HWND hWnd;
uintptr_t pID;

uintptr_t moduleBase;

uintptr_t localPlayer;
uintptr_t entList;
uintptr_t viewRenderer;
uintptr_t viewMatrix;

typedef struct _NULL_MEMORY
{
	void* buffer_address;
	UINT_PTR address;
	ULONGLONG size;
	ULONG pid;
	BOOLEAN write;
	BOOLEAN read;
	BOOLEAN req_base;
	void* output;
	const char* module_name;
	ULONG64 base_address;
}NULL_MEMORY;

uintptr_t base_address = 0;
std::uint32_t process_id = 0;

template<typename ... Arg>
uint64_t call_hook(const Arg ... args)
{
	void* hooked_func = GetProcAddress(LoadLibrary("win32u.dll"), "NtDxgkVailPromoteCompositionSurface");

	auto func = static_cast<uint64_t(_stdcall*)(Arg...)>(hooked_func);

	return func(args ...);
}

struct HandleDisposer
{
	using pointer = HANDLE;
	void operator()(HANDLE handle) const
	{
		if (handle != NULL || handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
		}
	}
};

using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;
struct GlowMode
{
	int8_t GeneralGlowMode, BorderGlowMode, BorderSize, TransparentLevel;
};

std::uint32_t get_process_id(std::string_view process_name)
{
	PROCESSENTRY32 processentry;
	const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL));

	if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		return NULL;

	processentry.dwSize = sizeof(MODULEENTRY32);

	while (Process32Next(snapshot_handle.get(), &processentry) == TRUE)
	{
		if (process_name.compare(processentry.szExeFile) == NULL)
		{
			return processentry.th32ProcessID;
		}
	}
	return NULL;
}



static ULONG64 get_module_base_address(const char* module_name)
{
	NULL_MEMORY instructions = { 0 };
	instructions.pid = process_id;
	instructions.req_base = TRUE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.module_name = module_name;
	call_hook(&instructions);
	
	ULONG64 base = NULL;
	base = instructions.base_address;
	return base;
}
template <class T>
T Read(uintptr_t read_address)
{
	T response{};
	NULL_MEMORY instructions;
	instructions.pid = process_id;
	instructions.size = sizeof(T);
	instructions.address = read_address;
	instructions.read = TRUE;
	instructions.write = FALSE;
	instructions.req_base = FALSE;
	instructions.output = &response;
	call_hook(&instructions);

	return response;
}

bool write_memory(UINT_PTR write_address, UINT_PTR source_address, SIZE_T write_size)
{
	NULL_MEMORY instructions;
	instructions.address = write_address;
	instructions.pid = process_id;
	instructions.write = TRUE;
	instructions.read = FALSE;
	instructions.req_base = FALSE;
	instructions.buffer_address = (void*)source_address;
	instructions.size = write_size;

	call_hook(&instructions);

	return true;
}

template<typename S>
bool write(UINT_PTR write_address, const S& value)
{
	return write_memory(write_address, (UINT_PTR)&value, sizeof(S));
}


DWORD64 GetEntityById(int Ent, DWORD64 Base)
{
	DWORD64 EntityList = Base + OFFSET_ENTITYLIST; //updated
	DWORD64 BaseEntity = Read<DWORD64>(EntityList);
	if (!BaseEntity)
	return NULL;
	return  Read<DWORD64>(EntityList + (Ent << 5));
}

std::string random_string(const int len) {
	const std::string alpha_numeric("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");

	std::default_random_engine generator{ std::random_device{}() };
	const std::uniform_int_distribution< std::string::size_type > distribution{ 0, alpha_numeric.size() - 1 };

	std::string str(len, 0);
	for (auto& it : str) {
		it = alpha_numeric[distribution(generator)];
	}

	return str;
}

//{1, { "Kraber", REDORANGE, true, HEIRLOOM }},
//{ 2,{ "Mastiff", RED,  true, SHOTGUN } },
//{ 7,{ "L-Star", MINT, true, ENERGY } },
//{ 12,{ "Havoc", MINT, true, ENERGY } },
//{ 18,{ "Devotion", MINT, true, ENERGY } },
//{ 24,{ "Triple Take", REDORANGE, true, MARKSMAN } },
//{ 25,{ "Flatline", TEAL, true, HEAVY } },
//{ 30,{ "Hemlok", TEAL, true, HEAVY } },
//{ 35,{ "G7 Scout", ORANGE, true, MARKSMAN } },
//{ 41,{ "Alternator", ORANGE, true, LIGHT } },
//{ 47,{ "R-99", ORANGE, true, LIGHT } },
//{ 52,{ "Prowler", REDORANGE, true, HEIRLOOM } },
//{ 57,{ "Volt", MINT, true, ENERGY } },
//{ 62,{ "Longbow", BLURPLE, true, SNIPER } },
//{ 67,{ "Charge Rifle", BLURPLE, true, SNIPER } },
//{ 72,{ "Spitfire", TEAL, true, HEAVY } },
//{ 77,{ "R-301", ORANGE, true, LIGHT } },
//{ 82,{ "EVA-8 Auto", RED, true, SHOTGUN } },
//{ 87,{ "Peacekeeper", REDORANGE, true, HEIRLOOM} },
//{ 92,{ "Mozambique", RED, true, SHOTGUN } },
//{ 98,{ "Wingman", TEAL,  true, HEAVY} },
//{ 103,{ "P2020", ORANGE,  true, LIGHT } },
//{ 109,{ "RE-45", ORANGE, true, LIGHT } },
//{ 115,{ "Sentinel", BLURPLE, true, SNIPER } },
//{ 120,{ "Bocek Bow", YELLOW, true, MARKSMAN} },
//{ 125,{ "Repeater", TEAL, true, MARKSMAN } },
//
//{ 130,{ "Light Rounds", ORANGE, true, LIGHT | AMMO } },
//{ 131,{ "Energy Ammo", MINT, true, ENERGY | AMMO } },
//{ 132,{ "Shotgun Shells", RED, true, SHOTGUN | AMMO } },
//{ 133,{ "Heavy Rounds", TEAL, true, HEAVY | AMMO } },
//{ 134,{ "Sniper Rounds", BLURPLE, true, SNIPER | AMMO } },
//{ 135,{ "Arrows", YELLOW, true, MARKSMAN | AMMO } },
//
//{ 136,{ "Ultimate Accelerant", GREEN, true, CONSUMABLES } },
//{ 137,{ "Phoenix Kit", PURPLE, true, CONSUMABLES } },
//{ 138,{ "Med Kit", BLUE, true, CONSUMABLES } },
//{ 139,{ "Syringe", WHITE, true, CONSUMABLES } },
//{ 140,{ "Shield Battery", BLUE, true, CONSUMABLES } },
//{ 141,{ "Shield Cell", WHITE, true, CONSUMABLES } },
//
//{ 142,{ "Helmet 1", WHITE, true, GEAR } },
//{ 143,{ "Helmet 2", BLUE, true, GEAR } },
//{ 144,{ "Helmet 3", PURPLE, true, GEAR } },
//{ 145,{ "Helmet 4", YELLOW, true, GEAR } },
//{ 146,{ "Body Armor 1", WHITE, true, GEAR } },
//{ 147,{ "Body Armor 2", BLUE, true, GEAR } },
//{ 148,{ "Body Armor 3", PURPLE, true, GEAR } },
//{ 149,{ "Body Armor 4", YELLOW, true, GEAR } },
//{ 151,{ "Evo Shield 1", WHITE, true, GEAR } },
//{ 152,{ "Evo Shield 2", BLUE, true, GEAR } },
//{ 153,{ "Evo Shield 3", PURPLE, true, GEAR } },
//{ 154,{ "Evo Shield 4", REDORANGE, true, GEAR } },
//{ 156,{ "Knockdown Shield 1", WHITE, true, GEAR } },
//{ 157,{ "Knockdown Shield 2", BLUE, true, GEAR } },
//{ 158,{ "Knockdown Shield 3", PURPLE, true, GEAR } },
//{ 159,{ "Knockdown Shield 4", YELLOW, true, GEAR } },
//{ 160,{ "Backpack 1", WHITE, true, GEAR } },
//{ 161,{ "Backpack 2", BLUE, true, GEAR } },
//{ 162,{ "Backpack 3", PURPLE, true, GEAR } },
//{ 163,{ "Backpack 4", YELLOW, true, GEAR } },
//
//{ 164,{ "Thermite Grenade", MAROON, true, THROWABLE } },
//{ 165,{ "Frag Grenade", MAROON, true, THROWABLE } },
//{ 166,{ "Arc Star", MAROON, true, THROWABLE } },
//
//{ 167,{ "1x HCOG (Classic)", WHITE, true, ATTACHMENTS_SIGHTS } },
//{ 168,{ "2x HCOG (Bruiser)", BLUE, true, ATTACHMENTS_SIGHTS } },
//{ 169,{ "1x Holo", WHITE, true, ATTACHMENTS_SIGHTS } },
//{ 170,{ "1x-2x Variable Holo", BLUE, true, ATTACHMENTS_SIGHTS } },
//{ 171,{ "1x Digital Threat", YELLOW, true, ATTACHMENTS_SIGHTS } },
//{ 172,{ "3x HCOG (Ranger)", PURPLE, true, ATTACHMENTS_SIGHTS } },
//{ 173,{ "2x-4x Variable AOG", PURPLE, true, ATTACHMENTS_SIGHTS } },
//{ 174,{ "6x Sniper", BLUE, true, ATTACHMENTS_SIGHTS } },
//{ 175,{ "4x-8x Variable Sniper", PURPLE, true, ATTACHMENTS_SIGHTS } },
//{ 176,{ "4x-10x Digital Sniper Threat", YELLOW, true, ATTACHMENTS_SIGHTS } },
//
//{ 177,{ "Barrel Stabilizer 1", WHITE, true, LIGHT | HEAVY | SNIPER | ENERGY | ATTACHMENTS_GENERAL } },
//{ 178,{ "Barrel Stabilizer 2", BLUE, true, LIGHT | HEAVY | SNIPER | ENERGY | ATTACHMENTS_GENERAL } },
//{ 179,{ "Barrel Stabilizer 3", PURPLE, true, LIGHT | HEAVY | SNIPER | ENERGY | ATTACHMENTS_GENERAL } },
//{ 180,{ "Barrel Stabilizer 4", YELLOW, true, LIGHT | HEAVY | SNIPER | ENERGY | ATTACHMENTS_GENERAL } },
//{ 181,{ "Light Magazine 1", WHITE, true, LIGHT | ATTACHMENTS_GENERAL } },
//{ 182,{ "Light Magazine 2", BLUE, true, LIGHT | ATTACHMENTS_GENERAL } },
//{ 183,{ "Light Magazine 3", PURPLE, true, LIGHT | ATTACHMENTS_GENERAL } },
//{ 184,{ "Light Magazine 4", YELLOW, true, LIGHT | ATTACHMENTS_GENERAL } },
//{ 185,{ "Heavy Magazine 1", WHITE, true, HEAVY | ATTACHMENTS_GENERAL } },
//{ 186,{ "Heavy Magazine 2", BLUE, true, HEAVY | ATTACHMENTS_GENERAL } },
//{ 187,{ "Heavy Magazine 3", PURPLE, true, HEAVY | ATTACHMENTS_GENERAL } },
//{ 188,{ "Heavy Magazine 4", YELLOW, true, HEAVY | ATTACHMENTS_GENERAL } },
//{ 189,{ "Energy Magazine 1", WHITE, true, ENERGY | ATTACHMENTS_GENERAL } },
//{ 190,{ "Energy Magazine 2", BLUE, true, ENERGY | ATTACHMENTS_GENERAL } },
//{ 191,{ "Energy Magazine 3", PURPLE, true, ENERGY | ATTACHMENTS_GENERAL } },
//{ 192,{ "Energy Magazine 4", YELLOW, true, ENERGY | ATTACHMENTS_GENERAL } },
//{ 193,{ "Sniper Magazine 1", WHITE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 194,{ "Sniper Magazine 2", BLUE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 195,{ "Sniper Magazine 3", PURPLE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 196,{ "Sniper Magazine 4", YELLOW, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 197,{ "Shotgun Bolt 1", WHITE,  true, SHOTGUN | ATTACHMENTS_GENERAL } },
//{ 198,{ "Shotgun Bolt 2", BLUE,  true, SHOTGUN | ATTACHMENTS_GENERAL } },
//{ 199,{ "Shotgun Bolt 3", PURPLE,  true, SHOTGUN | ATTACHMENTS_GENERAL } },
//{ 200,{ "Standard Stock 1", WHITE, true, LIGHT | HEAVY | ENERGY | ATTACHMENTS_GENERAL } },
//{ 201,{ "Standard Stock 2", BLUE, true, LIGHT | HEAVY | ENERGY | ATTACHMENTS_GENERAL } },
//{ 202,{ "Standard Stock 3", PURPLE, true, LIGHT | HEAVY | ENERGY | ATTACHMENTS_GENERAL } },
//{ 203,{ "Sniper Stock 1", WHITE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 204,{ "Sniper Stock 2", BLUE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 205,{ "Sniper Stock 3", PURPLE, true, SNIPER | ATTACHMENTS_GENERAL } },
//{ 206,{ "Turbocharger", YELLOW,  true, ATTACHMENTS_HOP_UP } },
//{ 209,{ "Anvil Receiver", YELLOW, true, LIGHT | HEAVY } },
//{ 211,{ "Deadeye's Tempo", PURPLE, true, MARKSMAN } },
//{ 212,{ "Quickdraw Holster", PURPLE, true, LIGHT | HEAVY } },
//{ 213,{ "Shatter Caps", PURPLE, true, MARKSMAN } },
//{ 214,{ "Vault Key", RED, true, CONSUMABLES } },
//{ 215,{ "Respawn Beacon", BLUE, true, CONSUMABLES } },
//{ 217,{ "Treasure Pack", RED, true, CONSUMABLES } },

int main()
{
	SetConsoleTitle(random_string(24).c_str());

	if (hWnd == 0)
	{
		hWnd = FindWindow(NULL, "Apex Legends");
		cout << "[+] Found Apex!!" << endl;
	}
	else
	{
		cout << "[+] WTFFF MEN!!" << endl;
	}

	process_id = get_process_id("r5apex.exe");
	base_address = get_module_base_address("r5apex.exe");
	cout << process_id << endl;
	cout << base_address << endl;

	while (true)  // loop function
	{
		for (int i = 0; i < 10000; i++)
		{
			DWORD64 Entity = GetEntityById(i, base_address);
			if (Entity == 0)
			continue;

			int itemid = Read<int>(Entity + OFFSET_ITEM_ID);

			if (itemid == 41 || itemid == 77 || itemid == 109) //Item ID to shine
			{
				write<int>(Entity + 0x3C8, 1);
				write<int>(Entity + 0x3D0, 2);
				write<GlowMode>(Entity + 0x2C4, { 101,102,46,96 });
				write<float>(Entity + 0x1D0, 0.f);
				write<float>(Entity + 0x1D4, 122.f);
				write<float>(Entity + 0x1D8, 122.f);
			}
		}
	}
}