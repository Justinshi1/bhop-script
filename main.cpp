#include "memory.h"

#include <thread>
#include <iostream>

namespace offsets
{
	constexpr auto localPlayer = 0xDA747C;
	constexpr auto flags = 0x104;
	constexpr auto forceJump = 0x526B590;
	constexpr auto entityList = 0x4DC177C;
	constexpr auto glowObjectManager = 0x5309C78;
	constexpr auto teamNum = 0xF4;
	constexpr auto glowIndex = 0x10488;
}

__declspec(align(16)) struct Color
{
	constexpr Color(const float r, const float g, const float b, const float a = 1.f) noexcept :
		r(r), g(g), b(b), a(a) { }

	float r, g, b, a;
};

int main()
{
	const auto mem = Memory("csgo.exe");

	std::cout << "csgo found!" << std::endl;

	const auto client = mem.GetModuleAddress("client.dll");
	std::cout << "client.dll -> " << "0x" << std::hex << client << std::dec << std::endl;

	constexpr const auto color = Color{ 1.f, 0.f, 1.f };

	while (true)
	{
		// we don't need this running a billion times per second :)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		const auto localPlayer = mem.Read<std::uintptr_t>(client + offsets::localPlayer);

		if (!localPlayer)
			continue;

		const auto localPlayerTeam = mem.Read<std::uintptr_t>(localPlayer + offsets::teamNum);
		const auto localPlayerFlags = mem.Read<std::uintptr_t>(localPlayer + offsets::flags);

		// bhop
		if (GetAsyncKeyState(VK_SPACE))
			(localPlayerFlags & (1 << 0)) ?
			mem.Write<std::uintptr_t>(client + offsets::forceJump, 6) :
			mem.Write<std::uintptr_t>(client + offsets::forceJump, 4);

		// glow
		const auto glowObjectManager = mem.Read<std::uintptr_t>(client + offsets::glowObjectManager);

		for (auto i = 1; i <= 32; ++i)
		{
			const auto entity = mem.Read<std::uintptr_t>(client + offsets::entityList + i * 0x10);

			if (!entity)
				continue;

			// dont glow if they are on our team
			if (mem.Read<std::uintptr_t>(entity + offsets::teamNum) == localPlayerTeam)
				continue;

			const auto glowIndex = mem.Read<std::int32_t>(entity + offsets::glowIndex);

			mem.Write<Color>(glowObjectManager + (glowIndex * 0x38) + 0x8, color);

			mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
			mem.Write<bool>(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
		}
	}
}
