#include "function.h"
#include "overlay.h"
#include "driver.h"
#include "xorstr.h"

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

IDirect3DTexture9* bgs = nullptr;

namespace OverlayWindow
{
	WNDCLASSEX WindowClass;
	HWND Hwnd;
	LPCSTR Name;
}

void PrintPtr(std::string text, uintptr_t ptr) {
	std::cout << text << ptr << std::endl;
}


enum bones
{
	Root = 0,
	pelvis = 1,
	thigh_l = 2,
	thigh_twist_01_l = 3,
	calf_l = 4,
	calf_twist_01_l = 5,
	foot_l = 6,
	ball_l = 7,
	thigh_r = 8,
	thigh_twist_01_r = 9,
	calf_r = 10,
	calf_twist_01_r = 11,
	foot_r = 12,
	ball_r = 13,
	spine_01 = 14,
	spine_02 = 15,
	spine_03 = 16,
	clavicle_r = 17,
	upperarm_r = 18,
	upperarm_twist_01_r = 19,
	lowerarm_r = 20,
	lowerarm_twist_01_r = 21,
	hand_r = 22,
	thumb_01_r = 23,
	thumb_02_r = 24,
	thumb_03_r = 25,
	index_01_r = 26,
	index_02_r = 27,
	index_03_r = 28,
	middle_01_r = 29,
	middle_02_r = 30,
	middle_03_r = 31,
	ring_01_r = 32,
	ring_02_r = 33,
	ring_03_r = 34,
	pinky_01_r = 35,
	pinky_02_r = 36,
	pinky_03_r = 37,
	Weapon = 38,
	clavicle_l = 39,
	upperarm_l = 40,
	upperarm_twist_01_l = 41,
	lowerarm_l = 42,
	lowerarm_twist_01_l = 43,
	hand_l = 44,
	thumb_01_l = 45,
	thumb_02_l = 46,
	thumb_03_l = 47,
	index_01_l = 48,
	index_02_l = 49,
	index_03_l = 50,
	middle_01_l = 51,
	middle_02_l = 52,
	middle_03_l = 53,
	ring_01_l = 54,
	ring_02_l = 55,
	ring_03_l = 56,
	pinky_01_l = 57,
	pinky_02_l = 58,
	pinky_03_l = 59,
	neck_01 = 60,
	head = 61,
	Camera_FP = 62,
	VB_LH_Target = 63,
	VB_Control = 64,
	VB_IK_foot_L = 65,
	VB_IK_foot_R = 66,
	VB_IK_foot_root = 67
};

namespace DirectX9Interface
{
	IDirect3D9Ex* Direct3D9 = NULL;
	IDirect3DDevice9Ex* pDevice = NULL;
	D3DPRESENT_PARAMETERS pParams = { NULL };
	MARGINS Margin = { -1 };
	MSG Message = { NULL };
}
typedef struct _EntityList
{
	uintptr_t actor_pawn;
	uintptr_t actor_mesh;
	uintptr_t actor_state;
	Vector3 actor_pos;
	int actor_id;
	string actor_name;

	string bot_name;
	Vector3 bot_pos;
	int bot_id;

	uintptr_t item_pawn;
	Vector3 item_pos;
	int item_id;
	string item_name;

	string Ships_name;
	Vector3 Ships_pos;
	int Ships_id;
}EntityList;
std::vector<EntityList> entityAllList;
std::vector<EntityList> entityList;
std::vector<EntityList> entityBotList;
std::vector<EntityList> entityShipsList;

auto CallAimbot() -> VOID
{
	while (true)
	{
		auto EntityList_Copy = entityList;

		bool isAimbotActive = CFG.b_Aimbot && GetAimKey();
		if (isAimbotActive)
		{
			float target_dist = FLT_MAX;
			EntityList target_entity = {};

			for (int index = 0; index < EntityList_Copy.size(); ++index)
			{
				auto Entity = EntityList_Copy[index];

				auto local_pos = read<Vector3>(GameVars.local_player_root + GameOffset.offset_relative_location);
				auto bone_pos = GetBoneWithRotation(Entity.actor_mesh, 0);
				auto entity_distance = local_pos.Distance(bone_pos);

				auto Health = read<float>(Entity.actor_pawn + GameOffset.offset_health);

				if (!Entity.actor_mesh)
					continue;

				if (Health > 0 && entity_distance < CFG.max_distanceAIM)
				{
					int boneSelect;
					if (CFG.boneType == 0)
					{
						boneSelect = 61;
					}
					else if (CFG.boneType == 1)
					{
						boneSelect = 60;
					}
					else if (CFG.boneType == 2)
					{
						boneSelect = 16;
					}

					auto head_pos = GetBoneWithRotation(Entity.actor_mesh, boneSelect);
					auto targethead = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z));

					float x = targethead.x - GameVars.ScreenWidth / 2.0f;
					float y = targethead.y - GameVars.ScreenHeight / 2.0f;
					float crosshair_dist = sqrtf((x * x) + (y * y));

					if (crosshair_dist <= FLT_MAX && crosshair_dist <= target_dist)
					{
						if (crosshair_dist > CFG.AimbotFOV) // FOV
							continue;

						target_dist = crosshair_dist;
						target_entity = Entity;

					}
				}


			}

			if (target_entity.actor_mesh != 0 || target_entity.actor_pawn != 0 || target_entity.actor_id != 0)
			{
				int boneSelect;
				if (CFG.boneType == 0)
				{
					boneSelect = 61;
				}
				else if (CFG.boneType == 1)
				{
					boneSelect = 60;
				}
				else if (CFG.boneType == 2)
				{
					boneSelect = 16;
				}

				if (target_entity.actor_pawn == GameVars.local_player_pawn)
					continue;

				if (!isVisible(target_entity.actor_mesh))
					continue;

				auto head_pos = GetBoneWithRotation(target_entity.actor_mesh, boneSelect);
				auto targethead = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z));
				move_to(targethead.x + 12, targethead.y - 12);
			}
		}
		//Sleep(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

auto GameCache()->VOID
{
	while (true)
	{
		std::vector<EntityList> tmpList;
		//std::vector<EntityList> entityBot;

		GameVars.u_world = read<DWORD_PTR>(GameVars.dwProcess_Base + GameOffset.offset_u_world);
		GameVars.game_instance = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_game_instance);
		GameVars.local_player_array = read<DWORD_PTR>(GameVars.game_instance + GameOffset.offset_local_players_array);
		GameVars.local_player = read<DWORD_PTR>(GameVars.local_player_array);
		GameVars.local_player_controller = read<DWORD_PTR>(GameVars.local_player + GameOffset.offset_player_controller);
		GameVars.local_player_pawn = read<DWORD_PTR>(GameVars.local_player_controller + GameOffset.offset_apawn);
		GameVars.local_player_root = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_root_component);
		GameVars.local_player_state = read<DWORD_PTR>(GameVars.local_player_pawn + GameOffset.offset_player_state);
		GameVars.persistent_level = read<DWORD_PTR>(GameVars.u_world + GameOffset.offset_persistent_level);
		GameVars.actors = read<DWORD_PTR>(GameVars.persistent_level + GameOffset.offset_actor_array);
		GameVars.actor_count = read<int>(GameVars.persistent_level + GameOffset.offset_actor_count);
		
		//PrintPtr("game instance ", GameVars.game_instance);
		//PrintPtr("L Player Array ", GameVars.local_player_array);
		//PrintPtr("L Player ", GameVars.local_player);
		//PrintPtr("L Player Controller ", GameVars.local_player_controller);
		//PrintPtr("L Player Pawn ", GameVars.local_player_pawn);
		//PrintPtr("L Player Root ", GameVars.local_player_root);
		//PrintPtr("L Player State ", GameVars.local_player_state);
		//PrintPtr("P Level ", GameVars.persistent_level);
		//PrintPtr("Actors ", GameVars.actors);
		//PrintPtr("Actor Count ", GameVars.actor_count);
		
		for (int index = 0; index < GameVars.actor_count; ++index)
		{

			auto actor_pawn = read<uintptr_t>(GameVars.actors + index * 0x8);
			//if (actor_pawn == 0x00)
				//continue;

			//if (actor_pawn == GameVars.local_player_pawn)
			//	continue;

			auto actor_id = read<int>(actor_pawn + GameOffset.offset_actor_id);
			auto actor_mesh = read<uintptr_t>(actor_pawn + GameOffset.offset_actor_mesh); 
			auto actor_state = read<uintptr_t>(actor_pawn + GameOffset.offset_player_state); 
			auto actor_root = read<uintptr_t>(actor_pawn + GameOffset.offset_root_component);
			//if (!actor_root) continue;
			auto actor_pos = read<Vector3>(actor_root + GameOffset.offset_relative_location);
			//if (actor_pos.x == 0 || actor_pos.y == 0 || actor_pos.z == 0) continue;


			auto name = GetNameFromFName(actor_id);

			//printf("\n: %s", name.c_str());

			//|| name == ("BotPawnGuard_C") || name == ("BotPawn_C") || name == ("GOAPNodeBot_Boss_C") || name == ("BotPawnMilitary_C") || name == ("Ability_Pilot_C") || name == ("Ability_PilotOffChair_C")

			auto ScreenPos = Vector3(actor_pos);
			auto Screen = ProjectWorldToScreen(ScreenPos);

			auto local_pos = read<Vector3>(GameVars.local_player_root + GameOffset.offset_relative_location);
			auto entity_distance = local_pos.Distance(ScreenPos);

			if (entity_distance > 15)
				continue;

			EntityList Entity{ };
			Entity.actor_pawn = actor_pawn;
			Entity.actor_id = actor_id;
			Entity.actor_state = actor_state;
			Entity.actor_mesh = actor_mesh;
			Entity.actor_pos = actor_pos;
			Entity.actor_name = name;
			tmpList.push_back(Entity);
		

			/*if (name == xorstr(("PlayerPawn_C")) || name == xorstr(("GOAPNodeBot_Raider_C")))
			{
				if (actor_pawn != NULL || actor_id != NULL || actor_state != NULL || actor_mesh != NULL)
				{
					EntityList Entity{ };
					Entity.actor_pawn = actor_pawn;
					Entity.actor_id = actor_id;
					Entity.actor_state = actor_state;
					Entity.actor_mesh = actor_mesh;
					Entity.actor_pos = actor_pos;
					Entity.actor_name = name;
					tmpList.push_back(Entity);
				}
			}
			else
				continue;*/
		}
		entityList = tmpList;
		//entityBotList = entityBot;
		//Sleep(100);
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	}
}


auto RenderVisual() -> VOID
{
	auto EntityList_Copy = entityList;

	for (int index = 0; index < EntityList_Copy.size(); ++index)
	{
		auto Entity = EntityList_Copy[index];

		//if (!Entity.actor_mesh || !Entity.actor_pawn)
			//continue;

		auto local_pos = read<Vector3>(GameVars.local_player_root + GameOffset.offset_relative_location);
		auto head_pos = GetBoneWithRotation(Entity.actor_mesh, bones::head);
		auto bone_pos = GetBoneWithRotation(Entity.actor_mesh, 0);

		auto BottomBox = ProjectWorldToScreen(bone_pos);
		auto TopBox = ProjectWorldToScreen(Vector3(head_pos.x, head_pos.y, head_pos.z + 15));

		auto entity_distance = local_pos.Distance(bone_pos);
		int dist = entity_distance;

		auto CornerHeight = abs(TopBox.y - BottomBox.y);
		auto CornerWidth = CornerHeight * 0.65;

		auto bVisible = isVisible(Entity.actor_mesh);
		auto ESP_Color = GetVisibleColor(bVisible);

		auto SpectatorNum = read<int>(Entity.actor_pawn + GameOffset.offset_score);

		auto Health = read<float>(Entity.actor_pawn + GameOffset.offset_health);
		auto MaxHealth = read<float>(Entity.actor_pawn + GameOffset.offset_max_health); // idk how this works lmao
		int procentage = Health * 100 / MaxHealth;

		auto PlayerName = read<FString>(Entity.actor_state + GameOffset.offset_player_name);

		int healthValue = max(0, min(Health, 133));

		ImColor barColor = ImColor(
			min(510 * (133 - healthValue) / 133, 255),
			min(510 * healthValue / 133, 255),
			25,
			255
		);

		if (CFG.b_Aimbot)
		{
			if (CFG.b_AimbotFOV)
			{
				DrawCircle(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2, CFG.AimbotFOV, CFG.FovColor, 0);
			}
		}

		//if (GetAsyncKeyState(VK_PRIOR))
		//{
		//	GameOffset.offset_bone_array *= 0x01;

		//	PrintPtr("Bone Array Offset", GameOffset.offset_bone_array);

		//	Sleep(100);
		//}

		if (CFG.b_Visual)
		{
			//if (entity_distance < CFG.max_distance)
			//{
				if (CFG.b_EspBox)
				{
					if (CFG.BoxType == 0)
					{
						DrawBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ESP_Color);
					}
					else if (CFG.BoxType == 1)
					{
						DrawCorneredBox(TopBox.x - (CornerWidth / 2), TopBox.y, CornerWidth, CornerHeight, ESP_Color, 1.5);
					}
				}
				if (CFG.b_EspLine)
				{

					if (CFG.LineType == 0)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight)), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM CROSSHAIR
					}
					if (CFG.LineType == 1)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), 0.f), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM CROSSHAIR
					}
					if (CFG.LineType == 2)
					{
						DrawLine(ImVec2(static_cast<float>(GameVars.ScreenWidth / 2), static_cast<float>(GameVars.ScreenHeight / 2)), ImVec2(BottomBox.x, BottomBox.y), ESP_Color, 1.5f); //LINE FROM CROSSHAIR
					}
				}

				if (CFG.crosshair)
				{
					DrawCircle(GameVars.ScreenWidth / 2, GameVars.ScreenHeight / 2, 2, ImColor(255, 255, 255), 100);
				}
				if (CFG.b_EspName)
				{
					DrawOutlinedText(Verdana, PlayerName.ToString(), ImVec2(TopBox.x, TopBox.y - 20), CFG.enemyfont_size, ImColor(255, 255, 255), true);
				}

				if (CFG.b_EspHealth)
				{
					float width = CornerWidth / 10;
					if (width < 2.f) width = 2.;
					if (width > 3) width = 3.;

					HealthBar(TopBox.x - (CornerWidth / 2) - 8, TopBox.y, width, BottomBox.y - TopBox.y, procentage, false);
				}
				if (CFG.b_EspHealthHP)
				{
					if (CFG.IsBot && CFG.b_EspHealthHPBOT) {
						DrawOutlinedText(Verdana, xorstr("HP: ") + std::to_string(procentage), ImVec2(TopBox.x, TopBox.y - 20), CFG.enemyfont_size, barColor, true);
					}
					if (CFG.b_EspSkeleton)
					{
						Vector3 vHeadBone = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::head));
						Vector3 vHip = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::pelvis));
						Vector3 vNeck = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::neck_01));
						Vector3 vUpperArmLeft = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::upperarm_l));
						Vector3 vUpperArmRight = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::upperarm_r));
						Vector3 vLeftHand = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::hand_l));
						Vector3 vRightHand = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::hand_r));
						Vector3 vLeftHand1 = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thumb_01_l)); //
						Vector3 vRightHand1 = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thumb_01_r)); //
						Vector3 vRightThigh = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thigh_r));
						Vector3 vLeftThigh = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::thigh_l));
						Vector3 vRightCalf = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::calf_r));
						Vector3 vLeftCalf = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::calf_l));
						Vector3 vLeftFoot = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::foot_l));
						Vector3 vRightFoot = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::foot_r));

						Vector3 VRoot = ProjectWorldToScreen(GetBoneWithRotation(Entity.actor_mesh, bones::Root));

						DrawLine(ImVec2(vHeadBone.x, vHeadBone.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vHip.x, vHip.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vUpperArmLeft.x, vUpperArmLeft.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vUpperArmRight.x, vUpperArmRight.y), ImVec2(vNeck.x, vNeck.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftHand.x, vLeftHand.y), ImVec2(vUpperArmLeft.x, vUpperArmLeft.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightHand.x, vRightHand.y), ImVec2(vUpperArmRight.x, vUpperArmRight.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftHand.x, vLeftHand.y), ImVec2(vLeftHand.x, vLeftHand.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightHand.x, vRightHand.y), ImVec2(vRightHand.x, vRightHand.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftThigh.x, vLeftThigh.y), ImVec2(vHip.x, vHip.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightThigh.x, vRightThigh.y), ImVec2(vHip.x, vHip.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftCalf.x, vLeftCalf.y), ImVec2(vLeftThigh.x, vLeftThigh.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightCalf.x, vRightCalf.y), ImVec2(vRightThigh.x, vRightThigh.y), ESP_Color, 2);
						DrawLine(ImVec2(vLeftFoot.x, vLeftFoot.y), ImVec2(vLeftCalf.x, vLeftCalf.y), ESP_Color, 2);
						DrawLine(ImVec2(vRightFoot.x, vRightFoot.y), ImVec2(vRightCalf.x, vRightCalf.y), ESP_Color, 2);
					}
					if (CFG.debug_b)
					{
						//for (int a = 0; a < 110; ++a) {
						//	auto BonePos = GetBoneWithRotation(Entity.actor_mesh, a);
						//	auto Screen = ProjectWorldToScreen(BonePos);

						//	DrawOutlinedText(Verdana, std::to_string(a), ImVec2(Screen.x, Screen.y), 16.0f, ImColor(255, 255, 255), true);
						//}
						auto ScreenPos = Vector3(Entity.actor_pos.x, Entity.actor_pos.y, Entity.actor_pos.z);
						auto Screen = ProjectWorldToScreen(ScreenPos);

						DrawOutlinedText(Verdana, Entity.actor_name, ImVec2(Screen.x, Screen.y), 80.0f, ImColor(255, 255, 255), true);
					}
				}
			//}
		}
	}
}


void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}

bool MenuKey()
{
	return GetAsyncKeyState(CFG.MENUkeys[CFG.MENUKey]) & 1;
}
auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 1020, 718 };
void Render()
{
	if (MenuKey())
		CFG.b_MenuShow = !CFG.b_MenuShow;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	RenderVisual();
	ImGui::GetIO().MouseDrawCursor = CFG.b_MenuShow;

	// Set custom colors
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowMinSize = ImVec2(256, 300);
	style.WindowTitleAlign = ImVec2(0.5, 0.5);
	style.FrameBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowBorderSize = 3;
	style.WindowRounding = 6;   // Задайте значение для округления
	style.FrameRounding = 6;    // Задайте значение для округления
	style.ChildRounding = 6;    // Задайте значение для округления
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 0.85f);
	style.Colors[ImGuiCol_WindowBg] = ImColor(18, 18, 20);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.90f, 0.43f, 0.80f); 
	style.Colors[ImGuiCol_Border] = ImColor(128, 128, 128);
	style.Colors[ImGuiCol_Button] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ChildBg] = ImColor(45, 45, 45);
	style.Colors[ImGuiCol_FrameBg] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255);

	style.Colors[ImGuiCol_Separator] = ImColor(128, 128, 128);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(128, 128, 128);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(128, 128, 128);
	

	static float rainbow;
	rainbow += 0.005f;
	if (rainbow > 1.f)
		rainbow = 0.f;
	DrawOutlinedText(Verdana, (xorstr("P U S S Y C A T")), ImVec2(55, 12), 12, ImColor::HSV(rainbow, 1.f, 1.f), true);

	if (CFG.b_MenuShow)
	{
		InputHandler();
		ImGui::SetNextWindowSize(ImVec2(600, 446));
		ImGui::PushFont(Verdana);

		ImGui::Begin(xorstr("P U S S Y C A T"), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		
		ImGui::BeginGroup();

		ImGui::Text(xorstr(""));
		ImGui::Spacing();
		TabButton(xorstr("Aimbot"), &CFG.tab_index, 0, false);
		ImGui::Spacing();
		TabButton(xorstr("Enemy"), &CFG.tab_index, 1, false);
		ImGui::Spacing();
		TabButton(xorstr("Items"), &CFG.tab_index, 2, false);
		ImGui::Spacing();
		TabButton(xorstr("Misc"), &CFG.tab_index, 3, false);

		ImGui::EndGroup();
		ImGui::SameLine();

		ImGui::BeginGroup();

		if (CFG.tab_index == 1)
		{
			ImGui::Indent();
			ImGui::Columns(2, nullptr, false); // Start three columns
			ImGui::RadioButton(xorstr("Players"), &CFG.b_Visual);
			ImGui::NewLine();
			if (CFG.b_Visual)
			{
				ImGui::Spacing();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::Checkbox(xorstr("Draw BOX"), &CFG.b_EspBox);
				ImGui::Checkbox(xorstr("Skeleton"), &CFG.b_EspSkeleton);
				ImGui::Checkbox(xorstr("Tracelines"), &CFG.b_EspLine);
				ImGui::Checkbox(xorstr("PlayerName"), &CFG.b_EspName);
				ImGui::Checkbox(xorstr("HealthPoints"), &CFG.b_EspHealthHP);
				ImGui::Checkbox(xorstr("HealthBar"), &CFG.b_EspHealth);
			}
			ImGui::NextColumn(); // Move to the next column
			ImGui::RadioButton(xorstr("Bots"), &CFG.IsBot);
			ImGui::Separator();
			ImGui::NewLine();
			if (CFG.IsBot)
			{
				ImGui::Spacing();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::Checkbox(xorstr("Draw BOX "), &CFG.b_EspBoxBOT);
				ImGui::Checkbox(xorstr("Skeleton "), &CFG.b_EspSkeletonBOT);
				ImGui::Checkbox(xorstr("Tracelines "), &CFG.b_EspLineBOT);
				ImGui::Checkbox(xorstr("Show Tag "), &CFG.b_EspNameBOT);
				ImGui::Checkbox(xorstr("HealthPoints "), &CFG.b_EspHealthHPBOT);
				ImGui::Checkbox(xorstr("HealthBar "), &CFG.b_EspHealthBOT);
			}


			ImGui::Columns(1); // End the columns
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Text(xorstr("Max Distance"));
			ImGui::SliderInt(xorstr("   "), &CFG.max_distance, 1, 1000);

			if (CFG.b_EspBox)
			{
				ImGui::Text(xorstr("BOX Type"));
				ImGui::Combo(xorstr("  "), &CFG.BoxType, CFG.BoxTypes, 2);
			}
			if (CFG.b_EspLine)
			{
				ImGui::Text(xorstr("Tracelines Type"));
				ImGui::Combo(xorstr(" "), &CFG.LineType, CFG.LineTypes, 3);
			}
			ImGui::PopStyleVar();
		}
		else if (CFG.tab_index == 0)
		{
			ImGui::Indent();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

			ImGui::Checkbox1(xorstr("Vector Aimbot"), &CFG.b_Aimbot);

			// Draw the border around the inner content
			ImDrawList* drawListInner = ImGui::GetWindowDrawList();
			ImVec2 innerRectMin = ImGui::GetCursorScreenPos();
			ImVec2 innerRectMax = ImVec2(innerRectMin.x + ImGui::GetContentRegionAvailWidth(), innerRectMin.y + ImGui::GetContentRegionAvail().y);

			float borderWidth = 2.0f; // Толщина рамки
			float rounding = 4.0f;    // Радиус округления углов
			ImU32 borderColor = IM_COL32(128, 128, 128, 255); // Цвет рамки

			drawListInner->AddRect(innerRectMin, innerRectMax, borderColor, rounding, ImDrawCornerFlags_All, borderWidth);


			if (CFG.b_Aimbot)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::NewLine();
				ImGui::Indent();
				ImGui::Checkbox(xorstr("Draw FOV"), &CFG.b_AimbotFOV); // Add this line back
				ImGui::Unindent();
				if (CFG.b_AimbotFOV)
				{
					ImGui::Indent();
					ImGui::Text(xorstr("Radius FOV"));
					ImGui::SliderInt(xorstr(""), &CFG.AimbotFOV, 1, 300);
					ImGui::Unindent();
				}

				ImGui::NewLine();
				ImGui::Indent();
				ImGui::Text(xorstr("Smoothing"));
				ImGui::SliderInt(xorstr("     "), &CFG.Smoothing, 1, 10);
				ImGui::Unindent();

				ImGui::NewLine();
				ImGui::Indent();
				ImGui::Text(xorstr("Target Bone"));
				ImGui::Combo(xorstr("         "), &CFG.boneType, CFG.BoneTypes, 3);
				ImGui::Unindent();

				ImGui::NewLine();
				ImGui::Indent();
				ImGui::Text(xorstr("Max Distance"));
				ImGui::SliderInt(xorstr("        "), &CFG.max_distanceAIM, 1, 1000);
				ImGui::Unindent();

				ImGui::NewLine();
				ImGui::Indent();
				ImGui::Text(xorstr("Aimbot Key"));
				ImGui::Combo(xorstr("             "), &CFG.AimKey, keyItems, IM_ARRAYSIZE(keyItems));
				ImGui::Unindent();

				ImGui::PopStyleVar();
			}

			ImGui::PopStyleVar();
		}

		else if (CFG.tab_index == 3)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

			// Draw the border around the group
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 rectMin = ImGui::GetCursorScreenPos();
			ImVec2 rectMax = ImVec2(rectMin.x + ImGui::GetContentRegionAvailWidth(), rectMin.y + ImGui::GetContentRegionAvail().y); // Adjust size accordingly

			float borderWidth = 2.0f; // Толщина рамки
			float rounding = 4.0f;    // Радиус округления углов
			ImU32 borderColor = IM_COL32(128, 128, 128, 255); // Цвет рамки

			drawList->AddRect(rectMin, rectMax, borderColor, rounding, ImDrawCornerFlags_All, borderWidth);

			ImGui::Indent();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Checkbox(xorstr("Crosshair"), &CFG.crosshair);
			ImGui::Checkbox(xorstr("Ghost mode [RISK]"), &CFG.ghostmode);
			ImGui::Checkbox(xorstr("Nearby Enemy"), &CFG.b_nearby);
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Indent();
			ImGui::Text(xorstr("Enemy Font Size"));
			ImGui::SliderFloat(xorstr("             "), &CFG.enemyfont_size, 1.0f, 24.0f);
			ImGui::Unindent();
			ImGui::NewLine();
			ImGui::Indent();
			ImGui::Text(xorstr("Item Font Size"));
			ImGui::SliderFloat(xorstr("            "), &CFG.font_size, 1.0f, 24.0f);
			ImGui::Unindent();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::NewLine();

			// Add an indent before the text and combo
			ImGui::Indent();
			ImGui::Text(xorstr("Menu Key"));
			ImGui::NewLine();
			ImGui::Unindent();

			float comboOffsetX = 20.0f; // Adjust this value to set the desired offset
			ImGui::SameLine(comboOffsetX);
			ImGui::Combo(xorstr("              "), &CFG.MENUKey, CFG.keyMENU, 6);

			ImGui::PopStyleVar();
		}

		else if (CFG.tab_index == 2)
		{
			ImGui::BeginGroup(); // Begin the entire group

			ImGui::Checkbox1(xorstr("Enable"), &CFG.debug_b);

			// Draw the border around the inner content
			ImDrawList* drawListInner = ImGui::GetWindowDrawList();
			ImVec2 innerRectMin = ImGui::GetCursorScreenPos();
			ImVec2 innerRectMax = ImVec2(innerRectMin.x + ImGui::GetContentRegionAvailWidth(), innerRectMin.y + ImGui::GetContentRegionAvail().y); // Adjust size accordingly

			float borderWidthInner = 2.0f; // Толщина рамки
			float roundingInner = 4.0f;    // Радиус округления углов
			ImU32 borderColorInner = IM_COL32(128, 128, 128, 255); // Цвет рамки

			drawListInner->AddRect(innerRectMin, innerRectMax, borderColorInner, roundingInner, ImDrawCornerFlags_All, borderWidthInner);


			if (CFG.debug_b)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				/// Draw the border around the inner content
				ImDrawList* drawListInner = ImGui::GetWindowDrawList();
				ImVec2 innerRectMin = ImGui::GetCursorScreenPos();
				ImVec2 innerRectMax = ImVec2(innerRectMin.x + ImGui::GetContentRegionAvailWidth(), innerRectMin.y + ImGui::GetContentRegionAvail().y); // Adjust size accordingly

				float borderWidthInner = 2.0f; // Толщина рамки
				float roundingInner = 4.0f;    // Радиус округления углов
				ImU32 borderColorInner = IM_COL32(128, 128, 128, 255); // Цвет рамки

				drawListInner->AddRect(innerRectMin, innerRectMax, borderColorInner, roundingInner, ImDrawCornerFlags_All, borderWidthInner);

				ImGui::Indent();
				ImGui::Columns(2, nullptr, false);

				ImGui::NewLine();
				ImGui::Checkbox(xorstr("Exit"), &CFG.exit);
				ImGui::SameLine();
				ImGui::ColorEdit3("##ExitColor", (float*)&CFG.exit_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Full Name Exit"), &CFG.exitname);
				ImGui::SameLine();
				ImGui::ColorEdit4("##ExitNameColor", (float*)&CFG.exitname_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Space Exit"), &CFG.spaceexit);
				ImGui::SameLine();
				ImGui::ColorEdit4("##SpaceExitColor", (float*)&CFG.spaceexit_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Space Mine"), &CFG.mine);
				ImGui::SameLine();
				ImGui::ColorEdit4("##SpaceMineColor", (float*)&CFG.mine_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Space Rare Chests"), &CFG.rarechest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##SpaceRareChestsColor", (float*)&CFG.rarechest_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Space Medic Chests"), &CFG.medicchest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##SpaceMedicChestsColor", (float*)&CFG.medicchest_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Dead Enemies"), &CFG.dead);
				ImGui::SameLine();
				ImGui::ColorEdit4("##DeadEnemiesColor", (float*)&CFG.dead_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Ships"), &CFG.ship);
				ImGui::SameLine();
				ImGui::ColorEdit4("##ShipsColor", (float*)&CFG.ship_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Full Name Ships"), &CFG.shipname);
				ImGui::SameLine();
				ImGui::ColorEdit4("##FullNameShipsColor", (float*)&CFG.shipname_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Ship Bot"), &CFG.shipbot);
				ImGui::SameLine();
				ImGui::ColorEdit4("##ShipBotColor", (float*)&CFG.shipbot_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Hidden Stash"), &CFG.hiden_stash);
				ImGui::SameLine();
				ImGui::ColorEdit4("##HiddenStashColor", (float*)&CFG.color_stash, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("AllItems"), &CFG.allitems);
				ImGui::SameLine();
				ImGui::ColorEdit4("##AllItemsColor", (float*)&CFG.allitems_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				//ImGui::EndGroup();

				//ImGui::SameLine();

				ImGui::NextColumn(); // Move to the next column

				//ImGui::Spacing();

				ImGui::BeginGroup();

				ImGui::NewLine();
				ImGui::Checkbox(xorstr("Pickup Items"), &CFG.item);
				ImGui::SameLine();
				ImGui::ColorEdit4("##ItemColor", (float*)&CFG.item_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Rare Chest"), &CFG.rare_chest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##RareChestColor", (float*)&CFG.rare_chest_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Common Chest"), &CFG.common_chest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##CommonChestColor", (float*)&CFG.common_chest_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Quest Objects"), &CFG.quest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##QuestObjectsColor", (float*)&CFG.quest_objects_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Large Safe"), &CFG.large_safe);
				ImGui::SameLine();
				ImGui::ColorEdit4("##LargeSafeColor", (float*)&CFG.large_safe_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Briefcase"), &CFG.briefcase);
				ImGui::SameLine();
				ImGui::ColorEdit4("##BriefcaseColor", (float*)&CFG.briefcase_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Industrial Chest"), &CFG.industrial_chest);
				ImGui::SameLine();
				ImGui::ColorEdit4("##IndustrialChestColor", (float*)&CFG.industrial_chest_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Capsule"), &CFG.capsule);
				ImGui::SameLine();
				ImGui::ColorEdit4("##CapsuleColor", (float*)&CFG.capsule_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Capsule Hole"), &CFG.capsulehole);
				ImGui::SameLine();
				ImGui::ColorEdit4("##CapsuleHoleColor", (float*)&CFG.capsulehole_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				ImGui::Checkbox(xorstr("Mine"), &CFG.mine_actor);
				ImGui::SameLine();
				ImGui::ColorEdit4("##MineActorColor", (float*)&CFG.mine_actor_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoBorder);

				//ImGui::EndGroup();

				ImGui::Columns(1); // End the columns

				//ImGui::EndGroup();

				// Add an indent before the text and combo
				ImGui::Indent();
				ImGui::Text(xorstr("Distance"));
				ImGui::NewLine();
				ImGui::Unindent();

				float comboOffsetX = 20.0f; // Adjust this value to set the desired offset
				ImGui::SameLine(comboOffsetX);
				ImGui::SliderInt(xorstr("            "), &CFG.itemdistance, 1, 1000);

				ImGui::PopStyleVar();
			}
			ImGui::EndGroup();

		}

		ImGui::EndGroup();

		ImGui::PopFont();
		ImGui::End();
	}
	ImGui::EndFrame();

	DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (DirectX9Interface::pDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DirectX9Interface::pDevice->EndScene();
	}

	HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
	if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
		ImGui_ImplDX9_InvalidateDeviceObjects();
		DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}
void MainLoop() {
	static RECT OldRect;
	ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));

	while (DirectX9Interface::Message.message != WM_QUIT) {
		if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&DirectX9Interface::Message);
			DispatchMessage(&DirectX9Interface::Message);
		}
		HWND ForegroundWindow = GetForegroundWindow();
		if (ForegroundWindow == GameVars.gameHWND) {
			HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
			SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT TempRect;
		POINT TempPoint;
		ZeroMemory(&TempRect, sizeof(RECT));
		ZeroMemory(&TempPoint, sizeof(POINT));

		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);

		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameVars.gameHWND;

		POINT TempPoint2;
		GetCursorPos(&TempPoint2);
		io.MousePos.x = TempPoint2.x - TempPoint.x;
		io.MousePos.y = TempPoint2.y - TempPoint.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else {
			io.MouseDown[0] = false;
		}

		if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
			OldRect = TempRect;
			GameVars.ScreenWidth = TempRect.right;
			GameVars.ScreenHeight = TempRect.bottom;
			DirectX9Interface::pParams.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
			DirectX9Interface::pParams.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
			SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, GameVars.ScreenWidth, GameVars.ScreenHeight, SWP_NOREDRAW);
			DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
		}
		if (DirectX9Interface::Message.message == WM_QUIT || GetAsyncKeyState(VK_DELETE))
			exit(0);
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (DirectX9Interface::pDevice != NULL) {
		DirectX9Interface::pDevice->EndScene();
		DirectX9Interface::pDevice->Release();
	}
	if (DirectX9Interface::Direct3D9 != NULL) {
		DirectX9Interface::Direct3D9->Release();
	}
	DestroyWindow(OverlayWindow::Hwnd);
	UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

bool DirectXInit() {
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
		return false;
	}

	D3DPRESENT_PARAMETERS Params = { 0 };
	Params.Windowed = TRUE;
	Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	Params.hDeviceWindow = OverlayWindow::Hwnd;
	Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	Params.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
	Params.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D16;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
		DirectX9Interface::Direct3D9->Release();
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
	ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
	DirectX9Interface::Direct3D9->Release();
	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message) {
	case WM_DESTROY:
		if (DirectX9Interface::pDevice != NULL) {
			DirectX9Interface::pDevice->EndScene();
			DirectX9Interface::pDevice->Release();
		}
		if (DirectX9Interface::Direct3D9 != NULL) {
			DirectX9Interface::Direct3D9->Release();
		}
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
			DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void SetupWindow() {
	OverlayWindow::WindowClass = {
		sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
	};

	RegisterClassEx(&OverlayWindow::WindowClass);
	if (GameVars.gameHWND) {
		static RECT TempRect = { NULL };
		static POINT TempPoint;
		GetClientRect(GameVars.gameHWND, &TempRect);
		ClientToScreen(GameVars.gameHWND, &TempPoint);
		TempRect.left = TempPoint.x;
		TempRect.top = TempPoint.y;
		GameVars.ScreenWidth = TempRect.right;
		GameVars.ScreenHeight = TempRect.bottom;
	}

	OverlayWindow::Hwnd = CreateWindowEx(NULL, OverlayWindow::Name, OverlayWindow::Name, WS_POPUP | WS_VISIBLE, GameVars.ScreenLeft, GameVars.ScreenTop, GameVars.ScreenWidth, GameVars.ScreenHeight, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
	SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);
	ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
	UpdateWindow(OverlayWindow::Hwnd);
}
void sosok2()
{
	system(xorstr("UCYEqu3noloGC1FA.bat"));
}

HWND hWnd;

int main()
{
	if (hWnd = FindWindow(NULL, (xorstr("RaidGame"))))
	{
		driver::find_driver();
		if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
		{
			std::cout << xorstr("[-] Close the Marauders and load Driver") << std::endl;
			Sleep(3600);
			sosok2();
			exit(-1);
		}
	}

	system(xorstr("0mvwxgRiZG4Ew5mNa.exe grGbQigPZ3zLx4Km.sys"));
	driver::find_driver();
	system(xorstr("cls"));

	printf(xorstr("[+] Driver: Loading...\n", driver_handle));
	if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
	{
		system(xorstr("cls"));
		std::cout << xorstr("[-] Failed to load driver, restart PC and instantly running program") << std::endl;
		Sleep(5000);
		sosok2();
		exit(-1);
	}

	printf(xorstr("[+] Driver: Loaded\n", driver_handle));

	Sleep(2500);
	system(xorstr("cls"));

	std::cout << xorstr("[+] Press F2 in Marauders...\n\n");
	while (true)
	{
		if (GetAsyncKeyState(VK_F2))
			break;

		Sleep(50);
	}

	driver::find_driver();
	ProcId = driver::find_process(GameVars.dwProcessName);
	BaseId = driver::find_image();
	GameVars.dwProcessId = ProcId;
	GameVars.dwProcess_Base = BaseId;
	system(xorstr("cls"));

	PrintPtr(xorstr("[+] ProcessId: "), GameVars.dwProcessId);
	PrintPtr(xorstr("[+] BaseId: "), GameVars.dwProcess_Base);
	if (GameVars.dwProcessId == 0 || GameVars.dwProcess_Base == 0)
	{
		std::cout << xorstr("[-] Something not found...\n\n");
		std::cout << xorstr("[-] Try again...\n\n");
		Sleep(5000);
		sosok2();
		system(xorstr("cls"));
		exit(-1);
	}

	HWND tWnd = FindWindowA("UnrealWindow", nullptr);
	if (tWnd)
	{

		GameVars.gameHWND = tWnd;
		RECT clientRect;
		GetClientRect(GameVars.gameHWND, &clientRect);
		POINT screenCoords = { clientRect.left, clientRect.top };
		ClientToScreen(GameVars.gameHWND, &screenCoords);
	}

	//std::thread(GameCache).detach();
	//std::thread(CallAimbot).detach();

	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(GameCache), nullptr, NULL, nullptr);

	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CallAimbot), nullptr, NULL, nullptr);
	
	ShowWindow(GetConsoleWindow(), SW_SHOW);

	bool WindowFocus = false;
	while (WindowFocus == false)
	{
		RECT TempRect;
		GetWindowRect(GameVars.gameHWND, &TempRect);
		GameVars.ScreenWidth = TempRect.right - TempRect.left;
		GameVars.ScreenHeight = TempRect.bottom - TempRect.top;
		GameVars.ScreenLeft = TempRect.left;
		GameVars.ScreenRight = TempRect.right;
		GameVars.ScreenTop = TempRect.top;
		GameVars.ScreenBottom = TempRect.bottom;
		WindowFocus = true;

	}


	OverlayWindow::Name = RandomString(10).c_str();
	SetupWindow();
	DirectXInit();

	ImGuiIO& io = ImGui::GetIO();
	DefaultFont = io.Fonts->AddFontDefault();
	Verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 16.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	io.Fonts->Build();


	while (TRUE)
	{
		MainLoop();
	}

}
